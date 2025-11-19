#include <Arduino.h>
#include "ConfigManager.h"

ConfigManager configManager;

void printConfig(const DeviceConfig& config);

void setup() {
  Serial.begin(115200);
  delay(5000); // Wait for Serial to initialize

  Serial.println("\n\n--- ConfigManager Test ---");

  configManager.begin();
  configManager.loadConfig();

  if (configManager.isConfigured()) {
    // --- SECOND RUN ---
    Serial.println("Configuration found in NVS. Loading and printing:");
    const DeviceConfig& config = configManager.getConfig();
    printConfig(config);

    Serial.println("\nClearing configuration for the next test run.");
    configManager.clearConfig();
    Serial.println("Configuration cleared.");

  } else {
    // --- FIRST RUN ---
    Serial.println("No configuration found. Creating and saving a dummy config.");
    DeviceConfig& config = configManager.getMutableConfig();

    // Populate the struct with dummy data
    strcpy(config.wifiSSID, "MyHomeWiFi");
    strcpy(config.wifiPassword, "SecretPassword123");
    strcpy(config.serverUrl, "http://192.168.1.100:3000/api");
    strcpy(config.deviceName, "Living Room Sensor");
    strcpy(config.deviceType, "Temp/Humidity");
    strcpy(config.locationHint, "On the bookshelf");
    config.sleepIntervalSeconds = 600;
    config.configured = true; // Mark as configured

    configManager.saveConfig();
    Serial.println("Dummy configuration saved to NVS.");
    Serial.println("Please reset the device to test loading.");
  }

  Serial.println("--- End of Test ---");
}

void loop() {
  // Nothing to do here for this test.
}

void printConfig(const DeviceConfig& config) {
  Serial.println("---------------------------------");
  Serial.print("WiFi SSID: "); Serial.println(config.wifiSSID);
  Serial.print("WiFi Password: "); Serial.println(config.wifiPassword);
  Serial.print("Server URL: "); Serial.println(config.serverUrl);
  Serial.print("Device ID: "); Serial.println(config.deviceId);
  Serial.print("Device Name: "); Serial.println(config.deviceName);
  Serial.print("Device Type: "); Serial.println(config.deviceType);
  Serial.print("Location Hint: "); Serial.println(config.locationHint);
  Serial.print("Sleep Interval: "); Serial.print(config.sleepIntervalSeconds); Serial.println("s");
  Serial.print("Is Configured: "); Serial.println(config.configured ? "true" : "false");
  Serial.println("---------------------------------");
}
