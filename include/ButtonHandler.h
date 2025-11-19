#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <OneButton.h>

/**
 * @brief Enum to represent the different button events that the main app can query.
 * 
 * EV_NONE is returned when no new event has occurred.
 */
enum ButtonEvent {
  EV_NONE,
  EV_SINGLE_CLICK,
  EV_DOUBLE_CLICK,
  EV_TRIPLE_CLICK,
  EV_LONG_PRESS
};

/**
 * @brief Provides a high-level abstraction for handling complex button events like
 * single, double, triple, and long presses.
 * 
 * HOW TO USE:
 * 1. Create a global instance of this class: 
 *    `ButtonHandler myButton(BUTTON_PIN);`
 * 
 * 2. In the main `setup()` function, call the begin method:
 *    `myButton.begin();`
 * 
 * 3. In the main `loop()` function, you MUST call the tick method on every iteration:
 *    `myButton.tick();`
 * 
 * 4. To check for an event, call getEvent(). This will return the last event and
 *    consume it (it will return EV_NONE on subsequent calls until a new event occurs).
 *    `ButtonEvent event = myButton.getEvent();`
 *    `if (event != EV_NONE) { ... process event ... }`
 */
class ButtonHandler {
public:
  /**
   * @brief Construct a new Button Handler object.
   * @param pin The GPIO pin the button is connected to.
   */
  ButtonHandler(int pin);

  /**
   * @brief Initializes the button and attaches the internal callbacks.
   * Call this in your main setup() function.
   */
  void begin();

  /**
   * @brief Ticks the underlying OneButton library and our state machine.
   * MUST be called in the main loop() on every iteration.
   */
  void tick();

  /**
   * @brief Returns the last detected button event.
   * This is a polling method. It consumes the event, so subsequent calls will
   * return EV_NONE until a new event is detected.
   * @return ButtonEvent The event that occurred, or EV_NONE.
   */
  ButtonEvent getEvent();

private:
  OneButton _button;
  int _pin;

  // --- State machine members ---
  // Note: These are static because the C-style callbacks from OneButton can't access instance members.
  // This works safely under the assumption that we only have one ButtonHandler instance.
  enum ButtonState { S_IDLE, S_DOUBLE_CLICK_PENDING };
  volatile static ButtonState _currentState;
  volatile static unsigned long _doubleClickStartTime;
  volatile static ButtonEvent _lastEvent;

  // --- Static callbacks that will be attached to the button ---
  static void handleSingleClick();
  static void handleDoubleClick();
  static void handleLongPress();
};

#endif // BUTTONHANDLER_H
