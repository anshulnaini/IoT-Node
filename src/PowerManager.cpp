#include "PowerManager.h"
#include "esp_sleep.h"

PowerManager::PowerManager(int buttonPin, int oledPowerPin, int sensorPowerPin) 
  : _buttonPin(buttonPin), _oledPowerPin(oledPowerPin), _sensorPowerPin(sensorPowerPin) {
  pinMode(_oledPowerPin, OUTPUT);
  pinMode(_sensorPowerPin, OUTPUT);
}

void PowerManager::peripherals_on() {
  Serial.println("Turning peripherals ON.");
  digitalWrite(_oledPowerPin, HIGH);
  digitalWrite(_sensorPowerPin, HIGH);
}

void PowerManager::peripherals_off() {
  Serial.println("Turning peripherals OFF.");
  digitalWrite(_oledPowerPin, LOW);
  digitalWrite(_sensorPowerPin, LOW);
}

void PowerManager::enterDeepSleep(uint32_t sleepDurationSeconds) {
  Serial.printf("Enabling timer wakeup for %d seconds.\n", sleepDurationSeconds);
  esp_sleep_enable_timer_wakeup(sleepDurationSeconds * 1000000ULL);

  Serial.printf("Enabling wakeup from button on GPIO %d.\n", _buttonPin);
  // For ESP32-C3, we use esp_deep_sleep_enable_gpio_wakeup
  const uint64_t ext_wakeup_pin_mask = 1ULL << _buttonPin;
  const esp_deepsleep_gpio_wake_up_mode_t ext_wakeup_mode = ESP_GPIO_WAKEUP_GPIO_LOW;
  esp_deep_sleep_enable_gpio_wakeup(ext_wakeup_pin_mask, ext_wakeup_mode);

  Serial.println("Entering deep sleep now.");
  // Flush serial buffer before sleeping
  Serial.flush(); 
  
  esp_deep_sleep_start();
}

