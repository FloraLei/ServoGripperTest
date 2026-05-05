/*
 * Electric Gripper Control - Keyboard Version (ESP32)
 *
 * Input:
 *   'a'  →  Open gripper
 *   'q'  →  Close gripper
 *   (no key)  →  Hold current position
 *
 * Hardware wiring:
 *   Servo signal     → GPIO 5
 *   Vibration motor  → IN: GPIO 18 | VCC: 5V | GND: GND
 *   INA219           → SDA: GPIO 21 | SCL: GPIO 22 | VCC: 3.3V | GND: GND
 *                      VIN+ → external 5V supply positive
 *                      VIN- → servo VCC (in series, measures servo current)
 *
 * Note: 'q' and 'a' map to close/open without needing Enter key.
 *       All other commands still require Enter (e.g. "go 135", "config", "log").
 */

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <EEPROM.h>

// ============ PIN DEFINITIONS ============
const int SERVO_PIN      = 5;
const int VIBRATION_PIN  = 18;
// INA219: SDA=21, SCL=22 (ESP32 default I2C)

// ============ SERVO CONFIGURATION ============
const int PULSE_MIN = 500;
const int PULSE_MAX = 2600;
const int ANGLE_MAX = 270;

// ============ EEPROM ============
const int EEPROM_SIZE          = 512;
const int EEPROM_ADDR_OPEN     = 0;
const int EEPROM_ADDR_CLOSE    = 2;
const int EEPROM_ADDR_SPEED    = 4;
const int EEPROM_ADDR_CURR_THR = 6;
const int EEPROM_ADDR_VIBTIME  = 8;

// ============ CONFIG (loaded from EEPROM) ============
int angleOpen       = 0;
int angleClose      = 270;
int moveSpeed       = 5;    // degrees per 100 ms
int currentThresh   = 300;  // mA delta to trigger vibration
int vibrateTime     = 200;  // ms

// ============ STATE ============
int  currentAngle = 135;
int  targetAngle  = 135;

float currentCurrent = 0;
float lastCurrent    = 0;

bool  vibrating       = false;
unsigned long vibrationEndTime    = 0;
unsigned long lastMoveTime        = 0;
unsigned long lastCurrentReadTime = 0;

const int MOVE_INTERVAL         = 100;  // ms
const int CURRENT_READ_INTERVAL = 100;  // ms

// ============ INA219 ============
Adafruit_INA219 ina219;

// ============ LOG ============
struct LogEntry {
  unsigned long timestamp;
  int   angle;
  float current;
  char  action[20];
};

#define MAX_LOGS 100
LogEntry logs[MAX_LOGS];
int logCount = 0;

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SERVO_PIN,     OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);

  Wire.begin(21, 22);

  if (!ina219.begin()) {
    Serial.println("[WARN] INA219 not found – current monitoring disabled");
  } else {
    Serial.println("[OK] INA219 ready");
  }

  EEPROM.begin(EEPROM_SIZE);
  loadConfig();

  moveToAngle(currentAngle);
  delay(500);

  printBanner();
}

// ============ MAIN LOOP ============
void loop() {
  unsigned long now = millis();

  if (now - lastMoveTime >= MOVE_INTERVAL) {
    updateServoPosition();
    lastMoveTime = now;
  }

  if (now - lastCurrentReadTime >= CURRENT_READ_INTERVAL) {
    readCurrent();
    checkCurrentSpike();
    lastCurrentReadTime = now;
  }

  if (vibrating && now >= vibrationEndTime) {
    stopVibration();
  }

  handleSerial();

  delay(10);
}

// ============ SERIAL INPUT ============
/*
 * Single-character keys 'a' and 'q' act immediately (no Enter needed).
 * Longer commands (e.g. "go 90") still need Enter.
 * When nothing is received the target angle is unchanged = hold.
 */
void handleSerial() {
  if (!Serial.available()) return;

  // Peek at the first byte to decide single-key vs command
  char first = Serial.peek();

  if ((first == 'a' || first == 'A') && pendingOnlyOneChar()) {
    Serial.read();  // consume
    gripperOpen();
    return;
  }
  if ((first == 'q' || first == 'Q') && pendingOnlyOneChar()) {
    Serial.read();
    gripperClose();
    return;
  }

  // Multi-character command – wait for newline
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();
  if (cmd.length() == 0) return;

  processCommand(cmd);
}

