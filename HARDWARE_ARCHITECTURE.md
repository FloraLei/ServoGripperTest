# Electric Gripper Control System - Hardware Architecture & Wiring Guide

## 📋 Table of Contents
1. [System Overview](#system-overview)
2. [Hardware Components](#hardware-components)
3. [PIN Assignment](#pin-assignment)
4. [Wiring Diagram](#wiring-diagram)
5. [Detailed Connections](#detailed-connections)
6. [Power Distribution](#power-distribution)
7. [Software Architecture](#software-architecture)
8. [Data Flow](#data-flow)

---

## System Overview

A complete electric gripper control system combining:
- **Servo Motor Control** - Wheeltech HW020 270° servo
- **Current Monitoring** - INA219 I2C current sensor
- **User Input** - 3-position manual switch
- **Haptic Feedback** - Vibration motor triggered by current spikes
- **Data Logging** - Complete angle and current history

### Block Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      Arduino UNO/Nano                       │
│                                                              │
│  ┌────────────┐  ┌──────────┐  ┌─────────────────┐        │
│  │  Servo     │  │ INA219   │  │ Switch Input    │        │
│  │ Controller │  │ Monitor  │  │ (3-position)    │        │
│  │  (PIN 8)   │  │(I2C)     │  │ (PIN 2/3/4)     │        │
│  └────────────┘  └──────────┘  └─────────────────┘        │
│         │              │              │                     │
│  ┌──────v──────────────v──────────────v──────┐             │
│  │      Main Control Logic & State Machine     │             │
│  │      - Angle calculation                   │             │
│  │      - Speed control                       │             │
│  │      - Current spike detection             │             │
│  │      - Logging system                      │             │
│  └──────┬──────────────┬──────────────────────┘             │
│         │              │                                     │
│  ┌──────v──────┐  ┌────v─────────┐                          │
│  │Vibration    │  │EEPROM Config │                          │
│  │Motor Output │  │Storage       │                          │
│  │  (PIN 6)    │  │              │                          │
│  └─────────────┘  └──────────────┘                          │
└─────────────────────────────────────────────────────────────┘
```

---

## Hardware Components

| Component | Model | Function | Quantity |
|-----------|-------|----------|----------|
| Microcontroller | Arduino UNO/Nano | Main control | 1 |
| Servo Motor | Wheeltech HW020 270° | Gripper actuation | 1 |
| Current Sensor | INA219 | Current monitoring | 1 |
| 3-Position Switch | Manual switch | User control | 1 |
| Vibration Motor | Small motor/buzzer | Haptic feedback | 1 |
| Power Supply | 5V 3A | Servo power | 1 |
| USB Cable | USB-A to USB-B | Arduino power | 1 |

---

## PIN Assignment

### Arduino Pin Configuration

| PIN | Function | Direction | Voltage | Notes |
|-----|----------|-----------|---------|-------|
| 8 | Servo Signal | Output | 5V PWM | 1000-2600µs pulses |
| 6 | Vibration Motor | Output | 5V PWM | Optional PWM for intensity |
| 4 | Switch Hold | Input | 5V | Active LOW (pulled to GND) |
| 3 | Switch Close | Input | 5V | Active LOW (pulled to GND) |
| 2 | Switch Open | Input | 5V | Active LOW (pulled to GND) |
| A5 (SCL) | I2C Clock | Bidirectional | 5V | INA219 communication |
| A4 (SDA) | I2C Data | Bidirectional | 5V | INA219 communication |
| GND | Ground | Reference | 0V | **MUST connect to power GND** |
| 5V | 5V Output | Output | 5V | **NOT for servo power** |

---

## Wiring Diagram

### Overall System Layout

```
┌─────────────────────────────────────────────────────────────┐
│                     EXTERNAL 5V 3A POWER SUPPLY             │
│                    (+5V) ─── (GND)                          │
└──────────────┬──────────────────────────┬───────────────────┘
               │                          │
               │                          │
        ┌──────v─────────┐        ┌──────v──────────┐
        │ Servo Motor    │        │ INA219 & Others │
        │ (Red: 5V+)     │        │ (Multiple GND)  │
        │ (Black: GND)   │        └─────────────────┘
        │ (Yellow: SIG)  │
        └──────┬─────────┘
               │ Signal only
               │ to PIN 8
        ┌──────v──────────────────────────────┐
        │         Arduino UNO/Nano            │
        │                                      │
        │  USB  ──────► 5V + GND (Optional)  │
        │                                      │
        │  PIN 8  ──────► Servo Signal       │
        │  PIN 2  ◄────── Switch Open        │
        │  PIN 3  ◄────── Switch Close       │
        │  PIN 4  ◄────── Switch Hold        │
        │  PIN 6  ──────► Vibration Motor    │
        │  A4/A5 ◄─────► INA219 (I2C)        │
        │  GND   ───────► Common Ground      │
        └───────────────────────────────────┘
```

---

## Detailed Connections

### 1. Servo Motor Connection

```
Wheeltech HW020 Servo
├─ Red wire (5V+)     → External 5V power supply (+)
├─ Black wire (GND)   → External 5V power supply (-) + Arduino GND
└─ Yellow wire (SIG)  → Arduino PIN 8 (PWM signal)

⚠️ CRITICAL: 
- Do NOT connect servo power to Arduino 5V pin
- Must use external 5V 3A power supply
- Arduino GND and power supply GND MUST be connected together
```

**Pulse Width Mapping:**
```
0°   = 500µs   (0% = open position)
45°  = 850µs
90°  = 1200µs
135° = 1550µs  (50% = center position)
180° = 1900µs
225° = 2250µs
270° = 2600µs  (100% = close position)
```

### 2. INA219 Current Sensor Connection (I2C)

```
INA219 Breakout Board
├─ VCC       → Arduino 5V
├─ GND       → Arduino GND (also external power GND)
├─ SCL       → Arduino A5 (I2C Clock)
├─ SDA       → Arduino A4 (I2C Data)
├─ VIN+      → Servo power supply (+5V)
└─ VIN-      → Servo power supply GND (0V)

Features:
- Measures voltage: 0-26V max
- Measures current: ±3.2A max
- Default I2C Address: 0x40
- Resolution: ~0.1mA
```

**I2C Address Configuration:**

The INA219 uses I2C address 0x40 by default. If you need multiple sensors, you can change the address by connecting the A0/A1 pins:

| A1 | A0 | Address |
|----|----|---------| 
| GND | GND | 0x40 (default) |
| GND | Vs+ | 0x41 |
| Gnd | SDA | 0x42 |
| Gnd | SCL | 0x43 |

### 3. Three-Position Switch Connection

```
Three-Position Switch (e.g., toggle switch with 3 positions)

Position 1 (Open):
└─ Switch terminal 1 → Arduino PIN 2
   When switch is in position 1, PIN 2 goes LOW

Position 2 (Close):
└─ Switch terminal 2 → Arduino PIN 3
   When switch is in position 2, PIN 3 goes LOW

Position 3 (Hold):
└─ Switch terminal 3 → Arduino PIN 4
   When switch is in position 3, PIN 4 goes LOW

Common pin:
└─ Switch common → Arduino GND

Circuit per switch:
Arduino PIN (2/3/4) ──[Pull-up 10kΩ resistor]──► 5V
                     └─ [Switch] ──► GND

Note: Arduino's INPUT_PULLUP enables internal pull-up,
      so external resistors are optional but recommended
```

### 4. Vibration Motor Connection

```
Vibration Motor / Buzzer
├─ Positive (+5V) → Arduino PIN 6 (via current limiting resistor)
└─ Negative (GND) → Arduino GND

For small vibration motor (< 100mA):
- Use 220Ω resistor in series
- Direct connection to PIN 6

For buzzer (> 100mA):
- Use external transistor (NPN 2N2222 or similar)
- Arduino PIN 6 → Base via 1kΩ resistor
- Collector → Buzzer positive
- Emitter → GND
- Buzzer negative → Ground

Optional: Add 1µF capacitor across motor for debouncing
```

---

## Power Distribution

### Recommended Power Scheme

```
┌─────────────────────────────────────┐
│   External 5V 3A Power Supply       │
│   (USB adapter or battery pack)     │
│        +5V ──┬──── GND             │
└──────────────┼────────────┬────────┘
               │            │
        ┌──────v──┐   ┌─────v──────┐
        │  Servo  │   │ INA219 &   │
        │  Motor  │   │ Vibration  │
        └──────┬──┘   └─────┬──────┘
               │            │
        ┌──────v────────────v──────┐
        │  Arduino GND (Common)     │
        └───────────────────────────┘
        
        Arduino USB Power:
        ├─ Optional 5V for logic only
        ├─ NOT for servo
        └─ Can power Arduino + sensors
```

### Power Budget

| Component | Voltage | Current | Peak | Notes |
|-----------|---------|---------|------|-------|
| Arduino | 5V | ~50mA | 100mA | Logic + sensors |
| INA219 | 5V | ~0.5mA | 1mA | Very low power |
| Servo (idle) | 5V | ~50mA | 100mA | Holding position |
| Servo (moving) | 5V | 300-800mA | 1A | Under load |
| Vibration | 5V | 50mA | 100mA | Short bursts |
| **Total** | | | **3A** | Requires external supply |

---

## Software Architecture

### Module Structure

```
gripper_main.ino
│
├─ Hardware Initialization
│  ├─ Serial (9600 baud)
│  ├─ GPIO pins setup
│  └─ I2C + INA219 setup
│
├─ Main Control Loop (20ms cycle)
│  ├─ Servo position update
│  ├─ Current monitoring
│  ├─ Switch input polling
│  ├─ Vibration control
│  └─ Command parsing
│
├─ Servo Control Module
│  ├─ PWM pulse generation
│  ├─ Smooth position update
│  └─ Angle-to-pulse mapping
│
├─ Current Monitoring Module
│  ├─ INA219 I2C reading
│  ├─ Current spike detection
│  └─ Vibration triggering
│
├─ Switch Input Module
│  ├─ Pin polling
│  ├─ Debouncing
│  └─ Action execution
│
├─ Configuration Module
│  ├─ EEPROM storage
│  ├─ Parameter loading
│  └─ Parameter saving
│
└─ Logging & Serial Module
   ├─ Data logging (angle + current)
   ├─ Serial command parsing
   ├─ Status display
   └─ Menu system
```

### EEPROM Layout

| Address | Size | Parameter | Default | Range |
|---------|------|-----------|---------|-------|
| 0-1 | 2 bytes | angleOpen | 0 | 0-270 |
| 2-3 | 2 bytes | angleClose | 270 | 0-270 |
| 4-5 | 2 bytes | moveSpeed | 5 | 1-10 |
| 6-7 | 2 bytes | currentThreshold | 300 | 50-1000 |
| 8-9 | 2 bytes | vibrateTime | 200 | 50-1000 |

---

## Data Flow

### Normal Operation Sequence

```
1. Initialization (setup)
   ├─ Load config from EEPROM
   ├─ Initialize servo to 135°
   ├─ Initialize INA219 sensor
   └─ Print menu to serial

2. Main Loop (every 20ms)
   │
   ├─ [Every 100ms] Servo Update
   │  ├─ Calculate next angle
   │  ├─ Send PWM pulse
   │  └─ Update currentAngle variable
   │
   ├─ [Every 100ms] Current Reading
   │  ├─ Read INA219 voltage & current
   │  ├─ Calculate delta from previous
   │  ├─ Check if delta > threshold
   │  └─ Trigger vibration if spike detected
   │
   ├─ [Continuous] Switch Polling
   │  ├─ Check PIN 2/3/4 state
   │  ├─ Debounce (wait for stable state)
   │  └─ Execute action (open/close/stop)
   │
   ├─ [Continuous] Vibration Control
   │  ├─ Check if vibration timeout reached
   │  └─ Turn off motor if time elapsed
   │
   └─ [Continuous] Serial Command Parsing
      ├─ Check for incoming data
      ├─ Parse command string
      └─ Execute corresponding action

3. User Actions
   │
   ├─ Manual Switch Input
   │  ├─ Reads switch position
   │  ├─ Immediately moves gripper
   │  └─ Logs action with timestamp & current
   │
   └─ Serial Command Input
      ├─ User types command (e.g., "open")
      ├─ Command parsed and validated
      ├─ Action executed
      └─ Response sent to serial
```

### Current Spike Detection Logic

```
When INA219 reads current:

1. Read current from INA219
   └─ currentCurrent (mA)

2. Calculate delta
   └─ delta = |currentCurrent - lastCurrent|

3. Check threshold
   ├─ IF delta >= currentThreshold
   │  ├─ Log "spike_detected"
   │  ├─ Start vibration motor
   │  ├─ Print alert to serial
   │  └─ Vibrate for vibrateTime ms
   │
   └─ ELSE
      └─ Continue normally

4. Update last reading
   └─ lastCurrent = currentCurrent
```

---

## Operating Modes

### Mode 1: Manual Switch Control
```
User Position → Switch Detects → Arduino Reads PIN → Action Executed
                                 (debounced)

- No serial input required
- Immediate response (~50ms including debounce)
- Automatic logging with timestamp & current
```

### Mode 2: Serial Command Control
```
Serial Input → Parse Command → Validate → Execute → Log + Response
(via USB/serial monitor)

Example: "open" → Move to angleOpen → Log action → Print "✓"
```

### Mode 3: Autonomous Current-Based Control
```
Servo Moving → INA219 Monitoring → Current Spike Detected → Vibration
                                   (every 100ms)

- Runs automatically in background
- No user input required
- Useful for detecting gripper "grip" on object
```

---

## Troubleshooting Wiring Issues

| Symptom | Likely Cause | Solution |
|---------|-------------|----------|
| Servo doesn't move | Wrong PIN or no power | Check PIN 8 and 5V 3A supply |
| INA219 not detected | I2C connection issue | Check SDA/SCL (A4/A5) connections |
| Switch doesn't work | Wrong PIN or loose wire | Verify PIN 2/3/4 and GND connection |
| Vibration doesn't work | PIN 6 issue or motor dead | Test motor directly or change PIN |
| Servo jitters | Power supply unstable | Use separate 5V supply, add capacitor |
| Current readings wrong | INA219 loose wiring | Reseat I2C connections |
| Can't upload code | USB issue | Try different cable or USB port |

---

## Component Specifications

### Wheeltech HW020 Servo
```
Control Signal:  Standard PWM (50Hz = 20ms period)
Pulse Width:     500-2600µs (0-270°)
Operating Voltage: 5-7.4V
Stall Current:   500-800mA at 5V
Weight:          ~55g
Size:            40.7 x 20.1 x 38.2mm
Torque:          10kg·cm at 5V
```

### INA219 Current Sensor
```
Operating Voltage: 3.3-5V
Max Measured Voltage: 26V
Max Current: ±3.2A
I2C Address: 0x40 (default)
Resolution: ~0.1mA (10-bit ADC)
Response Time: ~1ms
Size: ~15 x 13mm
```

### Arduino UNO/Nano
```
Microcontroller: ATmega328P
Operating Voltage: 5V
Clock Speed: 16MHz
Digital Pins: 14 (6 with PWM)
Analog Pins: 6 (A0-A5)
EEPROM: 1024 bytes
SRAM: 2048 bytes
Flash: 32KB (28KB available for code)
```

---

## Safety Considerations

⚠️ **Power Supply Safety**
- Always use a **regulated 5V 3A minimum** power supply
- Use appropriate power connector (avoid loose connections)
- Add a main power switch in the supply line

⚠️ **Mechanical Safety**
- Gripper can apply force - keep fingers away
- Test with empty gripper first
- Monitor current to detect unusual loads

⚠️ **Electrical Safety**
- Ensure proper grounding throughout
- Use appropriate wire gauges (at least 22AWG for power)
- Keep water/liquid away from electronics

⚠️ **Software Safety**
- Don't set open/close angles to same value
- Monitor current spike threshold setting
- Keep serial logs to review operation

---

## Testing Checklist

- [ ] Arduino uploads code successfully
- [ ] Serial monitor shows "✓ INA219 initialized"
- [ ] Servo responds to "open" command (moves to 0°)
- [ ] Servo responds to "close" command (moves to 270°)
- [ ] Current readings are reasonable (50-800mA)
- [ ] Manual switch at position 1 opens gripper
- [ ] Manual switch at position 2 closes gripper
- [ ] Manual switch at position 3 stops gripper
- [ ] Vibration motor activates when commanded
- [ ] Data logs show entries with timestamp and current
- [ ] Configuration saves to EEPROM

---

## Further Reading

- [Adafruit INA219 Guide](https://learn.adafruit.com/adafruit-ina219-current-sensor-breakout)
- [Arduino PWM Reference](https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/)
- [Arduino I2C Reference](https://www.arduino.cc/en/Reference/Wire)
- [Servo Motor Control](https://www.arduino.cc/reference/en/libraries/servo/)

