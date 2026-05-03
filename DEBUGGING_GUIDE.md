# Electric Gripper Control System - Debugging Guide

## Table of Contents
1. [Hardware Connection Verification](#hardware-connection-verification)
2. [Component-by-Component Testing](#component-by-component-testing)
3. [Sensor Calibration](#sensor-calibration)
4. [Common Issues and Solutions](#common-issues-and-solutions)
5. [Performance Testing](#performance-testing)
6. [Log Analysis](#log-analysis)
7. [Emergency Troubleshooting](#emergency-troubleshooting)

---

## Hardware Connection Verification

### Initial Power-Up Check
1. **Before applying power:**
   - Verify all connections are correct using HARDWARE_ARCHITECTURE.md
   - Check for loose wires or cold solder joints
   - Ensure servo power supply is separate from Arduino (not USB-powered)
   - Confirm GND is shared between Arduino and external power supply

2. **During power-up:**
   - Listen for servo initialization (slight movement or buzz)
   - Check if INA219 LED indicator is lit (if present)
   - Verify serial monitor opens at 9600 baud
   - Look for the banner message in serial output

### Quick Connection Test
```
Expected Serial Output:
==========================================
  Electric Gripper Control System v2.0
==========================================
✓ Servo: PIN 8 (Wheeltech HW020 270°)
✓ INA219: I2C (A4/A5)
✓ Switch: PIN 2/3/4 (3-position)
✓ Vibration: PIN 6
✓ Power: External 5V 3A
==========================================

✓ INA219 initialized
```

If you see `✗ INA219 not found!`, see [INA219 Not Responding](#ina219-not-responding).

---

## Component-by-Component Testing

### 1. Servo Motor Control

#### Test 1: Basic Movement
```
Serial Command:  go 0
Expected Result: Servo moves to open position, banner shows:
                 → Moving to 0° | Current: X mA

Serial Command:  go 270
Expected Result: Servo moves to close position (smooth movement)
```

**If servo doesn't move:**
- Check PIN 8 connection to servo signal wire (white wire)
- Verify servo power supply (red wire) is 5V and has adequate current
- Check GND connection (black wire)
- Try: `go 135` (middle position)

**If servo moves but slowly:**
- Check voltage at servo power supply (should be stable 5V)
- Reduce current draw by other components
- Normal speed depends on `moveSpeed` setting (default 5°/100ms)

**If servo jitters or stutters:**
- Add a 100µF capacitor across servo power supply
- Ensure servo power supply has good current capacity (3A minimum)
- Check for EMI interference from vibration motor - separate power lines if possible

#### Test 2: Angle Accuracy
```
Serial Commands:  go 0, go 45, go 90, go 135, go 180, go 225, go 270
Check:           Servo reaches each position smoothly with no overshoot
Measure:         Manually check angle with protractor (±5° acceptable)
```

If angles are off by >10°:
- Run servo calibration with `simple_pulse_test.ino`
- Check if servo has mechanical play or gear wear
- Verify servo is at neutral before initial move

#### Test 3: Speed Adjustment
```
Serial Command:  set speed 1
Then:            go 270
Expected:        Slow movement (~1°/100ms)

Serial Command:  set speed 10
Then:            go 0
Expected:        Fast movement (~10°/100ms)
```

If speed doesn't change, verify settings were saved:
```
Serial Command:  config
Check:           "Movement speed: X°/100ms"
```

---

### 2. INA219 Current Sensor

#### Test 1: Basic Current Reading
```
Serial Command:  current
Expected Output: Current: 50-100 mA (at rest, no load)
```

**If shows "Current: 0 mA":**
- Check I2C connection (SCL on A5, SDA on A4)
- Verify 5V power to INA219 module
- Check for loose GND connection
- Inspect for burned components on INA219 board

**If shows erratic values (jumps wildly):**
- Add 0.1µF ceramic capacitor on INA219 power supply
- Ensure power supply is stable under load
- Check for loose connections causing intermittent contact

#### Test 2: Current During Movement
```
Serial Command:  config            (note current threshold)
Serial Command:  go 270             (closes gripper)
Serial Command:  log
Check:           Current increases as servo reaches resistance
Expected:        Gradual increase from ~80mA to 200-400mA during load
```

**If current spikes above threshold:**
```
Check in log:    "⚡ Current spike detected! Delta: XXX mA"
Verify:          Vibration motor activates (should feel vibration)
```

#### Test 3: Current Threshold Adjustment
```
Serial Command:  set threshold 250  (lower threshold)
Serial Command:  go 270
Expected:        Spike triggered sooner when closing

Serial Command:  set threshold 500  (higher threshold)
Serial Command:  go 0
Serial Command:  go 270
Expected:        Spike only triggered with heavy load
```

---

### 3. Three-Position Switch

#### Test 1: Open Position (PIN 2)
```
Action:          Press and release open switch
Expected Output: 🔓 Switch: Open
                 🔓 Opening gripper...
                 → Moving to 0° | Current: X mA
```

**If nothing happens:**
- Check PIN 2 is connected to switch position 1
- Verify switch makes contact (test with multimeter)
- Confirm switch has pull-up resistor or INPUT_PULLUP is used
- Check for debouncing issues (hold switch longer)

#### Test 2: Close Position (PIN 3)
```
Action:          Press and release close switch
Expected Output: 🔒 Switch: Close
                 🔒 Closing gripper...
                 → Moving to 270° | Current: X mA
```

#### Test 3: Hold Position (PIN 4)
```
Action:          Release gripper first (serial: open)
Serial Command:  go 135              (manual move to middle)
Action:          Press hold switch
Expected Output: ⏸ Switch: Hold
                 ⏸ Stopping gripper
Behavior:        Gripper stays at 135°, doesn't move further
```

#### Test 4: Switch Debouncing
```
Action:          Tap switch very quickly (multiple times)
Expected Output: Should only register once per press
                 (If registers multiple times, debounce delay is too short)
```

---

### 4. Vibration Motor Feedback

#### Test 1: Manual Trigger
```
Serial Command:  test vibrate
Expected Output: 📳 Vibration started
Duration:        Motor vibrates for 200ms (default)
                 📳 Vibration stopped

Feel:            Noticeable vibration/buzzing
```

**If no vibration:**
- Check PIN 6 connection to vibration motor
- Verify motor has power (5V)
- Check GND connection
- Test motor with direct 5V connection (bypass Arduino to isolate issue)

**If weak vibration:**
- Check power supply voltage (should be 5V, not 4.5V)
- Verify current capacity (vibration motor can draw 500mA+)
- Ensure adequate power supply capacity

#### Test 2: Current Spike Trigger
```
Serial Command:  set threshold 150   (lower for testing)
Serial Command:  go 270              (close gripper)
Manually apply:  External pressure on gripper
Expected Output: ⚡ Current spike detected!
                 📳 Vibration started
                 [motor vibrates]
                 📳 Vibration stopped
```

#### Test 3: Vibration Duration Configuration
```
Serial Command:  set vibrate 50      (very short)
Serial Command:  test vibrate
Check:           Vibrates for ~50ms

Serial Command:  set vibrate 500     (longer)
Serial Command:  test vibrate
Check:           Vibrates for ~500ms
```

---

## Sensor Calibration

### Servo Calibration (if angles are inaccurate)

1. **Upload `simple_pulse_test.ino`:**
   - Tests pulse widths: 500, 1050, 1550, 2100, 2600 µs
   - Hold each position for 5 seconds
   - Measure actual angle with protractor

2. **If measured angles differ from expected:**
   - Note the pulse values and actual angles
   - Update `getPulseForAngle()` lookup table in `gripper_main.ino`
   - Retest with `go` command

3. **Upload `fine_search.ino` for precision:**
   - Tests 1400-1600 µs range in 10 µs steps
   - Find exact pulse boundaries for your servo
   - Update PULSE_MIN and PULSE_MAX constants

### INA219 Calibration

**Check internal resistance:**
```
1. Disconnect load (gripper not gripping)
2. Serial Command: current
3. Note baseline current (should be 0-50 mA with no movement)

4. If baseline is >100 mA:
   - Check for shorts in wiring
   - Verify servo isn't held against mechanical stop
   - Test servo motor isolation
```

**Check measurement accuracy:**
```
1. Use multimeter to measure actual voltage across load
2. Compare with INA219 reading
3. If difference >10%, adjust INA219 configuration (see datasheet)
```

---

## Common Issues and Solutions

### Issue 1: Servo Only Moves 3-5°
**Symptom:** Servo moves but range is extremely limited
**Root Cause:** Integer division precision loss in angle calculation
**Solution:** Already fixed in current version - uses lookup table instead of formula
**Verify:** Check that `getPulseForAngle()` is being used, not old formula

### Issue 2: INA219 Not Responding
**Symptom:** Serial output shows `✗ INA219 not found!`
**Troubleshooting:**
```
1. Check I2C Address:
   - Default INA219 address: 0x40
   - If different, modify: ina219.begin(0x4F) with correct address

2. Test I2C Connection:
   Upload i2c_scanner sketch (standard Arduino example)
   Look for address 0x40 in results

3. Check Physical Connections:
   - SCL: A5 (white wire)
   - SDA: A4 (green wire)
   - GND: GND (black wire)
   - 5V:  5V (red wire)

4. Try Different I2C Pins:
   If using Arduino Mega: SCL=21, SDA=20 (modify Wire.begin())

5. Verify Adafruit Library:
   - Check version: Sketch → Include Library → Manage Libraries
   - Update to latest if >6 months old
```

### Issue 3: Servo Jitters or Trembles
**Symptom:** Servo shakes while holding position
**Causes & Solutions:**
```
1. Inadequate Power Supply:
   - Upgrade to 5V 3A or higher
   - Add 470µF + 0.1µF capacitors on servo power

2. Noisy I2C Lines:
   - Use shielded wire for I2C
   - Keep I2C wires away from servo power wires
   - Add 4.7kΩ pull-up resistors if not present

3. USB Noise Interference:
   - Power Arduino from external 5V supply, not USB
   - Add ferrite core on USB cable if USB must stay connected

4. PWM Frequency Conflict:
   - Avoid using PIN 9 with PWM for other components
   - Keep vibration motor power separate
```

### Issue 4: Switch Not Responding
**Symptom:** Pressing switch does nothing
**Troubleshooting:**
```
1. Test Physical Switch:
   - Set multimeter to continuity mode
   - Press each position, should hear/see contact

2. Test Arduino Pin:
   - Upload BasicDigitalRead sketch
   - Monitor PIN 2/3/4 values when pressing
   - Should go LOW (0) when pressed

3. Check Debounce:
   - Increase delay(50) in checkSwitches() if too sensitive
   - Verify pull-up resistors are present

4. Reset EEPROM:
   - Upload gripper_main.ino
   - Serial Command: set open 0
   - Serial Command: set close 270
   - This forces EEPROM reset
```

### Issue 5: Current Reading Shows 0 or Negative
**Symptom:** `current` command returns 0 or very negative values
**Troubleshooting:**
```
1. Check INA219 Power:
   - Multimeter on INA219 5V and GND pins
   - Should show 5V stable

2. Check Shunt Connection:
   - INA219 measures voltage across tiny shunt resistor
   - Ensure servo power flows through shunt terminals
   - Not around the sensor

3. Verify Current Direction:
   - Current flows IN+ to IN- in normal operation
   - Reverse connection reads as negative

4. Check for Shorts:
   - Disconnect servo
   - Test current with just passive load (resistor)
```

---

## Performance Testing

### Load Test
```
Setup:        Grip an object with known weight
Commands:     set threshold 250
              go 270
              log
```

**Check Results:**
- Current increases smoothly as gripper closes
- Spike triggers when object is fully gripped
- Motor vibrates to signal object detected
- No jerking or oscillation

### Speed Test
```
Measure time for full range (0° → 270°):
Command:      go 270
Time:         Should be ~5.4 seconds at speed 5
              (270° ÷ 5°/100ms = 5400ms)

Modify speed: set speed 10
Retest:       Should be ~2.7 seconds now
```

### Endurance Test
```
Repeat 100 times:
- Open gripper (go 0)
- Close gripper (go 270)
- Measure final state

Check For:
- Any servo wear or degradation
- Memory leaks (RAM usage should be stable)
- EEPROM corruption (settings unchanged)
- Motor heating (touch servo to check temp)
```

---

## Log Analysis

### Viewing the Log
```
Serial Command:  log
Output Format:   Time(s) | Angle | Current(mA) | Action
Example:
Time(s) | Angle | Current(mA) | Action
--------|-------|-------------|------------------
5       | 0°    | 85          | open
12      | 270°  | 410         | spike_detected
18      | 270°  | 95          | close
```

### What to Look For

**1. Current Progression Pattern:**
```
✓ Normal:    85 → 120 → 150 → 200 → 350 (smooth increase, then spike)
✗ Problem:   400 → 300 → 500 → 100 (erratic, missing spike threshold)
```

**2. Spike Detection Timing:**
```
✓ Correct:   Current spike detected at 300mA when gripper reaches resistance
✗ Problem:   Spike detected with 0 load / Not detected with full load
Solution:    Adjust threshold with: set threshold XXX
```

**3. Action Sequence:**
```
✓ Logical:   open → move → spike_detected → stop
✗ Wrong:     spike_detected (with no load) → move → stop
```

### Clearing Old Logs
```
Serial Command:  clear log
Output:          ✓ Logs cleared
Verify:          log
                 (should show empty log)
```

---

## Emergency Troubleshooting

### Servo Not Responding (Nothing Works)

**Step 1: Verify Power**
```
Multimeter across servo power (red/black):
- Should show 5V stable
- Not just 4V or fluctuating
```

**Step 2: Test Servo Directly**
```
Upload simple_pulse_test.ino to Arduino
- If servo moves: controller issue
- If servo doesn't move: servo or power issue
```

**Step 3: Check Signal Line**
```
Multimeter on PIN 8 (signal line):
- Should see pulse signal (varying voltage 0-5V)
- Use oscilloscope if available
```

### Multiple Components Not Working

**Complete Reset Procedure:**
```
1. Disconnect ALL power
2. Wait 30 seconds
3. Disconnect servo signal wire from PIN 8
4. Reconnect power to Arduino (USB)
5. Open serial monitor
6. Verify banner appears
7. Reconnect servo signal
8. Test: go 90
```

### Unexpected Behavior After Code Update

**Factory Reset:**
```
1. Upload any sketch (even blank)
2. Disconnect Arduino from power
3. Remove external power from servo
4. Wait 1 minute
5. Reconnect power
6. Upload gripper_main.ino
7. Reset EEPROM: set open 0, set close 270
```

---

## Serial Command Reference for Debugging

| Command | Purpose | Example Output |
|---------|---------|-----------------|
| `pos` | Show current angle | `Angle: 135° \| Current: 95 mA` |
| `current` | Show current reading | `Current: 120 mA` |
| `config` | Show all settings | Display all parameters |
| `log` | Show action history | Table of all actions |
| `clear log` | Clear log history | `✓ Logs cleared` |
| `test vibrate` | Test vibration motor | Vibrates for configured duration |
| `go <angle>` | Move to angle | `→ Moving to 180° \| Current: 95 mA` |
| `open` | Move to open position | `🔓 Opening gripper...` |
| `close` | Move to close position | `🔒 Closing gripper...` |
| `set threshold <mA>` | Set spike threshold | `✓ Current threshold set to 250 mA` |
| `set speed <1-10>` | Set movement speed | `✓ Speed set to 3°/100ms` |
| `help` | Show all commands | Complete command menu |

---

## When to Contact Support

If after completing all debugging steps you still have issues:
1. Run complete log with timestamps
2. Note exact symptoms and steps to reproduce
3. Check Arduino IDE version (recommend 1.8.19+)
4. Verify all hardware connections match HARDWARE_ARCHITECTURE.md
5. Confirm Adafruit_INA219 library is latest version
