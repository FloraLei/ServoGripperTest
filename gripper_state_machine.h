#ifndef GRIPPER_STATE_MACHINE_H
#define GRIPPER_STATE_MACHINE_H

#include <Arduino.h>
#include "servo_controller.h"

// State definitions
enum GripperState {
  STATE_OPENING,   // Moving towards open position
  STATE_CLOSING,   // Moving towards closed position
  STATE_OPEN,      // Fully open, idle
  STATE_CLOSED,    // Fully closed, idle
  STATE_HOLDING    // Holding position without movement command
};

class GripperStateMachine {
private:
  ServoController* servo;
  GripperState currentState;
  GripperState requestedState;
  int openAngle;
  int closedAngle;
  int moveSpeed;

public:
  GripperStateMachine();

  // Initialize state machine with servo controller and angle limits
  void begin(ServoController* servoController, int minAngle, int maxAngle, int speed);

  // Request gripper to open
  void requestOpen();

  // Request gripper to close
  void requestClose();

  // Request gripper to hold position
  void requestHold();

  // Update state machine (call in main loop)
  void update();

  // Get current state
  GripperState getState();

  // Get state name for debugging
  const char* getStateName();

private:
  void transitionState();
  void executeState();
};

#endif
