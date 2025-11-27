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
   */
  PowerManager(int buttonPin);

  /**
   * @brief Configures the wake-up sources and puts the device into deep sleep.
   * 
   * The device can be woken up by two sources:
   * 1. A timer expiring after the specified duration.
   * 2. The external wake-up button being pressed.
   * 
   * @param sleepDurationSeconds The number of seconds to sleep before the timer wakes the device.
   */
  void enterDeepSleep(uint32_t sleepDurationSeconds);

private:
  int _buttonPin;
};

#endif // POWERMANAGER_H
