# Electric Gripper Control System

Single-servo PWM-based two-finger gripper controller with button-driven open/close operations.

## Quick Start

### Hardware Requirements
- Arduino board (Uno/Nano/Mega)
- 1x PWM servo (standard servo, e.g., MG995)
- 2x momentary push buttons
- Power supply (5V for Arduino, 6V+ for servo)

### Wiring
```
Pin 9 (PWM)  → Servo signal wire
Pin 2        → Open button (to GND on press)
Pin 3        → Close button (to GND on press)
GND          → Common ground for buttons
```

### Installation
1. Copy all `.ino`, `.h`, `.cpp` files to Arduino sketch folder
2. Open `gripper_control.ino` in Arduino IDE
3. Select your board and COM port
4. Upload sketch

### Operation
- **Press Open Button**: Gripper opens (moves to 0°)
- **Press Close Button**: Gripper closes (moves to 180°)
- **Release Button**: Gripper holds position
- **Both Buttons**: Gripper holds (close wins in priority)

## File Structure
```
├── gripper_control.ino          # Main sketch file
├── servo_controller.h/cpp       # Low-level servo control
├── button_handler.h/cpp         # Button debouncing
├── gripper_state_machine.h/cpp  # State machine logic
├── ARCHITECTURE.md              # Detailed system design
├── CONFIG_GUIDE.md              # Configuration instructions
└── README.md                    # This file
```

## Configuration

Edit `gripper_control.ino` to adjust:
```cpp
#define SERVO_PIN 9           // PWM pin
#define BUTTON_OPEN_PIN 2     // Open button pin
#define BUTTON_CLOSE_PIN 3    // Close button pin
#define SERVO_MIN_ANGLE 0     // Open angle
#define SERVO_MAX_ANGLE 180   // Close angle
#define SERVO_SPEED 30        // Movement speed
```

## Serial Debug Output
Connect via USB and open Serial Monitor (9600 baud) to see:
```
State: OPENING | Angle: 45 | OpenBtn: 1 | CloseBtn: 0
State: OPEN | Angle: 0 | OpenBtn: 0 | CloseBtn: 0
State: HOLDING | Angle: 0 | OpenBtn: 0 | CloseBtn: 0
```

## Key Features

✓ **Smooth Movement** - Gradual servo motion (configurable speed)  
✓ **Hard Limits** - Cannot move past min/max angles  
✓ **Position Holding** - Maintains torque when no command  
✓ **Debouncing** - Filters button noise  
✓ **Status LED** - Visual feedback of gripper state  
✓ **Debug Output** - Serial monitoring of state/angles  

## Customization

### Change Angle Limits
To make gripper open more/less, adjust `SERVO_MIN_ANGLE` and `SERVO_MAX_ANGLE`.  
Note: Most servos range 0-180°, but some have wider range.

### Change Movement Speed
Increase `SERVO_SPEED` for faster movement, decrease for slower/more forceful.

### Change Button Pins
Update `BUTTON_OPEN_PIN` and `BUTTON_CLOSE_PIN` to your Arduino pins.

### Adjust Debounce
In `button_handler.cpp`, change `debounceDelay` or call `setDebounceDelay(ms)` in setup().

## Troubleshooting

**Gripper won't respond:**
- Check servo power supply (separate from Arduino 5V)
- Verify pin assignments
- Look at Serial output for state changes

**Servo stutters/jitters:**
- Increase `SERVO_SPEED` value
- Add external power supply for servo

**Buttons not working:**
- Verify button wiring to GND
- Check pin numbers match defines
- Test with Serial.println(digitalRead(pin))

**Servo doesn't reach limits:**
- Servo may be underpowered
- Check for mechanical friction
- Increase movement speed for more torque

## Next Steps

1. **Test Basic Movement**: Hold open/close buttons, watch servo move
2. **Calibrate Angles**: Adjust `SERVO_MIN_ANGLE` and `SERVO_MAX_ANGLE` for your gripper
3. **Tune Speed**: Find best movement speed for your application
4. **Add Force Feedback** (optional): Sense grip force, not just angle

## References

- Arduino Servo Library: https://www.arduino.cc/reference/en/libraries/servo/
- PWM Servo Control: https://learn.adafruit.com/adafruit-16-channel-pwm-servo-driver
