/*
 * Electric Gripper Control System v2.0 - Full Featured
 *
 * Features:
 * - Servo motor control (PIN 8)
 * - INA219 current monitoring (I2C: A4/A5)
 * - Three-position switch control (PIN 2/3/4)
 * - Vibration feedback on current spike (PIN 6)
 * - Complete angle and current logging
 * - Configurable parameters with EEPROM
 *
 * Hardware: Arduino + Wheeltech HW020 270° + INA219 + 3-pos switch + vibration motor
 * Power: External 5V 3A
 */

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <EEPROM.h>

// ============ PIN DEFINITIONS ============
const int SERVO_PIN = 8;
const int SWITCH_OPEN = 2;       // Position 1: Open
const int SWITCH_CLOSE = 3;      // Position 2: Close
const int SWITCH_HOLD = 4;       // Position 3: Hold
const int VIBRATION_PIN = 6;     // Vibration motor (PWM)

// ============ SERVO CONFIGURATION ============
const int PULSE_MIN = 500;
const int PULSE_MAX = 2600;
const int ANGLE_MAX = 270;

// ============ EEPROM ADDRESSES ============
const int EEPROM_ADDR_OPEN = 0;
const int EEPROM_ADDR_CLOSE = 2;
const int EEPROM_ADDR_SPEED = 4;
const int EEPROM_ADDR_CURRENT_THR = 6;
const int EEPROM_ADDR_VIBRATE_TIME = 8;

// ============ DEFAULT CONFIGURATION ============
int angleOpen = 0;               // Open position (0°)
int angleClose = 270;            // Close position (270°)
int moveSpeed = 5;               // Speed (1-10 °/100ms)
int currentThreshold = 300;      // Current spike threshold (mA)
int vibrateTime = 200;           // Vibration duration (ms)

// ============ STATE VARIABLES ============
int currentAngle = 135;
int targetAngle = 135;
unsigned long lastMoveTime = 0;
const int MOVE_INTERVAL = 100;

float currentCurrent = 0;         // Current in mA
float lastCurrent = 0;            // Previous current reading
unsigned long lastCurrentReadTime = 0;
const int CURRENT_READ_INTERVAL = 100; // Read current every 100ms

unsigned long vibrationEndTime = 0;
bool vibrating = false;

// ============ INA219 SENSOR ============
Adafruit_INA219 ina219;

// ============ LOGGING ============
struct LogEntry {
  unsigned long timestamp;
  int angle;
  float current;
  char action[20];
};

#define MAX_LOGS 100
LogEntry logs[MAX_LOGS];
int logCount = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(9600);
  delay(1000);

  // Initialize hardware
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(SWITCH_OPEN, INPUT_PULLUP);
  pinMode(SWITCH_CLOSE, INPUT_PULLUP);
  pinMode(SWITCH_HOLD, INPUT_PULLUP);
  pinMode(VIBRATION_PIN, OUTPUT);

  // Initialize I2C and INA219
  Wire.begin();
  if (!ina219.begin()) {
    Serial.println("✗ INA219 not found! Check I2C connection.");
    // Continue anyway, just without current monitoring
  } else {
    Serial.println("✓ INA219 initialized");
  }

  // Load configuration from EEPROM
  loadConfig();

  // Initialize servo
  moveToAngle(currentAngle);
  delay(500);

  printBanner();
  printMenu();
}

// ============ MAIN LOOP ============
void loop() {
  unsigned long now = millis();

  // Update servo position smoothly
  if (now - lastMoveTime >= MOVE_INTERVAL) {
    updateServoPosition();
    lastMoveTime = now;
  }

  // Read current from INA219
  if (now - lastCurrentReadTime >= CURRENT_READ_INTERVAL) {
    readCurrent();
    checkCurrentSpike();
    lastCurrentReadTime = now;
  }

  // Handle switch inputs
  checkSwitches();

  // Handle vibration timeout
  if (vibrating && now >= vibrationEndTime) {
    stopVibration();
  }

  // Handle serial commands
  if (Serial.available() > 0) {
    handleCommand();
  }

  delay(10);
}

