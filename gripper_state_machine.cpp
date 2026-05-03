#include "gripper_state_machine.h"

GripperStateMachine::GripperStateMachine()
  : servo(nullptr), currentState(STATE_OPEN),
    requestedState(STATE_HOLDING), openAngle(0),
    closedAngle(180), moveSpeed(5) {
}

void GripperStateMachine::begin(ServoController* servoController, int minAngle, int maxAngle, int speed) {
  servo = servoController;
  openAngle = minAngle;
  closedAngle = maxAngle;
  moveSpeed = speed;

  servo->setSpeed(moveSpeed);
  servo->setTargetAngle(openAngle);
  currentState = STATE_OPEN;
  requestedState = STATE_HOLDING;
}

void GripperStateMachine::requestOpen() {
  requestedState = STATE_OPENING;
}

void GripperStateMachine::requestClose() {
  requestedState = STATE_CLOSING;
}

void GripperStateMachine::requestHold() {
  if (currentState == STATE_OPENING || currentState == STATE_CLOSING) {
    requestedState = STATE_HOLDING;
  }
}

void GripperStateMachine::update() {
  if (servo == nullptr) return;

  transitionState();
  executeState();
  servo->update();
}

void GripperStateMachine::transitionState() {
  switch (currentState) {
    case STATE_OPENING:
      if (servo->isAtTarget()) {
        currentState = STATE_OPEN;
      }
      break;

    case STATE_CLOSING:
      if (servo->isAtTarget()) {
        currentState = STATE_CLOSED;
      }
      break;

    case STATE_OPEN:
      if (requestedState == STATE_CLOSING) {
        currentState = STATE_CLOSING;
      } else if (requestedState == STATE_HOLDING) {
        currentState = STATE_HOLDING;
      }
      break;

    case STATE_CLOSED:
      if (requestedState == STATE_OPENING) {
        currentState = STATE_OPENING;
      } else if (requestedState == STATE_HOLDING) {
        currentState = STATE_HOLDING;
      }
      break;

    case STATE_HOLDING:
      if (requestedState == STATE_OPENING && servo->getCurrentAngle() > openAngle) {
        currentState = STATE_OPENING;
      } else if (requestedState == STATE_CLOSING && servo->getCurrentAngle() < closedAngle) {
        currentState = STATE_CLOSING;
      }
      break;
  }
}

void GripperStateMachine::executeState() {
  switch (currentState) {
    case STATE_OPENING:
      servo->setTargetAngle(openAngle);
      break;

    case STATE_CLOSING:
      servo->setTargetAngle(closedAngle);
      break;

    case STATE_OPEN:
      servo->setTargetAngle(openAngle);
      break;

    case STATE_CLOSED:
      servo->setTargetAngle(closedAngle);
      break;

    case STATE_HOLDING:
      servo->hold();
      break;
  }
}

GripperState GripperStateMachine::getState() {
  return currentState;
}

const char* GripperStateMachine::getStateName() {
  switch (currentState) {
    case STATE_OPENING:
      return "OPENING";
    case STATE_CLOSING:
      return "CLOSING";
    case STATE_OPEN:
      return "OPEN";
    case STATE_CLOSED:
      return "CLOSED";
    case STATE_HOLDING:
      return "HOLDING";
    default:
      return "UNKNOWN";
  }
}
