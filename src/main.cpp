#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// Default I2C pins for XIAO ESP32-C3 are:
// SDA: 6
// SCL: 7
#define I2C_SDA 9
#define I2C_SCL 10

Adafruit_AHTX0 aht;

void setup() {
  // Start Serial
  Serial.begin(115200);
  // It's good practice to wait for the serial port to connect, especially on native USB devices.
  while (!Serial) {
    delay(10); 
  }
  Serial.println("Serial connection established.");
  
  // Start I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("I2C bus initialized.");

  // Look for AHT sensor
  if (!aht.begin()) {
    Serial.println("Could not find AHT sensor. Check wiring and I2C pins.");
    while (1) delay(10);
  }
  Serial.println("AHT10 sensor found.");
}

void loop() {
  sensors_event_t humidity, temp;
  // getEvent() will read data from the sensor
  aht.getEvent(&humidity, &temp); 

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println("% rH");

  Serial.println("--------------------");

  delay(2000); // Wait 2 seconds
}