// ============ CURRENT MONITORING ============
void readCurrent() {
  float voltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();

  currentCurrent = current_mA;
}

void checkCurrentSpike() {
  float currentDelta = abs(currentCurrent - lastCurrent);

  if (currentDelta >= currentThreshold) {
    Serial.print("⚡ Current spike detected! Delta: ");
    Serial.print(currentDelta);
    Serial.println(" mA");

    startVibration();
    logAction(currentAngle, "spike_detected");
  }

  lastCurrent = currentCurrent;
}

void startVibration() {
  if (!vibrating) {
    digitalWrite(VIBRATION_PIN, HIGH);
    vibrating = true;
    vibrationEndTime = millis() + vibrateTime;
    Serial.println("📳 Vibration started");
  }
}

void stopVibration() {
  digitalWrite(VIBRATION_PIN, LOW);
  vibrating = false;
  Serial.println("📳 Vibration stopped");
}

// ============ SWITCH HANDLING ============
void checkSwitches() {
  // Note: Switches are active LOW (pulled to ground)
  if (digitalRead(SWITCH_OPEN) == LOW) {
    if (currentAngle != angleOpen) {
      Serial.println("🔓 Switch: Open");
      gripperOpen();
      delay(50);  // Debounce
      while (digitalRead(SWITCH_OPEN) == LOW) delay(10);
      delay(50);
    }
  }

  if (digitalRead(SWITCH_CLOSE) == LOW) {
    if (currentAngle != angleClose) {
      Serial.println("🔒 Switch: Close");
      gripperClose();
      delay(50);
      while (digitalRead(SWITCH_CLOSE) == LOW) delay(10);
      delay(50);
    }
  }

  if (digitalRead(SWITCH_HOLD) == LOW) {
    Serial.println("⏸ Switch: Hold");
    gripperStop();
    delay(50);
    while (digitalRead(SWITCH_HOLD) == LOW) delay(10);
    delay(50);
  }
}

// ============ SERVO CONTROL ============
int getPulseForAngle(int angle) {
  if (angle == 0) return 500;
  if (angle == 45) return 850;
  if (angle == 90) return 1200;
  if (angle == 135) return 1550;
  if (angle == 180) return 1900;
  if (angle == 225) return 2250;
  if (angle == 270) return 2600;

  int lower = (angle / 45) * 45;
  int upper = lower + 45;
  int lowerPulse = getPulseForAngle(lower);
  int upperPulse = getPulseForAngle(upper);

  return lowerPulse + ((upperPulse - lowerPulse) * (angle - lower) / 45);
}

void sendPWMPulse(int angle) {
  int pulseMicros = getPulseForAngle(angle);

  for (int i = 0; i < 100; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseMicros);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulseMicros);
  }
}

void updateServoPosition() {
  if (currentAngle < targetAngle) {
    currentAngle += moveSpeed;
    if (currentAngle > targetAngle) currentAngle = targetAngle;
  } else if (currentAngle > targetAngle) {
    currentAngle -= moveSpeed;
    if (currentAngle < targetAngle) currentAngle = targetAngle;
  }

  sendPWMPulse(currentAngle);
}

void moveToAngle(int angle) {
  angle = constrain(angle, 0, ANGLE_MAX);
  targetAngle = angle;

  Serial.print("→ Moving to ");
  Serial.print(angle);
  Serial.print("° | Current: ");
  Serial.print(currentCurrent, 0);
  Serial.println(" mA");

  logAction(angle, "move");
}

// ============ GRIPPER CONTROL ============
void gripperOpen() {
  Serial.println("🔓 Opening gripper...");
  moveToAngle(angleOpen);
  logAction(angleOpen, "open");
}

void gripperClose() {
  Serial.println("🔒 Closing gripper...");
  moveToAngle(angleClose);
  logAction(angleClose, "close");
}

void gripperStop() {
  Serial.println("⏸ Stopping gripper");
  targetAngle = currentAngle;
  logAction(currentAngle, "stop");
}

