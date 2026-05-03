# Configuration Guide

## Pin Configuration

### Update Pin Assignments
Open `gripper_control.ino` and modify:

```cpp
// Hardware Pins
#define SERVO_PIN 9           // PWM pin for servo
#define BUTTON_OPEN_PIN 2     // Button to open gripper
#define BUTTON_CLOSE_PIN 3    // Button to close gripper
#define LED_STATUS_PIN 13     // Status LED (optional)
```

**Common Arduino Pins:**
- PWM capable: 3, 5, 6, 9, 10, 11
- Digital I/O: 2-13, A0-A5
- All pins support INPUT_PULLUP for buttons

---

## Servo Configuration

### Angle Range
```cpp
#define SERVO_MIN_ANGLE 0     // Fully open position
#define SERVO_MAX_ANGLE 180   // Fully closed position
```

**Calibration:**
1. Attach servo to gripper mechanism
2. Set `SERVO_MIN_ANGLE = 0`, upload and test
3. Adjust until gripper is fully open
4. Record the optimal open angle
5. Repeat for close position

**Common Servo Ranges:**
- Standard servo: 0-180°
- Extended range servo: -90 to +270° (use 0-180 mapped)
- Continuous rotation: Not suitable (lacks position feedback)

### Movement Speed
```cpp
#define SERVO_SPEED 30        // Movement speed (degrees per second)
```

**Speed Tuning:**
- **10-15°/sec**: Slow, smooth, high torque
- **20-30°/sec**: Moderate, balanced
- **40-60°/sec**: Fast, needs higher power
- **100+°/sec**: Very fast, may cause jitter

**Formula:** Update cycle is 20ms, so:
- Speed 30 = 30 * (20/1000) = 0.6° per update

**Power Requirement:**
- Slower speeds need less instantaneous current
- If servo stutters, reduce speed or increase power supply capacity

---

## Button Configuration

### Debounce Settings
In `button_handler.cpp`, line ~28:
```cpp
unsigned long debounceDelay;
debounceDelay = 20;  // milliseconds
```

Or dynamically in `setup()`:
```cpp
buttonOpen.setDebounceDelay(50);   // More filtering
buttonClose.setDebounceDelay(50);
```

**Debounce Values:**
- 20ms: Standard, works for most switches
- 50ms: Extra filtering for noisy switches
- 10ms: Faster response, less filtering

### Button Logic
Current implementation: **Active LOW**
- Button pressed = Pin pulled to GND = LOW = active
- Uses Arduino `INPUT_PULLUP` (internal resistor)

To use external pull-up resistors instead:
1. Connect 10kΩ resistor from pin to 5V
2. Change in `button_handler.cpp`:
```cpp
pinMode(pin, INPUT);  // Remove INPUT_PULLUP
```

---

## Gripper State Machine Tuning

### State Transition Thresholds
In `servo_controller.cpp`, line ~48:
```cpp
bool ServoController::isAtTarget() {
  return abs(currentAngle - targetAngle) <= 2;  // tolerance
}
```

**Tolerance Values:**
- 2°: Strict, waits for exact position (default)
- 5°: Looser, faster transitions
- Use lower values for precision, higher for speed

---

## Advanced Configuration

### Safe Operating Ranges

#### Power Supply
- Arduino: 5V 500mA minimum
- Servo: 6V minimum (most run 5-7.4V)
- **Recommendation**: Separate power supplies
  - Arduino: 5V USB
  - Servo: 6V battery or regulated supply

#### Temperature
- Operating: 0-50°C
- Servo performance deglines at extremes
- If running continuously, add cooling

#### Load
- Match servo torque to gripper weight
- Light gripper (< 500g): Standard servo OK
- Heavy gripper: Consider high-torque servo (MG996R, etc.)

### Calibration Procedure

**Step 1: Find Open Angle**
```cpp
#define SERVO_MIN_ANGLE 90
#define SERVO_MAX_ANGLE 90   // Lock to one angle
// Upload, observe gripper position
// Adjust SERVO_MIN_ANGLE until fully open
```

**Step 2: Find Close Angle**
```cpp
#define SERVO_MIN_ANGLE 90   // Lock to open position
#define SERVO_MAX_ANGLE 90
// Adjust SERVO_MAX_ANGLE until fully closed
```

**Step 3: Verify Range**
```cpp
#define SERVO_MIN_ANGLE [open_angle]
#define SERVO_MAX_ANGLE [close_angle]
// Test full open/close cycle
```

**Step 4: Record Values**
```cpp
// Save these calibration values
#define GRIPPER_OPEN_ANGLE 15    // Adjust to your mechanism
#define GRIPPER_CLOSE_ANGLE 165  // Adjust to your mechanism
```

### Performance Tuning

#### Smooth Operation
- Increase `SERVO_SPEED` slightly if servo jitters
- Decrease if position overshoots target
- Ideal: Smooth motion, arrives exactly at target

#### Responsive Control
- Decrease debounce for faster button response
- Increase if buttons seem "stuck"
- Test with Serial output to confirm button reads

#### Power Efficiency
- Lower `SERVO_SPEED` uses less peak current
- Shorter operation time reduces heat
- Monitor servo temperature if running long operations

---

## Testing Configuration

### Serial Monitor Test
Upload and open Serial Monitor (9600 baud):
```
State: OPENING | Angle: 45 | OpenBtn: 1 | CloseBtn: 0
State: OPEN | Angle: 0 | OpenBtn: 0 | CloseBtn: 0
State: HOLDING | Angle: 0 | OpenBtn: 0 | CloseBtn: 0
State: CLOSING | Angle: 135 | OpenBtn: 0 | CloseBtn: 1
State: CLOSED | Angle: 180 | OpenBtn: 0 | CloseBtn: 0
```

### Manual Servo Test
Comment out main code, test basic servo:
```cpp
#include <Servo.h>
Servo myservo;

void setup() {
  myservo.attach(9);
  myservo.write(0);     // Test open
  delay(2000);
  myservo.write(180);   // Test close
  delay(2000);
}

void loop() {
  // Cycle through positions
  for(int i=0; i<=180; i+=10) {
    myservo.write(i);
    delay(100);
  }
  for(int i=180; i>=0; i-=10) {
    myservo.write(i);
    delay(100);
  }
}
```

---

## Troubleshooting Configuration

| Issue | Check | Solution |
|-------|-------|----------|
| No servo movement | Servo power supply | Verify 6V+ connected |
| Servo twitches | Power supply noise | Add capacitor (100µF) near servo |
| Button doesn't work | Pin definition | Verify `BUTTON_*_PIN` numbers |
| Gripper slow | SERVO_SPEED too low | Increase value |
| Gripper overshoots | isAtTarget tolerance | Increase tolerance threshold |
| Buttons debounce wrong | debounceDelay | Adjust 20ms to 50ms or higher |

