#include <Arduino.h>
#include "ConfigManager.h"
#include "PortalManager.h"
#include "ButtonHandler.h"

#define BUTTON_PIN 10

ConfigManager configManager;
ButtonHandler buttonHandler(BUTTON_PIN);

void checkForFactoryReset();

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("\n\n--- Booting IoT Node ---");

  // Initialize handlers
  configManager.begin();
  buttonHandler.begin();
  
  // Load the configuration from NVS
  configManager.loadConfig();

  // Before doing anything else, check if the user is holding the button
  // to force a factory reset. We'll check for 3 seconds.
  Serial.println("Checking for factory reset command (hold button)...");
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) {
    checkForFactoryReset();
  }

  if (!configManager.isConfigured()) {
    // --- SETUP MODE ---
    Serial.println("Device is not configured. Starting setup portal.");
    PortalManager portalManager(configManager);
    portalManager.start();

    while (!portalManager.isConfigSaved()) {
      portalManager.loop();
    }

    portalManager.stop();
    Serial.println("Configuration saved. Restarting device in 5 seconds...");
    delay(5000);
    ESP.restart();

  } else {
    // --- NORMAL OPERATION ---
    Serial.println("Device is configured. Proceeding with normal operation.");
    const DeviceConfig& config = configManager.getConfig();
    Serial.print("Device Name: ");
    Serial.println(config.deviceName);
  }
}

void loop() {
  // The main loop should always check for a factory reset.
  checkForFactoryReset();

  Serial.println("Main loop running...");
  delay(5000);
}

/**
 * @brief Checks the button for a long press event and triggers a factory reset.
 */
void checkForFactoryReset() {
  buttonHandler.tick();
  if (buttonHandler.getEvent() == EV_LONG_PRESS) {
    Serial.println("\n!!! FACTORY RESET TRIGGERED !!!");
    Serial.println("Clearing configuration and restarting in 3 seconds...");
    configManager.clearConfig();
    delay(3000);
    ESP.restart();
  }
}