// ============ LOGGING ============
void logAction(int angle, const char* action) {
  if (logCount >= MAX_LOGS) {
    for (int i = 0; i < MAX_LOGS - 1; i++) {
      logs[i] = logs[i + 1];
    }
    logCount = MAX_LOGS - 1;
  }

  logs[logCount].timestamp = millis() / 1000;
  logs[logCount].angle = angle;
  logs[logCount].current = currentCurrent;
  strcpy(logs[logCount].action, action);
  logCount++;
}

void showLogs() {
  Serial.println("\n================ ANGLE & CURRENT LOG ================");
  Serial.println("Time(s) | Angle | Current(mA) | Action");
  Serial.println("--------|-------|-------------|------------------");

  for (int i = 0; i < logCount; i++) {
    Serial.print(logs[i].timestamp);
    Serial.print("      | ");
    Serial.print(logs[i].angle);
    Serial.print("°    | ");
    Serial.print(logs[i].current, 0);
    Serial.print("      | ");
    Serial.println(logs[i].action);
  }

  Serial.println("====================================================\n");
}

void clearLogs() {
  logCount = 0;
  Serial.println("✓ Logs cleared");
}

// ============ CONFIGURATION ============
void loadConfig() {
  EEPROM.get(EEPROM_ADDR_OPEN, angleOpen);
  EEPROM.get(EEPROM_ADDR_CLOSE, angleClose);
  EEPROM.get(EEPROM_ADDR_SPEED, moveSpeed);
  EEPROM.get(EEPROM_ADDR_CURRENT_THR, currentThreshold);
  EEPROM.get(EEPROM_ADDR_VIBRATE_TIME, vibrateTime);

  // Validate
  if (angleOpen < 0 || angleOpen > ANGLE_MAX) angleOpen = 0;
  if (angleClose < 0 || angleClose > ANGLE_MAX) angleClose = 270;
  if (moveSpeed < 1 || moveSpeed > 10) moveSpeed = 5;
  if (currentThreshold < 50 || currentThreshold > 1000) currentThreshold = 300;
  if (vibrateTime < 50 || vibrateTime > 1000) vibrateTime = 200;
}

void saveConfig() {
  EEPROM.put(EEPROM_ADDR_OPEN, angleOpen);
  EEPROM.put(EEPROM_ADDR_CLOSE, angleClose);
  EEPROM.put(EEPROM_ADDR_SPEED, moveSpeed);
  EEPROM.put(EEPROM_ADDR_CURRENT_THR, currentThreshold);
  EEPROM.put(EEPROM_ADDR_VIBRATE_TIME, vibrateTime);

  Serial.println("✓ Configuration saved to EEPROM");
}

void showConfig() {
  Serial.println("\n============= CONFIGURATION =============");
  Serial.print("Open angle: ");
  Serial.print(angleOpen);
  Serial.println("°");
  Serial.print("Close angle: ");
  Serial.print(angleClose);
  Serial.println("°");
  Serial.print("Movement speed: ");
  Serial.print(moveSpeed);
  Serial.println("°/100ms");
  Serial.print("Current threshold: ");
  Serial.print(currentThreshold);
  Serial.println(" mA");
  Serial.print("Vibration time: ");
  Serial.print(vibrateTime);
  Serial.println(" ms");
  Serial.println("=========================================\n");
}

