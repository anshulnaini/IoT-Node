#include <Arduino.h>
#include <OLEDHandler.h>



OLEDHandler oled(9, 10);


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup started");

  oled.initializeOLED();
  
  oled.displayText("Hello, World!");
}

void loop() {
  
}
