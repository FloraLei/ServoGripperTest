#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Arduino.h>
#include <Servo.h>

class ServoController {
private:
  Servo servo;
  int pin;
  int currentAngle;
  int targetAngle;
  int minAngle;
  int maxAngle;
  int speed;  // degrees per update cycle
  unsigned long lastUpdateTime;

public:
  ServoController();

  // Initialize servo on specified pin with angle limits
  void begin(int servoPin, int min, int max);

  // Set target angle (will be clamped to min/max)
  void setTargetAngle(int angle);

  // Get current angle
  int getCurrentAngle();

  // Get target angle
  int getTargetAngle();

  // Check if servo has reached target
  bool isAtTarget();

  // Update servo position (call this in main loop)
  void update();

  // Set movement speed (degrees per update cycle)
  void setSpeed(int degreesPerUpdate);

  // Hold current position (don't move)
  void hold();

  // Get angle limits
  int getMinAngle() { return minAngle; }
  int getMaxAngle() { return maxAngle; }

private:
  // Clamp angle to min/max range
  int clampAngle(int angle);
};

#endif
