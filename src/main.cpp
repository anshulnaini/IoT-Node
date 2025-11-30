#include <Arduino.h>
#include "ConfigManager.h"
#include "PortalManager.h"
#include "ButtonHandler.h"
#include "OLEDHandler.h"
#include "ApiHandler.h"
#include "PowerManager.h"
#include "SensorHandler.h"
#include "esp_sleep.h"
#include <WiFi.h>

//Pins
#define BUTTON_PIN 0
#define I2C_SDA 8
#define I2C_SCL 9
#define OLED_POWER_PIN 3
#define SENSOR_POWER_PIN 2

// Global Objects
ConfigManager configManager;
ButtonHandler buttonHandler(BUTTON_PIN);
OLEDHandler oled(I2C_SDA, I2C_SCL);
ApiHandler apiHandler(configManager);
PowerManager powerManager(BUTTON_PIN, OLED_POWER_PIN, SENSOR_POWER_PIN);
PortalManager portalManager(configManager);
SensorHandler sensorHandler;

//State Machine
enum DeviceState {
  STATE_BOOT,
  STATE_INFO_DISPLAY,
  STATE_SETUP_START,
  STATE_SETUP_RUNNING,
  STATE_SETUP_COMPLETE,
  STATE_CONNECTING_WIFI,
  STATE_TELEMETRY_SEND,
  STATE_TASK_COMPLETE,
  STATE_DEEP_SLEEP
};
DeviceState currentState = STATE_BOOT;
unsigned long stateTimer = 0;

// prototypes
void checkWakeupReason();
bool connectToWiFi();

//setup
void setup() {
  Serial.begin(115200);
  delay(100);
  
  powerManager.peripherals_on();
  delay(100); 

  Serial.println("\n\n Booting IoT Node ");

  oled.initializeOLED();
  oled.displayText("Booting...");
  configManager.begin();
  buttonHandler.begin();
  sensorHandler.begin();
  configManager.loadConfig();

  checkWakeupReason();
}


