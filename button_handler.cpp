#include "button_handler.h"

ButtonHandler::ButtonHandler()
  : pin(-1), lastState(HIGH), lastDebounceTime(0),
    debounceDelay(20), isPressedState(false) {
}

void ButtonHandler::begin(int buttonPin) {
  pin = buttonPin;
  pinMode(pin, INPUT_PULLUP);  // Use internal pull-up
  lastState = digitalRead(pin);
}

bool ButtonHandler::isPressed() {
  debounce();
  return isPressedState;
}

bool ButtonHandler::justPressed() {
  static bool prevState = false;
  bool currentState = isPressed();

  if (currentState && !prevState) {
    prevState = currentState;
    return true;
  }

  prevState = currentState;
  return false;
}

void ButtonHandler::setDebounceDelay(int ms) {
  debounceDelay = ms;
}

void ButtonHandler::debounce() {
  int reading = digitalRead(pin);

  if (reading != lastState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    isPressedState = (reading == LOW);  // Active LOW
  }

  lastState = reading;
}