// ============ COMMAND HANDLING ============
void handleCommand() {
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd.length() == 0) return;

  // Gripper control
  if (cmd == "open" || cmd == "o") {
    gripperOpen();
  }
  else if (cmd == "close" || cmd == "c") {
    gripperClose();
  }
  else if (cmd == "stop" || cmd == "s") {
    gripperStop();
  }
  else if (cmd.startsWith("go ")) {
    int angle = cmd.substring(3).toInt();
    if (angle >= 0 && angle <= ANGLE_MAX) {
      moveToAngle(angle);
    } else {
      Serial.print("✗ Invalid angle! Range: 0-");
      Serial.println(ANGLE_MAX);
    }
  }

  // Configuration
  else if (cmd.startsWith("set open ")) {
    angleOpen = cmd.substring(9).toInt();
    angleOpen = constrain(angleOpen, 0, ANGLE_MAX);
    Serial.print("✓ Open angle set to ");
    Serial.print(angleOpen);
    Serial.println("°");
    saveConfig();
  }
  else if (cmd.startsWith("set close ")) {
    angleClose = cmd.substring(10).toInt();
    angleClose = constrain(angleClose, 0, ANGLE_MAX);
    Serial.print("✓ Close angle set to ");
    Serial.print(angleClose);
    Serial.println("°");
    saveConfig();
  }
  else if (cmd.startsWith("set speed ")) {
    moveSpeed = cmd.substring(10).toInt();
    moveSpeed = constrain(moveSpeed, 1, 10);
    Serial.print("✓ Speed set to ");
    Serial.print(moveSpeed);
    Serial.println("°/100ms");
    saveConfig();
  }
  else if (cmd.startsWith("set threshold ")) {
    currentThreshold = cmd.substring(14).toInt();
    currentThreshold = constrain(currentThreshold, 50, 1000);
    Serial.print("✓ Current threshold set to ");
    Serial.print(currentThreshold);
    Serial.println(" mA");
    saveConfig();
  }
  else if (cmd.startsWith("set vibrate ")) {
    vibrateTime = cmd.substring(12).toInt();
    vibrateTime = constrain(vibrateTime, 50, 1000);
    Serial.print("✓ Vibration time set to ");
    Serial.print(vibrateTime);
    Serial.println(" ms");
    saveConfig();
  }

  // Status and logging
  else if (cmd == "pos" || cmd == "position") {
    Serial.print("Angle: ");
    Serial.print(currentAngle);
    Serial.print("° | Current: ");
    Serial.print(currentCurrent, 0);
    Serial.println(" mA");
  }
  else if (cmd == "current") {
    Serial.print("Current: ");
    Serial.print(currentCurrent, 0);
    Serial.println(" mA");
  }
  else if (cmd == "config") {
    showConfig();
  }
  else if (cmd == "log") {
    showLogs();
  }
  else if (cmd == "clear log") {
    clearLogs();
  }
  else if (cmd == "test vibrate") {
    Serial.println("Testing vibration...");
    startVibration();
  }
  else if (cmd == "help" || cmd == "h" || cmd == "?") {
    printMenu();
  }
  else {
    Serial.println("✗ Unknown command");
  }
}

// ============ UI ============
void printBanner() {
  Serial.println("\n==========================================");
  Serial.println("  Electric Gripper Control System v2.0");
  Serial.println("==========================================");
  Serial.println("✓ Servo: PIN 8 (Wheeltech HW020 270°)");
  Serial.println("✓ INA219: I2C (A4/A5)");
  Serial.println("✓ Switch: PIN 2/3/4 (3-position)");
  Serial.println("✓ Vibration: PIN 6");
  Serial.println("✓ Power: External 5V 3A");
  Serial.println("==========================================\n");
}

void printMenu() {
  Serial.println("\n============== COMMANDS ==============");
  Serial.println("Gripper Control:");
  Serial.println("  open, o       - Open gripper");
  Serial.println("  close, c      - Close gripper");
  Serial.println("  stop, s       - Stop gripper");
  Serial.println("  go <angle>    - Move to angle (0-270)");

  Serial.println("\nConfiguration:");
  Serial.println("  set open <angle>       - Set open position");
  Serial.println("  set close <angle>      - Set close position");
  Serial.println("  set speed <1-10>       - Set movement speed");
  Serial.println("  set threshold <mA>     - Current spike threshold");
  Serial.println("  set vibrate <ms>       - Vibration duration");
  Serial.println("  config                 - Show all settings");

  Serial.println("\nStatus & Logging:");
  Serial.println("  pos, position   - Show current angle");
  Serial.println("  current         - Show current reading");
  Serial.println("  log             - Show complete log");
  Serial.println("  clear log       - Clear log history");
  Serial.println("  test vibrate    - Test vibration motor");
  Serial.println("  help, h, ?      - Show this menu");
  Serial.println("=====================================\n");
}
