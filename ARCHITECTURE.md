# Electric Gripper Control System - Architecture

## Overview
A modular Arduino-based control system for a single-servo two-finger gripper with button-controlled open/close operations and hard angle limits.

## Hardware Setup

### Components
- **Microcontroller**: Arduino (Uno/Nano/Mega)
- **Servo**: Standard PWM servo (e.g., MG995, MG996R, SG90)
- **Buttons**: 2x momentary push buttons (normally open)
- **Power**: 5V for Arduino, 6V+ for servo
- **Optional**: Status LED on pin 13

### Wiring
```
Arduino Pin 9  → Servo Signal (PWM)
Arduino Pin 2  → Button Open (GND on press)
Arduino Pin 3  → Button Close (GND on press)
Arduino GND    → Button common (GND)
Arduino 5V     → Buttons via pull-up resistors (optional, using INPUT_PULLUP)
```

## Software Architecture

### Module Structure

#### 1. **ServoController** (servo_controller.h/cpp)
Handles low-level servo movement and positioning.

**Responsibilities:**
- Attach servo to PWM pin
- Smooth movement from current to target angle
- Enforce min/max angle limits (hard stops)
- Update servo position in main loop

**Key Methods:**
- `begin(pin, min, max)` - Initialize servo
- `setTargetAngle(angle)` - Set desired angle
- `update()` - Move servo one step towards target
- `isAtTarget()` - Check if servo reached target
- `hold()` - Maintain current position

**Implementation Details:**
- Time-based movement (20ms update cycle = 50Hz)
- Speed control via `setSpeed()` (degrees per update)
- Angle clamping to prevent out-of-range commands

---

#### 2. **ButtonHandler** (button_handler.h/cpp)
Debounces button inputs and handles state detection.

**Responsibilities:**
- Read button state with debouncing
- Filter noise (20ms debounce by default)
- Active LOW detection (buttons pull pin to ground)

**Key Methods:**
- `begin(pin)` - Initialize button with internal pull-up
- `isPressed()` - Get debounced button state
- `justPressed()` - Detect button press edge

**Implementation Details:**
- Uses INPUT_PULLUP (internal pull-up resistors)
- 20ms debounce filter
- Active LOW: pin=LOW means button pressed

---

#### 3. **GripperStateMachine** (gripper_state_machine.h/cpp)
Manages gripper operational states and transitions.

**State Diagram:**
```
        ┌─────────────────────────────────┐
        │                                 │
        ▼                                 │
    OPENING ──────► OPEN ◄───┐            │
        │                    │            │
        │    ┌──────────────┘             │
        │    │                            │
        └────┴──────────────────────┐     │
                                    │     │
    CLOSING ◄─────────────────────┐ │     │
        │                         │ │     │
        ▼                         │ │     │
    CLOSED ──────────────────────┘ │     │
        │                          │     │
        └──────────────────────────┴─────┘
                    ▲    ▲
                    │    │
                 HOLDING STATE
                 (preserves position)
```

**States:**
- `STATE_OPENING` - Moving towards open angle
- `STATE_OPEN` - Idle at fully open position
- `STATE_CLOSING` - Moving towards closed angle
- `STATE_CLOSED` - Idle at fully closed position
- `STATE_HOLDING` - Stopped mid-motion, maintaining position

**Control Logic:**
1. **Button Open Pressed** → Request opening
2. **Button Close Pressed** → Request closing
3. **No Button Pressed** → Hold current position

**Hard Limits:**
- Cannot open past `openAngle` (0°)
- Cannot close past `closedAngle` (180°)
- Automatic stop at limits

---

## Control Flow

### Main Loop Sequence (20ms cycle)
```
1. Read button states
   ├─ openPressed = buttonOpen.isPressed()
   └─ closePressed = buttonClose.isPressed()

2. Update gripper request
   ├─ If open: requestOpen()
   ├─ If close: requestClose()
   └─ If neither: requestHold()

3. Update state machine
   ├─ Check state transitions
   ├─ Execute current state logic
   └─ servo.update() moves servo

4. Update status LED

5. Optional debug output
```

## Configuration

### In gripper_control.ino
```cpp
#define SERVO_PIN 9           // PWM output pin
#define BUTTON_OPEN_PIN 2     // Open button input
#define BUTTON_CLOSE_PIN 3    // Close button input
#define LED_STATUS_PIN 13     // Status indicator

#define SERVO_MIN_ANGLE 0     // Fully open
#define SERVO_MAX_ANGLE 180   // Fully closed
#define SERVO_SPEED 30        // degrees per second (adjust for smoothness)
```

### Speed Configuration
- `SERVO_SPEED`: Control how fast gripper moves
  - Lower (10-20): Slow, smooth, less torque required
  - Higher (30-45): Faster, more responsive
  - Adjust based on servo load and desired feel

## Operating Behavior

### Open Mode
1. User presses "Open" button
2. Gripper moves towards 0° angle
3. If reaches 0°, stops (hard limit)
4. While button held, tries to continue opening (but can't go past limit)
5. When button released, gripper holds position

### Close Mode
1. User presses "Close" button
2. Gripper moves towards 180° angle
3. If reaches 180°, stops (hard limit)
4. While button held, tries to continue closing (but can't go past limit)
5. When button released, gripper holds position

### Hold Mode (No Button)
1. When no button is pressed
2. Gripper stops moving
3. Servo maintains torque on current position
4. No power drift

## Troubleshooting

### Gripper won't move
- Check servo is powered separately (5V not enough for many servos)
- Verify servo signal cable connected to pin 9
- Check servo responds in simple test (attach, write angle)

### Gripper moves erratically
- Increase debounce delay: `buttonOpen.setDebounceDelay(50)`
- Check button wiring, may have intermittent contact
- Reduce servo speed if jerking

### Gripper stops before reaching limit
- Servo may be underpowered, increase supply voltage
- Reduce servo speed to give it more torque
- Check for mechanical friction

### Buttons not responding
- Verify pin assignments
- Check INPUT_PULLUP is set in button handler
- Test pins with Serial output in setup()

## Future Enhancements

### Potential Additions
1. **Position Feedback**: Add potentiometer/encoder for actual angle feedback
2. **Force Control**: Add force sensor to stop at grip force, not angle
3. **Multiple Speeds**: Different speeds for different operations
4. **Soft Limits**: Configurable angle limits without physical stops
5. **Reset Procedure**: Initialization sequence for repeatability
6. **UART Control**: Remote commands via serial interface
7. **Status Reporting**: Send gripper state via serial for monitoring

### Safety Features to Add
1. Emergency stop signal
2. Timeout - automatic stop after N seconds of operation
3. Stall detection - stop if servo can't move
4. Current monitoring - detect stuck gripper

## Testing Checklist

- [ ] Servo moves full range when powered
- [ ] Button presses detected (check Serial output)
- [ ] Open button moves to 0°
- [ ] Close button moves to 180°
- [ ] No button = holds position
- [ ] Gripper stops at hard limits
- [ ] Movement is smooth (not jerky)
- [ ] LED status indicates state correctly

