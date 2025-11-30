#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <Arduino.h>

/**
 * @brief Manages the device's power states, primarily handling the transition
 *        into deep sleep and configuring wake-up sources.
 */
class PowerManager {
public:
  /**
   * @brief Construct a new Power Manager object.
   * 
   * @param buttonPin The GPIO pin connected to the wake-up button.
   * @param oledPowerPin The GPIO pin controlling the OLED power transistor.
   * @param sensorPowerPin The GPIO pin controlling the sensor power transistor.
   */
  PowerManager(int buttonPin, int oledPowerPin, int sensorPowerPin);

  /**
   * @brief Turns power on for the peripherals (OLED, sensor).
   */
  void peripherals_on();

  /**
   * @brief Turns power off for the peripherals (OLED, sensor).
   */
  void peripherals_off();

  /**
   * @brief Configures the wakup sources and puts the device into deep sleep.
   * 
   * @param sleepDurationSeconds The number of seconds to sleep before the timer wakes the device.
   */
  void enterDeepSleep(uint32_t sleepDurationSeconds);

private:
  int _buttonPin;
  int _oledPowerPin;
  int _sensorPowerPin;
};

#endif // POWERMANAGER_H
