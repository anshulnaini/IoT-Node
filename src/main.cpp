#include <Arduino.h>
#include "ConfigManager.h"
#include "PortalManager.h"
#include "ButtonHandler.h"
#include "OLEDHandler.h"
#include "ApiHandler.h"
#include <WiFi.h>

// --- Pin Definitions ---
#define BUTTON_PIN 10
#define I2C_SDA 8
#define I2C_SCL 9

// --- Global Objects ---
ConfigManager configManager;
ButtonHandler buttonHandler(BUTTON_PIN);
OLEDHandler oled(I2C_SDA, I2C_SCL);
ApiHandler apiHandler(configManager);

// --- Non-blocking timer for the main loop ---
unsigned long lastLoopMessageTime = 0;
const long loopMessageInterval = 10000;

// --- Function Prototypes ---
void checkForFactoryReset();
bool connectToWiFi();

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n--- Booting IoT Node ---");

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
    if (connectToWiFi()) {
      oled.displayText("Registering...");
      if (apiHandler.registerDeviceIfNeeded()) {
        oled.displayText("Registered!");
        delay(2000); // Show "Registered!" for a moment

        // --- Send a test telemetry packet ---
        oled.displayText("Sending...");
        Serial.println("Sending initial telemetry packet...");
        if (apiHandler.sendTelemetry(24.5, 55.8, 95.0)) {
          oled.displayText("Sent!");
          Serial.println("Telemetry sent successfully.");
        } else {
          oled.displayText("Send Failed");
          Serial.println("Failed to send telemetry.");
        }

      } else {
        oled.displayText("Reg. Failed");
      }
    } else {
      oled.displayText("WiFi Failed");
    }
    delay(5000); // Keep message on screen
  }
}

void loop() {
  checkForFactoryReset();

  unsigned long currentTime = millis();
  if (currentTime - lastLoopMessageTime >= loopMessageInterval) {
    lastLoopMessageTime = currentTime;
    Serial.println("Main loop running...");
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
    if (millis() - startTime > 15000) {
      WiFi.disconnect();
      return false;
    }
    
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
  
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
  return true;
}