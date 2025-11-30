#include "ConfigManager.h"
#include <Preferences.h>

// The namespace for storing the preferences in NV mem
const char* PREFERENCES_NAMESPACE = "iot-node-config";

Preferences preferences;

ConfigManager::ConfigManager() {
  // Initialize with default/empty values
  memset(&_config, 0, sizeof(DeviceConfig));
}

void ConfigManager::begin() {
  // called once in setup
  // false means we open it in r/w mode.
  preferences.begin(PREFERENCES_NAMESPACE, false);
}

void ConfigManager::loadConfig() { // loads config
  _config.configured = preferences.getBool("configured", false);

  if (_config.configured) {
    preferences.getString("wifiSSID", _config.wifiSSID, sizeof(_config.wifiSSID));
    preferences.getString("wifiPassword", _config.wifiPassword, sizeof(_config.wifiPassword));
    preferences.getString("serverUrl", _config.serverUrl, sizeof(_config.serverUrl));
    preferences.getString("deviceId", _config.deviceId, sizeof(_config.deviceId));
    preferences.getString("deviceName", _config.deviceName, sizeof(_config.deviceName));
    preferences.getString("deviceType", _config.deviceType, sizeof(_config.deviceType));
    preferences.getString("locationHint", _config.locationHint, sizeof(_config.locationHint));
    _config.sleepIntervalSeconds = preferences.getInt("sleepInterval", 300);
  }
}

void ConfigManager::saveConfig() { // saves config
  preferences.putBool("configured", _config.configured);
  preferences.putString("wifiSSID", _config.wifiSSID);
  preferences.putString("wifiPassword", _config.wifiPassword);
  preferences.putString("serverUrl", _config.serverUrl);
  preferences.putString("deviceId", _config.deviceId);
  preferences.putString("deviceName", _config.deviceName);
  preferences.putString("deviceType", _config.deviceType);
  preferences.putString("locationHint", _config.locationHint);
  preferences.putInt("sleepInterval", _config.sleepIntervalSeconds);
}

void ConfigManager::clearConfig() { // clearns config
  preferences.clear();
  memset(&_config, 0, sizeof(DeviceConfig)); // Reset struct in NV memory
}

const DeviceConfig& ConfigManager::getConfig() const {
  return _config;
}

DeviceConfig& ConfigManager::getMutableConfig() {
  return _config;
}

bool ConfigManager::isConfigured() {
  // device configured if the flag is set.
  return _config.configured;
}
