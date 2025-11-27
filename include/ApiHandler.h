#ifndef APIHANDLER_H
#define APIHANDLER_H

#include "ConfigManager.h"

/**
 * @brief Manages all HTTP communication with the backend server, including
 * device registration and telemetry data submission.
 */
class ApiHandler {
public:
  /**
   * @brief Construct a new Api Handler object.
   * @param configManager A reference to the main ConfigManager instance.
   */
  ApiHandler(ConfigManager& configManager);

  /**
   * @brief Checks if the device has a deviceId. If not, it attempts to register
   * with the server to obtain one.
   * 
   * @return true if the device is registered (or already was).
   * @return false if registration failed.
   */
  bool registerDeviceIfNeeded();

  /**
   * @brief Sends telemetry data (temperature, humidity, battery) to the server.
   * 
   * @param temperature The temperature reading in Celsius.
   * @param humidity The humidity reading in percent.
   * @param battery The battery level in percent.
   * @return true if the data was sent successfully.
   * @return false if sending failed.
   */
  bool sendTelemetry(float temperature, float humidity, float battery);

private:
  ConfigManager& _configManager;
};

#endif // APIHANDLER_H
