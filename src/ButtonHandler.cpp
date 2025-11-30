#include "ButtonHandler.h"
#include <Arduino.h>

//static members
volatile ButtonHandler::ButtonState ButtonHandler::_currentState = S_IDLE;
volatile unsigned long ButtonHandler::_doubleClickStartTime = 0;
volatile ButtonEvent ButtonHandler::_lastEvent = EV_NONE;

// Timeout for detecting a triple click after a double click has occurred.
const int TRIPLE_CLICK_TIMEOUT = 700;

// Public Methods

ButtonHandler::ButtonHandler(int pin) 
  : _button(pin, true, true), _pin(pin) {
}

void ButtonHandler::begin() {
  _button.attachClick(handleSingleClick);
  _button.attachDoubleClick(handleDoubleClick);
  _button.attachLongPressStop(handleLongPress);
}

void ButtonHandler::tick() {
  _button.tick();

  // The main loop tick handles the timeout for deciding a doubleclick.
  if (_currentState == S_DOUBLE_CLICK_PENDING && (millis() - _doubleClickStartTime > TRIPLE_CLICK_TIMEOUT)) {
    _lastEvent = EV_DOUBLE_CLICK;
    _currentState = S_IDLE;
  }
}

ButtonEvent ButtonHandler::getEvent() {
  // This function is polled by the main code. It returns the last event and consumes it.
  ButtonEvent event = _lastEvent;
  if (event != EV_NONE) {
    _lastEvent = EV_NONE;
  }
  return event;
}



void ButtonHandler::handleSingleClick() {
  if (_currentState == S_DOUBLE_CLICK_PENDING) {
    // A single click occurred while a doubleclick was pending, so it's a triple click.
    _lastEvent = EV_TRIPLE_CLICK;
    _currentState = S_IDLE;
  } else {
    // normal single click.
    _lastEvent = EV_SINGLE_CLICK;
  }
}

void ButtonHandler::handleDoubleClick() {
  // A doubleclick  happened. Start the timer and wait to see if a third click happens too
  _currentState = S_DOUBLE_CLICK_PENDING;
  _doubleClickStartTime = millis();
}

void ButtonHandler::handleLongPress() {
  _lastEvent = EV_LONG_PRESS;
  //cancel any pending double click state
  _currentState = S_IDLE;
}
