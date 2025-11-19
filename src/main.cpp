#include <Arduino.h>
#include "OneButton.h"

#define BUTTON_PIN 10
OneButton button(BUTTON_PIN, true, true);

// --- Non-Blocking State Machine ---
enum ButtonState { S_IDLE, S_DOUBLE_CLICK_PENDING };
ButtonState currentState = S_IDLE;
unsigned long doubleClickStartTime = 0;

// This timeout MUST be longer than the time between the library firing
// a double-click event and a subsequent single-click event (~635ms in tests).
const int TRIPLE_CLICK_TIMEOUT = 700; 

// Forward declaration
void handleSingleClick();
void handleDoubleClick();
void handleLongPress();

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("Pushbutton test: Corrected timeout.");

  button.attachClick(handleSingleClick);
  button.attachDoubleClick(handleDoubleClick);
  button.attachLongPressStop(handleLongPress);
}

void loop() {
  button.tick(); // Keep the button library running

  // Check if a double-click is pending and has timed out
  if (currentState == S_DOUBLE_CLICK_PENDING && (millis() - doubleClickStartTime > TRIPLE_CLICK_TIMEOUT)) {
    Serial.println("Button double-clicked.");
    currentState = S_IDLE; // Reset state
  }
}

// --- Callback Functions ---

void handleSingleClick() {
  if (currentState == S_DOUBLE_CLICK_PENDING) {
    // This is the third click!
    Serial.println("Button triple-clicked.");
    currentState = S_IDLE; // Reset state
  } else {
    // This is a normal single click
    Serial.println("Button single-clicked.");
  }
}

void handleDoubleClick() {
  // A double-click has happened. Start the timer and wait to see if a third click follows.
  currentState = S_DOUBLE_CLICK_PENDING;
  doubleClickStartTime = millis();
}

void handleLongPress() {
  Serial.println("Button long-pressed.");
  // Ensure we cancel any pending double-click state
  currentState = S_IDLE;
}
