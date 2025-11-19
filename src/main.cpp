#include <Arduino.h>
#include "ButtonHandler.h"

// Define the pin the button is connected to.
#define BUTTON_PIN 10

// Create an instance of our button handler.
ButtonHandler buttonHandler(BUTTON_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("Button handler abstraction test.");
  
  // Initialize the button handler.
  buttonHandler.begin();
}

void loop() {
  // We must call the tick() method in every loop.
  buttonHandler.tick();

  // Get the latest event from the handler.
  ButtonEvent event = buttonHandler.getEvent();

  // Process the event.
  switch (event) {
    case EV_SINGLE_CLICK:
      Serial.println("Event: Single Click");
      break;
    case EV_DOUBLE_CLICK:
      Serial.println("Event: Double Click");
      break;
    case EV_TRIPLE_CLICK:
      Serial.println("Event: Triple Click");
      break;
    case EV_LONG_PRESS:
      Serial.println("Event: Long Press");
      break;
    
    case EV_NONE:
    default:
      // No event occurred in this loop.
      break;
  }
}