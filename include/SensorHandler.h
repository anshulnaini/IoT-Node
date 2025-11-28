#ifndef SENSORHANDLER_H
#define SENSORHANDLER_H

#include <Adafruit_AHTX0.h>

/**
 * @brief Manages the AHT10 temperature and humidity sensor.
 */
class SensorHandler {
public:
  SensorHandler();

  /**
   * @brief Initializes the sensor.
   * @return true if initialization is successful, false otherwise.
   */
  bool begin();

  /**
   * @brief Reads the temperature from the sensor.
   * @return The temperature in degrees Celsius, or a specific error value if the read fails.
   */
  float readTemperature();

  /**
   * @brief Reads the humidity from the sensor.
   * @return The relative humidity in percent, or a specific error value if the read fails.
   */
  float readHumidity();

private:
  Adafruit_AHTX0 aht;
  sensors_event_t humidity_event;
  sensors_event_t temp_event;
};

#endif // SENSORHANDLER_H
