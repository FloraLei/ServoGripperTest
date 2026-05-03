#include "servo_controller.h"

ServoController::ServoController()
  : currentAngle(90), targetAngle(90), minAngle(0),
    maxAngle(180), speed(5), lastUpdateTime(0) {
}

void ServoController::begin(int servoPin, int min, int max) {
  pin = servoPin;
  minAngle = min;
  maxAngle = max;
  currentAngle = (min + max) / 2;  // Start at middle position
  targetAngle = currentAngle;

  servo.attach(pin);
  servo.write(currentAngle);
}

void ServoController::setTargetAngle(int angle) {
  targetAngle = clampAngle(angle);
}

int ServoController::getCurrentAngle() {
  return currentAngle;
}

int ServoController::getTargetAngle() {
  return targetAngle;
}

bool ServoController::isAtTarget() {
  return abs(currentAngle - targetAngle) <= 2;  // 2 degree tolerance
}

void ServoController::update() {
  unsigned long now = millis();

  // Update based on time passed for smooth movement
  if (now - lastUpdateTime >= 20) {  // 50Hz update
    if (currentAngle < targetAngle) {
      currentAngle += speed;
      if (currentAngle > targetAngle) {
        currentAngle = targetAngle;
      }
    } else if (currentAngle > targetAngle) {
      currentAngle -= speed;
      if (currentAngle < targetAngle) {
        currentAngle = targetAngle;
      }
    }

    servo.write(currentAngle);
    lastUpdateTime = now;
  }
}

void ServoController::setSpeed(int degreesPerUpdate) {
  speed = degreesPerUpdate;
}

void ServoController::hold() {
  targetAngle = currentAngle;
}

int ServoController::clampAngle(int angle) {
  if (angle < minAngle) return minAngle;
  if (angle > maxAngle) return maxAngle;
  return angle;
}