// Returns true when the Serial buffer has exactly one printable character
// (ignores leading CR/LF).  Prevents "go 90<Enter>" from being consumed here.
bool pendingOnlyOneChar() {
  // Flush any trailing CR/LF that arrived with the char
  // We detect: only one non-whitespace byte available right now.
  // Because Serial.available() can return > 1 if \r\n follows immediately,
  // we allow 1-3 bytes as long as extras are all whitespace.
  int n = Serial.available();
  if (n > 3) return false;  // definitely a longer command

  // Read all available into a tiny buffer and check
  char buf[4] = {0};
  int read = 0;
  unsigned long t = millis();
  while (read < n && millis() - t < 5) {
    if (Serial.available()) buf[read++] = Serial.read();
  }

  int printable = 0;
  for (int i = 0; i < read; i++) {
    if (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ') printable++;
  }

  // Push back logic not available on Arduino – re-process if not single key
  if (printable == 1) return true;

  // More than one printable byte: push what we read back into a shadow buffer
  // Since Arduino Serial has no unget, we handle this by processing inline
  String reconstructed = "";
  for (int i = 0; i < read; i++) {
    if (buf[i] != '\r' && buf[i] != '\n') reconstructed += buf[i];
  }
  // Wait for rest of line if not already complete
  if (reconstructed.length() > 0) {
    String rest = Serial.readStringUntil('\n');
    rest.trim();
    reconstructed += rest;
    reconstructed.toLowerCase();
    processCommand(reconstructed);
  }
  return false;
}

// ============ COMMAND PROCESSING ============
void processCommand(String cmd) {
  if (cmd == "o" || cmd == "open") {
    gripperOpen();
  } else if (cmd == "c" || cmd == "close") {
    gripperClose();
  } else if (cmd == "h" || cmd == "hold" || cmd == "stop" || cmd == "s") {
    gripperHold();
  } else if (cmd.startsWith("go ")) {
    int angle = cmd.substring(3).toInt();
    if (angle >= 0 && angle <= ANGLE_MAX) moveToAngle(angle);
    else Serial.println("[ERR] Angle out of range 0-270");

  } else if (cmd.startsWith("set open ")) {
    angleOpen = constrain(cmd.substring(9).toInt(), 0, ANGLE_MAX);
    Serial.print("[OK] Open angle = "); Serial.print(angleOpen); Serial.println("deg");
    saveConfig();
  } else if (cmd.startsWith("set close ")) {
    angleClose = constrain(cmd.substring(10).toInt(), 0, ANGLE_MAX);
    Serial.print("[OK] Close angle = "); Serial.print(angleClose); Serial.println("deg");
    saveConfig();
  } else if (cmd.startsWith("set speed ")) {
    moveSpeed = constrain(cmd.substring(10).toInt(), 1, 10);
    Serial.print("[OK] Speed = "); Serial.print(moveSpeed); Serial.println("deg/100ms");
    saveConfig();
  } else if (cmd.startsWith("set threshold ")) {
    currentThresh = constrain(cmd.substring(14).toInt(), 50, 1000);
    Serial.print("[OK] Threshold = "); Serial.print(currentThresh); Serial.println(" mA");
    saveConfig();
  } else if (cmd.startsWith("set vibrate ")) {
    vibrateTime = constrain(cmd.substring(12).toInt(), 50, 1000);
    Serial.print("[OK] Vibration = "); Serial.print(vibrateTime); Serial.println(" ms");
    saveConfig();

  } else if (cmd == "pos" || cmd == "position") {
    Serial.print("Angle: "); Serial.print(currentAngle);
    Serial.print("deg  Current: "); Serial.print(currentCurrent, 0); Serial.println(" mA");
  } else if (cmd == "current") {
    float v = ina219.getBusVoltage_V();
    Serial.print("Voltage: "); Serial.print(v, 2);
    Serial.print(" V  Current: "); Serial.print(currentCurrent, 0); Serial.println(" mA");
  } else if (cmd == "config") {
    showConfig();
  } else if (cmd == "log") {
    showLogs();
  } else if (cmd == "clear log") {
    logCount = 0;
    Serial.println("[OK] Log cleared");
  } else if (cmd == "test vibrate") {
    startVibration();
  } else if (cmd == "?" || cmd == "help") {
    printBanner();
  } else {
    Serial.print("[?] Unknown: "); Serial.println(cmd);
  }
}

// ============ GRIPPER ACTIONS ============
void gripperOpen() {
  Serial.println("[OPEN] Opening gripper...");
  moveToAngle(angleOpen);
  logAction(angleOpen, "open");
}

void gripperClose() {
  Serial.println("[CLOSE] Closing gripper...");
  moveToAngle(angleClose);
  logAction(angleClose, "close");
}

void gripperHold() {
  targetAngle = currentAngle;
  Serial.print("[HOLD] Holding at "); Serial.print(currentAngle); Serial.println("deg");
  logAction(currentAngle, "hold");
}

// ============ SERVO CONTROL ============
int getPulseForAngle(int angle) {
  // Calibrated breakpoints for Wheeltech HW020 270deg servo
  if (angle <= 0)   return 500;
  if (angle >= 270) return 2600;

  int lower = (angle / 45) * 45;
  int upper = lower + 45;

  // Breakpoint pulses (microseconds)
  const int pulses[] = {500, 850, 1200, 1550, 1900, 2250, 2600};
  int lp = pulses[lower / 45];
  int up = pulses[upper / 45];

  return lp + ((up - lp) * (angle - lower) / 45);
}

void sendPWMPulse(int angle) {
  int pw = getPulseForAngle(angle);
  for (int i = 0; i < 20; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pw);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pw);
  }
}

