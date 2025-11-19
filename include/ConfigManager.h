#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>

// A struct to hold all the device's configuration data.
struct DeviceConfig {
  // WiFi Credentials
  char wifiSSID[33];
  char wifiPassword[65];

  // Server Details
  char serverUrl[256];
  char deviceId[33]; // Assigned by the server after registration

  // Device Details
  char deviceName[33];
  char deviceType[33];
  char locationHint[65];

  // Operational Parameters
  int sleepIntervalSeconds;
  bool configured; // A flag to check if the device has been set up
};

/**
 * @brief Manages saving, loading, and clearing of the device configuration
 *        to/from Non-Volatile Storage (NVS) using the Preferences library.
 */
class ConfigManager {
public:
  ConfigManager();

  void begin();
  
  // Loads the configuration from NVS into the internal config object.
  void loadConfig();

  // Saves the current internal config object to NVS.
  void saveConfig();

  // Clears all configuration from NVS and resets the internal config object.
  void clearConfig();

  // Provides direct, read-only access to the loaded configuration.
  const DeviceConfig& getConfig() const;

  // Allows modification of the configuration before saving.
  DeviceConfig& getMutableConfig();

  // A simple check to see if the device has been configured.
  bool isConfigured();

private:
  DeviceConfig _config;
};

#endif // CONFIGMANAGER_H
