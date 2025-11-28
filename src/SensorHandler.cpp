#include "SensorHandler.h"

SensorHandler::SensorHandler() {
  // Constructor
}

bool SensorHandler::begin() {
  if (!aht.begin()) {
    Serial.println("Failed to find AHT10 sensor");
    return false;
  }
  Serial.println("AHT10 sensor found");
  return true;
}

float SensorHandler::readTemperature() {
  aht.getEvent(&humidity_event, &temp_event); // Populate temp and humidity objects
  return temp_event.temperature;
}

float SensorHandler::readHumidity() {
  // The getEvent call in readTemperature already populated the humidity object,
  // but we can call it again to be safe in case the methods are called out of order.
  aht.getEvent(&humidity_event, &temp_event);
  return humidity_event.relative_humidity;
}