//loop
void loop() {
  buttonHandler.tick();
  ButtonEvent event = buttonHandler.getEvent();

  // checks for a factory reset before doing anything
  if (event == EV_LONG_PRESS) {
    Serial.println("\n!!! FACTORY RESET TRIGGERED !!!");
    oled.displayText("FACTORY RESET");
    configManager.clearConfig();
    delay(3000);
    ESP.restart();
  }

  // does differnt things based on what state it is in every loop
  switch (currentState) {
    case STATE_BOOT: // initial state after power on or reset
      Serial.println("State: BOOT");
      if (configManager.isConfigured()) {
        currentState = STATE_CONNECTING_WIFI;
      }
      else {
        currentState = STATE_SETUP_START; // start setup if not configured
      }
      break;

    case STATE_INFO_DISPLAY: // shows device info and sensor readings
      if (stateTimer == 0) {
        Serial.println("State: INFO_DISPLAY");
        const DeviceConfig& config = configManager.getConfig();
        float temp = sensorHandler.readTemperature();
        float humidity = sensorHandler.readHumidity();
        oled.displayInfo(config.deviceName, config.deviceId, config.serverUrl, temp, humidity);
        stateTimer = millis();
      }

      // Event handling for this state
      if (event == EV_SINGLE_CLICK) { // go back to sleep
        Serial.println("Single-click: Going to sleep.");
        stateTimer = 0;
        currentState = STATE_DEEP_SLEEP;
        break; 
      }
      if (event == EV_DOUBLE_CLICK) { // force telemetry send
        Serial.println("Double-click: Forcing telemetry send.");
        stateTimer = 0;
        currentState = STATE_TELEMETRY_SEND;
        break; 
      }
      if (event == EV_TRIPLE_CLICK) { // enter setup mode
        Serial.println("Triple-click: Entering setup mode.");
        stateTimer = 0;
        currentState = STATE_SETUP_START;
        break; 
      }

      // Timeout to go to sleep if no interaction
      if (millis() - stateTimer > 10000) {
        Serial.println("Info display timed out. Going to sleep.");
        stateTimer = 0;
        currentState = STATE_DEEP_SLEEP;
      }
      break;

    case STATE_SETUP_START: // starts the setup portal
      Serial.println("State: SETUP_START");
      oled.displayText("Setup Mode");
      portalManager.start();
      currentState = STATE_SETUP_RUNNING;
      break;

    case STATE_SETUP_RUNNING: // runs the setup portal and waits for the config to be saved (user input)
      portalManager.loop();
      if (portalManager.isConfigSaved()) {
        stateTimer = 0;
        currentState = STATE_SETUP_COMPLETE;
      }
      break;

    case STATE_SETUP_COMPLETE: // finalizes setup and restarts the device
       if (stateTimer == 0) {
        Serial.println("State: SETUP_COMPLETE");
        portalManager.stop();
        oled.displayText("Restarting...");
        stateTimer = millis();
      }
      if (millis() - stateTimer > 5000) {
        ESP.restart();
      }
      break;

    case STATE_CONNECTING_WIFI: // connects to wifi
      Serial.println("State: CONNECTING_WIFI");
      if (connectToWiFi()) {
        currentState = STATE_TELEMETRY_SEND;
      }
      else {
        oled.displayText("WiFi Failed");
        stateTimer = millis();
        currentState = STATE_TASK_COMPLETE;
      }
      break;

    case STATE_TELEMETRY_SEND: // sends telemetry to server
      Serial.println("State: TELEMETRY_SEND");
      oled.displayText("Registering...");
      if (apiHandler.registerDeviceIfNeeded()) { // tries to register if needed
        oled.displayText("Sending...");
        float temp = sensorHandler.readTemperature();
        float humidity = sensorHandler.readHumidity();
        Serial.printf("Readings: Temp=%.2f C, Humidity=%.2f %%\n", temp, humidity);

        if (apiHandler.sendTelemetry(temp, humidity, 95.0)) { // havent figures out battery reading so this is a placeholder
          oled.displayText("Sent!");
        }
        else {
          oled.displayText("Send Failed");
        }
      }
      else {
        oled.displayText("Reg. Failed");
      }
      stateTimer = millis();
      currentState = STATE_TASK_COMPLETE;
      break;

    case STATE_TASK_COMPLETE: //goes back to sleep
      if (millis() - stateTimer > 5000) {
        stateTimer = 0;
        currentState = STATE_DEEP_SLEEP;
      }
      break;

    case STATE_DEEP_SLEEP: // puts the device into deep sleep and shuts down peripherals
      Serial.println("State: DEEP_SLEEP");
      oled.displayText("Sleeping...");
      
      // Turn off peripherals and wait for them to power down
      powerManager.peripherals_off();
      delay(100);

      // Use  sleep interval 
      int sleepInterval = configManager.getConfig().sleepIntervalSeconds;
      //default
      if (sleepInterval <= 0) {
        sleepInterval = 300; 
      }
      powerManager.enterDeepSleep(sleepInterval);
      break;
  }
}


// checks why it woke up to figure out what state it should be in
void checkWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      currentState = STATE_CONNECTING_WIFI;
      break;
    case ESP_SLEEP_WAKEUP_GPIO:
      Serial.println("Wakeup caused by GPIO");
      currentState = STATE_INFO_DISPLAY;
      break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      Serial.println("Wakeup not caused by deep sleep");
      currentState = STATE_BOOT;
      break;
  }
}


// tries to connect to the wifi
bool connectToWiFi() {
  const DeviceConfig& config = configManager.getConfig();
  if (strlen(config.wifiSSID) == 0) return false;

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifiSSID, config.wifiPassword);

  oled.displayText("Connecting...");
  Serial.print("Connecting to WiFi...");

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > 15000) {
      WiFi.disconnect();
      Serial.println(" failed!");
      return false;
    }
    buttonHandler.tick();
    if (buttonHandler.getEvent() == EV_LONG_PRESS) { ESP.restart(); }
    delay(200);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  return true;
}