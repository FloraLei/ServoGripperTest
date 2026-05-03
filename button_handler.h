#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

class ButtonHandler {
private:
  int pin;
  int lastState;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;
  bool isPressedState;

public:
  ButtonHandler();

  // Initialize button on specified pin (active LOW)
  void begin(int buttonPin);

  // Check if button is currently pressed (debounced)
  bool isPressed();

  // Check if button was just pressed (edge detection)
  bool justPressed();

  // Set debounce delay in milliseconds
  void setDebounceDelay(int ms);

private:
  void debounce();
};

#endif
