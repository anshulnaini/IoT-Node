#include <Arduino.h>
#include "ConfigManager.h"
#include "PortalManager.h"
#include "ButtonHandler.h"

#define BUTTON_PIN 10

ConfigManager configManager;
ButtonHandler buttonHandler(BUTTON_PIN);

// --- Non-blocking timer for the main loop ---
unsigned long lastLoopMessageTime = 0;
const long loopMessageInterval = 10000; // Print a message every 10 seconds

void checkForFactoryReset();

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("\n\n--- Booting IoT Node ---");

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
  // These functions must be called as often as possible. The loop is now non-blocking.
  checkForFactoryReset();

  // --- Example of a non-blocking periodic task ---
  unsigned long currentTime = millis();
  if (currentTime - lastLoopMessageTime >= loopMessageInterval) {
    lastLoopMessageTime = currentTime;
    Serial.println("Main loop running...");
    // In the future, other periodic tasks like reading a sensor would go here.
  }
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