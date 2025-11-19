#include <Arduino.h>
#include "ConfigManager.h"
#include "PortalManager.h"
#include "ButtonHandler.h"
#include "OLEDHandler.h"
#include <WiFi.h>

// --- Pin Definitions ---
#define BUTTON_PIN 10
// Default I2C pins for XIAO ESP32-C3
#define I2C_SDA 8
#define I2C_SCL 9

// --- Global Objects ---
ConfigManager configManager;
ButtonHandler buttonHandler(BUTTON_PIN);
OLEDHandler oled(I2C_SDA, I2C_SCL);

// --- Non-blocking timer for the main loop ---
unsigned long lastLoopMessageTime = 0;
const long loopMessageInterval = 10000;

// --- Function Prototypes ---
void checkForFactoryReset();
bool connectToWiFi();

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("\n\n--- Booting IoT Node ---");

  // Initialize handlers
  oled.initializeOLED();
  oled.displayText("Booting...");
  configManager.begin();
  buttonHandler.begin();
  configManager.loadConfig();

  Serial.println("Checking for factory reset command (hold button)...");
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) {
    checkForFactoryReset();
  }

  if (!configManager.isConfigured()) {
    // --- SETUP MODE ---
    oled.displayText("Setup Mode");
    Serial.println("Device is not configured. Starting setup portal.");
    PortalManager portalManager(configManager);
    portalManager.start();

    while (!portalManager.isConfigSaved()) {
      portalManager.loop();
    }

    portalManager.stop();
    oled.displayText("Restarting...");
    Serial.println("Configuration saved. Restarting device in 5 seconds...");
    delay(5000);
    ESP.restart();

  } else {
    // --- NORMAL OPERATION ---
    Serial.println("Device is configured. Connecting to WiFi...");
    if (connectToWiFi()) {
      Serial.println("WiFi Connected!");
      String ipAddr = "IP: " + WiFi.localIP().toString();
      oled.displayText(ipAddr.c_str());
      delay(5000); // Show IP for 5 seconds
    } else {
      Serial.println("WiFi Connection Failed.");
      oled.displayText("WiFi Failed");
      delay(5000);
    }
  }
}

void loop() {
  checkForFactoryReset();

  unsigned long currentTime = millis();
  if (currentTime - lastLoopMessageTime >= loopMessageInterval) {
    lastLoopMessageTime = currentTime;
    Serial.println("Main loop running...");
    // In the future, this is where we will enter deep sleep.
  }
}

void checkForFactoryReset() {
  buttonHandler.tick();
  if (buttonHandler.getEvent() == EV_LONG_PRESS) {
    Serial.println("\n!!! FACTORY RESET TRIGGERED !!!");
    oled.displayText("FACTORY RESET");
    Serial.println("Clearing configuration and restarting in 3 seconds...");
    configManager.clearConfig();
    delay(3000);
    ESP.restart();
  }
}

bool connectToWiFi() {
  const DeviceConfig& config = configManager.getConfig();
  if (strlen(config.wifiSSID) == 0) {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifiSSID, config.wifiPassword);

  String connectingMsg = "Connecting";
  oled.displayText(connectingMsg.c_str());

  unsigned long startTime = millis();
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // Timeout after 15 seconds
    if (millis() - startTime > 15000) {
      WiFi.disconnect();
      return false;
    }
    
    // Animate "Connecting..." on the OLED
    if(++dots > 3) {
      dots = 0;
      connectingMsg = "Connecting";
    } else {
      connectingMsg += ".";
    }
    oled.displayText(connectingMsg.c_str());
    
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(); // Newline after the dots
  return true;
}
