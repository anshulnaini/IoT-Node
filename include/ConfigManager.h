#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>

//truct to hold all the device's configuration data.
struct DeviceConfig {
  // WiFi creds
  char wifiSSID[33];
  char wifiPassword[65];

  // Server Details
  char serverUrl[256];
  char deviceId[33]; // server gives us this

  // Device Details
  char deviceName[33];
  char deviceType[33];
  char locationHint[65];

  // Other params
  int sleepIntervalSeconds;
  bool configured; // check if the device has been set up
};

/**
 * @brief Manages saving, loading, and clearing of the device configuration
 *        to/from Non-Volatile Storage (NVS) using the Preferences library.
 */
class ConfigManager {
public:
  ConfigManager();

  void begin();
  
  // Loads the configuration from NVS into the  config object.
  void loadConfig();

  // Saves the current  config object to NV mem.
  void saveConfig();

  // Clears all configuration from NV mem and resets the  config object.
  void clearConfig();

  // Provides direct, readonly access to the loaded configuration.
  const DeviceConfig& getConfig() const;

  // Allows modification of the configuration before saving.
  DeviceConfig& getMutableConfig();

  // A simple check to see if the device has been configured.
  bool isConfigured();

private:
  DeviceConfig _config;
};

#endif // CONFIGMANAGER_H
