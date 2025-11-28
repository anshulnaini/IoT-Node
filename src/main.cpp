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

// --- Pin Definitions ---
#define BUTTON_PIN 0
#define I2C_SDA 8
#define I2C_SCL 9
#define OLED_POWER_PIN 3
#define SENSOR_POWER_PIN 2

// --- Global Objects ---
ConfigManager configManager;
ButtonHandler buttonHandler(BUTTON_PIN);
OLEDHandler oled(I2C_SDA, I2C_SCL);
ApiHandler apiHandler(configManager);
PowerManager powerManager(BUTTON_PIN, OLED_POWER_PIN, SENSOR_POWER_PIN);
PortalManager portalManager(configManager);
SensorHandler sensorHandler;

// --- State Machine ---
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

// --- Function Prototypes ---
void checkWakeupReason();
bool connectToWiFi();

void setup() {
  Serial.begin(115200);
  //while (!Serial) { delay(10); }
  delay(3000); // Wait for serial to stabilize
  
  powerManager.peripherals_on();
  // Add a small delay to ensure peripherals are stable before use.
  delay(100); 

  Serial.println("\n\n--- Booting IoT Node ---");

  oled.initializeOLED();
  oled.displayText("Booting...");
  configManager.begin();
  buttonHandler.begin();
  sensorHandler.begin();
  configManager.loadConfig();

  checkWakeupReason();
}

void loop() {
  buttonHandler.tick();
  ButtonEvent event = buttonHandler.getEvent();

  if (event == EV_LONG_PRESS) {
    Serial.println("\n!!! FACTORY RESET TRIGGERED !!!");
    oled.displayText("FACTORY RESET");
    configManager.clearConfig();
    delay(3000);
    ESP.restart();
  }

  switch (currentState) {
    case STATE_BOOT:
      Serial.println("State: BOOT");
      if (configManager.isConfigured()) {
        currentState = STATE_CONNECTING_WIFI;
      }
      else {
        currentState = STATE_SETUP_START;
      }
      break;

    case STATE_INFO_DISPLAY:
      if (stateTimer == 0) {
        Serial.println("State: INFO_DISPLAY");
        const DeviceConfig& config = configManager.getConfig();
        float temp = sensorHandler.readTemperature();
        float humidity = sensorHandler.readHumidity();
        oled.displayInfo(config.deviceName, config.deviceId, "N/A", temp, humidity);
        stateTimer = millis();
      }

      // Event handling for this state
      if (event == EV_SINGLE_CLICK) {
        Serial.println("Single-click: Going to sleep.");
        stateTimer = 0;
        currentState = STATE_DEEP_SLEEP;
        break; // Exit the switch case
      }
      if (event == EV_DOUBLE_CLICK) {
        Serial.println("Double-click: Forcing telemetry send.");
        stateTimer = 0;
        currentState = STATE_TELEMETRY_SEND;
        break; // Exit the switch case
      }
      if (event == EV_TRIPLE_CLICK) {
        Serial.println("Triple-click: Entering setup mode.");
        stateTimer = 0;
        currentState = STATE_SETUP_START;
        break; // Exit the switch case
      }

      // Timeout to go to sleep if no interaction
      if (millis() - stateTimer > 10000) {
        Serial.println("Info display timed out. Going to sleep.");
        stateTimer = 0;
        currentState = STATE_DEEP_SLEEP;
      }
      break;

    case STATE_SETUP_START:
      Serial.println("State: SETUP_START");
      oled.displayText("Setup Mode");
      portalManager.start();
      currentState = STATE_SETUP_RUNNING;
      break;

    case STATE_SETUP_RUNNING:
      portalManager.loop();
      if (portalManager.isConfigSaved()) {
        stateTimer = 0; // Reset timer for next use
        currentState = STATE_SETUP_COMPLETE;
      }
      break;

    case STATE_SETUP_COMPLETE:
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

    case STATE_CONNECTING_WIFI:
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

    case STATE_TELEMETRY_SEND:
      Serial.println("State: TELEMETRY_SEND");
      oled.displayText("Registering...");
      if (apiHandler.registerDeviceIfNeeded()) {
        oled.displayText("Sending...");
        float temp = sensorHandler.readTemperature();
        float humidity = sensorHandler.readHumidity();
        Serial.printf("Readings: Temp=%.2f C, Humidity=%.2f %%\n", temp, humidity);

        if (apiHandler.sendTelemetry(temp, humidity, 95.0)) { // Using 95.0 as placeholder for battery
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

    case STATE_TASK_COMPLETE:
      if (millis() - stateTimer > 5000) {
        stateTimer = 0;
        currentState = STATE_DEEP_SLEEP;
      }
      break;

    case STATE_DEEP_SLEEP:
      Serial.println("State: DEEP_SLEEP");
      oled.displayText("Sleeping...");
      
      // Turn off peripherals and wait a moment for them to power down
      powerManager.peripherals_off();
      delay(100);

      // Use a default sleep interval if not configured
      int sleepInterval = configManager.getConfig().sleepIntervalSeconds;
      if (sleepInterval <= 0) {
        sleepInterval = 300; // Default to 5 minutes if not set
      }
      powerManager.enterDeepSleep(sleepInterval);
      break;
  }
}

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
      Serial.println("Wakeup not caused by deep sleep (power on)");
      currentState = STATE_BOOT;
      break;
  }
}

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
    delay(200); // Keep small delay to prevent busy-looping too fast
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  return true;
}