void updateServoPosition() {
  if (currentAngle < targetAngle) {
    currentAngle = min(currentAngle + moveSpeed, targetAngle);
  } else if (currentAngle > targetAngle) {
    currentAngle = max(currentAngle - moveSpeed, targetAngle);
  }
  sendPWMPulse(currentAngle);
}

void moveToAngle(int angle) {
  targetAngle = constrain(angle, 0, ANGLE_MAX);
  Serial.print("-> target "); Serial.print(targetAngle); Serial.println("deg");
}

// ============ CURRENT MONITORING ============
void readCurrent() {
  currentCurrent = ina219.getCurrent_mA();
}

void checkCurrentSpike() {
  float delta = abs(currentCurrent - lastCurrent);
  if (delta >= currentThresh) {
    Serial.print("[SPIKE] delta="); Serial.print(delta, 0); Serial.println(" mA");
    startVibration();
    logAction(currentAngle, "spike");
  }
  lastCurrent = currentCurrent;
}

// ============ VIBRATION ============
void startVibration() {
  if (!vibrating) {
    digitalWrite(VIBRATION_PIN, HIGH);
    vibrating       = true;
    vibrationEndTime = millis() + vibrateTime;
  }
}

void stopVibration() {
  digitalWrite(VIBRATION_PIN, LOW);
  vibrating = false;
}

// ============ LOGGING ============
void logAction(int angle, const char* action) {
  if (logCount >= MAX_LOGS) {
    for (int i = 0; i < MAX_LOGS - 1; i++) logs[i] = logs[i + 1];
    logCount = MAX_LOGS - 1;
  }
  logs[logCount].timestamp = millis() / 1000;
  logs[logCount].angle     = angle;
  logs[logCount].current   = currentCurrent;
  strncpy(logs[logCount].action, action, sizeof(logs[logCount].action) - 1);
  logCount++;
}

void showLogs() {
  Serial.println("\nTime(s) | Angle | Current(mA) | Action");
  Serial.println("--------|-------|-------------|-------");
  for (int i = 0; i < logCount; i++) {
    Serial.print(logs[i].timestamp); Serial.print(" | ");
    Serial.print(logs[i].angle);     Serial.print("    | ");
    Serial.print(logs[i].current, 0);Serial.print("        | ");
    Serial.println(logs[i].action);
  }
  Serial.println();
}

// ============ CONFIG ============
void loadConfig() {
  EEPROM.get(EEPROM_ADDR_OPEN,     angleOpen);
  EEPROM.get(EEPROM_ADDR_CLOSE,    angleClose);
  EEPROM.get(EEPROM_ADDR_SPEED,    moveSpeed);
  EEPROM.get(EEPROM_ADDR_CURR_THR, currentThresh);
  EEPROM.get(EEPROM_ADDR_VIBTIME,  vibrateTime);

  if (angleOpen     < 0   || angleOpen     > ANGLE_MAX) angleOpen     = 0;
  if (angleClose    < 0   || angleClose    > ANGLE_MAX) angleClose    = 270;
  if (moveSpeed     < 1   || moveSpeed     > 10)        moveSpeed     = 5;
  if (currentThresh < 50  || currentThresh > 1000)      currentThresh = 300;
  if (vibrateTime   < 50  || vibrateTime   > 1000)      vibrateTime   = 200;
}

void saveConfig() {
  EEPROM.put(EEPROM_ADDR_OPEN,     angleOpen);
  EEPROM.put(EEPROM_ADDR_CLOSE,    angleClose);
  EEPROM.put(EEPROM_ADDR_SPEED,    moveSpeed);
  EEPROM.put(EEPROM_ADDR_CURR_THR, currentThresh);
  EEPROM.put(EEPROM_ADDR_VIBTIME,  vibrateTime);
  EEPROM.commit();
  Serial.println("[OK] Config saved");
}

void showConfig() {
  Serial.println("\n===== CONFIG =====");
  Serial.print("Open angle   : "); Serial.print(angleOpen);     Serial.println(" deg");
  Serial.print("Close angle  : "); Serial.print(angleClose);    Serial.println(" deg");
  Serial.print("Speed        : "); Serial.print(moveSpeed);     Serial.println(" deg/100ms");
  Serial.print("Curr threshold: "); Serial.print(currentThresh); Serial.println(" mA");
  Serial.print("Vibrate time : "); Serial.print(vibrateTime);   Serial.println(" ms");
  Serial.println("==================\n");
}

void printBanner() {
  Serial.println("\n===== Gripper Keyboard Control (ESP32) =====");
  Serial.println("  a         - Open gripper");
  Serial.println("  q         - Close gripper");
  Serial.println("  (no key)  - Hold position");
  Serial.println("--------------------------------------------");
  Serial.println("  o/open    - Open   |  c/close - Close");
  Serial.println("  go <deg>  - Go to angle 0-270");
  Serial.println("  pos       - Show angle & current");
  Serial.println("  current   - Show voltage & current");
  Serial.println("  config    - Show settings");
  Serial.println("  log       - Show log");
  Serial.println("  set open/close/speed/threshold/vibrate <n>");
  Serial.println("============================================\n");
}
