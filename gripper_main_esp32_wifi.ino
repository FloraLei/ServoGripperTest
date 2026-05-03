/*
 * Electric Gripper Control System v2.1 - ESP32 WiFi Version
 *
 * Features:
 * - Servo motor control (GPIO 5)
 * - INA219 current monitoring (I2C: GPIO 21/22)
 * - Three-position switch control (GPIO 35/34/39)
 * - Vibration feedback on current spike (GPIO 18)
 * - Complete angle and current logging
 * - Configurable parameters with EEPROM
 * - ✨ WiFi connectivity
 * - ✨ Web dashboard access
 * - ✨ Remote control via HTTP API
 * - ✨ Log upload to server
 *
 * Hardware: ESP32 Dev Board + Wheeltech HW020 270° + INA219 + 3-pos switch + vibration motor
 * Power: External 5V 3A
 */

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>

// ============ WiFi CONFIGURATION ============
const char* ssid = "YOUR_SSID";           // 修改为你的 WiFi 名称
const char* password = "YOUR_PASSWORD";   // 修改为你的 WiFi 密码
const int webServerPort = 80;

WebServer server(webServerPort);
bool wifiConnected = false;

// ============ PIN DEFINITIONS (ESP32) ============
const int SERVO_PIN = 5;
const int SWITCH_OPEN = 35;
const int SWITCH_CLOSE = 34;
const int SWITCH_HOLD = 39;
const int VIBRATION_PIN = 18;

// ============ SERVO CONFIGURATION ============
const int PULSE_MIN = 500;
const int PULSE_MAX = 2600;
const int ANGLE_MAX = 270;

// ============ EEPROM ADDRESSES ============
const int EEPROM_SIZE = 512;
const int EEPROM_ADDR_OPEN = 0;
const int EEPROM_ADDR_CLOSE = 2;
const int EEPROM_ADDR_SPEED = 4;
const int EEPROM_ADDR_CURRENT_THR = 6;
const int EEPROM_ADDR_VIBRATE_TIME = 8;

// ============ DEFAULT CONFIGURATION ============
int angleOpen = 0;
int angleClose = 270;
int moveSpeed = 5;
int currentThreshold = 300;
int vibrateTime = 200;

// ============ STATE VARIABLES ============
int currentAngle = 135;
int targetAngle = 135;
unsigned long lastMoveTime = 0;
const int MOVE_INTERVAL = 100;

float currentCurrent = 0;
float lastCurrent = 0;
unsigned long lastCurrentReadTime = 0;
const int CURRENT_READ_INTERVAL = 100;

unsigned long vibrationEndTime = 0;
bool vibrating = false;

unsigned long systemStartTime = 0;

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
  Serial.begin(115200);
  delay(1000);

  systemStartTime = millis();

  // Initialize hardware
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(SWITCH_OPEN, INPUT);
  pinMode(SWITCH_CLOSE, INPUT);
  pinMode(SWITCH_HOLD, INPUT);
  pinMode(VIBRATION_PIN, OUTPUT);

  // Initialize I2C
  Wire.begin(21, 22);

  if (!ina219.begin()) {
    Serial.println("✗ INA219 not found! Check I2C connection.");
  } else {
    Serial.println("✓ INA219 initialized");
  }

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Load configuration
  loadConfig();

  // Initialize servo
  moveToAngle(currentAngle);
  delay(500);

  // Connect to WiFi
  connectToWiFi();

  // Setup web server routes
  setupWebServer();

  printBanner();
  printMenu();
}

// ============ MAIN LOOP ============
void loop() {
  unsigned long now = millis();

  // Handle WiFi
  if (wifiConnected) {
    server.handleClient();
  }

  // Update servo position
  if (now - lastMoveTime >= MOVE_INTERVAL) {
    updateServoPosition();
    lastMoveTime = now;
  }

  // Read current
  if (now - lastCurrentReadTime >= CURRENT_READ_INTERVAL) {
    readCurrent();
    checkCurrentSpike();
    lastCurrentReadTime = now;
  }

  // Check switches
  checkSwitches();

  // Handle vibration
  if (vibrating && now >= vibrationEndTime) {
    stopVibration();
  }

  // Handle serial commands
  if (Serial.available() > 0) {
    handleCommand();
  }

  delay(10);
}

// ============ WiFi FUNCTIONS ============
void connectToWiFi() {
  Serial.println("\n📡 Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\n✓ WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Port: ");
    Serial.println(webServerPort);
  } else {
    wifiConnected = false;
    Serial.println("\n✗ WiFi connection failed!");
    Serial.println("Continuing in local mode...");
  }
}

void setupWebServer() {
  // API endpoints
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/logs", HTTP_GET, handleLogsAPI);
  server.on("/api/config", HTTP_GET, handleConfigAPI);
  server.on("/api/config", HTTP_POST, handleConfigPOST);

  // Control endpoints
  server.on("/api/open", HTTP_POST, handleOpen);
  server.on("/api/close", HTTP_POST, handleClose);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/go", HTTP_POST, handleGo);
  server.on("/api/vibrate", HTTP_POST, handleVibrate);

  // Dashboard
  server.on("/", HTTP_GET, handleDashboard);
  server.on("/dashboard.html", HTTP_GET, handleDashboard);

  // 404 handler
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("✓ Web server started");
}

void handleStatus() {
  String json = "{";
  json += "\"angle\":" + String(currentAngle) + ",";
  json += "\"current\":" + String(currentCurrent, 1) + ",";
  json += "\"target\":" + String(targetAngle) + ",";
  json += "\"uptime\":" + String((millis() - systemStartTime) / 1000) + ",";
  json += "\"logs_count\":" + String(logCount) + ",";
  json += "\"wifi\":" + String(wifiConnected ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void handleLogsAPI() {
  String json = "[";
  for (int i = 0; i < logCount; i++) {
    json += "{";
    json += "\"time\":" + String(logs[i].timestamp) + ",";
    json += "\"angle\":" + String(logs[i].angle) + ",";
    json += "\"current\":" + String(logs[i].current, 1) + ",";
    json += "\"action\":\"" + String(logs[i].action) + "\"";
    json += "}";
    if (i < logCount - 1) json += ",";
  }
  json += "]";

  server.send(200, "application/json", json);
}

void handleConfigAPI() {
  String json = "{";
  json += "\"open_angle\":" + String(angleOpen) + ",";
  json += "\"close_angle\":" + String(angleClose) + ",";
  json += "\"speed\":" + String(moveSpeed) + ",";
  json += "\"threshold\":" + String(currentThreshold) + ",";
  json += "\"vibrate_time\":" + String(vibrateTime);
  json += "}";

  server.send(200, "application/json", json);
}

void handleConfigPOST() {
  if (server.hasArg("open")) {
    angleOpen = server.arg("open").toInt();
    angleOpen = constrain(angleOpen, 0, ANGLE_MAX);
    saveConfig();
  }
  if (server.hasArg("close")) {
    angleClose = server.arg("close").toInt();
    angleClose = constrain(angleClose, 0, ANGLE_MAX);
    saveConfig();
  }
  if (server.hasArg("speed")) {
    moveSpeed = server.arg("speed").toInt();
    moveSpeed = constrain(moveSpeed, 1, 10);
    saveConfig();
  }
  if (server.hasArg("threshold")) {
    currentThreshold = server.arg("threshold").toInt();
    currentThreshold = constrain(currentThreshold, 50, 1000);
    saveConfig();
  }
  if (server.hasArg("vibrate")) {
    vibrateTime = server.arg("vibrate").toInt();
    vibrateTime = constrain(vibrateTime, 50, 1000);
    saveConfig();
  }

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleOpen() {
  gripperOpen();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleClose() {
  gripperClose();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleStop() {
  gripperStop();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleGo() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    if (angle >= 0 && angle <= ANGLE_MAX) {
      moveToAngle(angle);
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid angle\"}");
    }
  }
}

void handleVibrate() {
  startVibration();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleDashboard() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'><meta name='viewport' content='width=device-width'>";
  html += "<title>Gripper Control</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:20px;background:#f5f5f5}";
  html += ".container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}";
  html += "h1{color:#333;text-align:center}";
  html += ".status{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:20px 0}";
  html += ".status-item{background:#f9f9f9;padding:15px;border-radius:5px;border-left:4px solid #4CAF50}";
  html += ".status-value{font-size:24px;font-weight:bold;color:#4CAF50}";
  html += ".status-label{color:#666;font-size:12px}";
  html += ".buttons{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:20px 0}";
  html += "button{padding:15px;font-size:16px;border:none;border-radius:5px;cursor:pointer;transition:0.3s}";
  html += ".btn-open{background:#4CAF50;color:white}";
  html += ".btn-close{background:#f44336;color:white}";
  html += ".btn-stop{background:#ff9800;color:white}";
  html += ".btn-vibrate{background:#2196F3;color:white}";
  html += "button:hover{opacity:0.8}";
  html += "input{padding:10px;width:100%;margin:10px 0;border:1px solid #ddd;border-radius:5px}";
  html += ".slider-container{margin:20px 0}";
  html += "input[type='range']{width:100%}";
  html += ".logs{margin-top:20px}";
  html += ".log-entry{background:#f9f9f9;padding:10px;margin:5px 0;border-radius:3px;font-size:12px}";
  html += "</style></head><body>";

  html += "<div class='container'>";
  html += "<h1>📱 Electric Gripper Control</h1>";

  html += "<div class='status'>";
  html += "<div class='status-item'><div class='status-label'>Current Angle</div><div class='status-value' id='angle'>0</div></div>";
  html += "<div class='status-item'><div class='status-label'>Current (mA)</div><div class='status-value' id='current'>0</div></div>";
  html += "</div>";

  html += "<div class='buttons'>";
  html += "<button class='btn-open' onclick='api(\"/api/open\")'>🔓 Open</button>";
  html += "<button class='btn-close' onclick='api(\"/api/close\")'>🔒 Close</button>";
  html += "<button class='btn-stop' onclick='api(\"/api/stop\")'>⏸ Stop</button>";
  html += "<button class='btn-vibrate' onclick='api(\"/api/vibrate\")'>📳 Test Vibrate</button>";
  html += "</div>";

  html += "<div class='slider-container'>";
  html += "<label>Go to angle: <span id='angleValue'>90</span>°</label>";
  html += "<input type='range' min='0' max='270' value='90' oninput='document.getElementById(\"angleValue\").innerText=this.value'>";
  html += "<button onclick='goToAngle()'>Go</button>";
  html += "</div>";

  html += "<div class='logs'>";
  html += "<h3>Recent Activity</h3>";
  html += "<div id='logsContainer'></div>";
  html += "</div>";

  html += "</div>";

  html += "<script>";
  html += "function api(endpoint, method='POST'){fetch(endpoint,{method:method}).then(r=>r.json()).then(d=>updateStatus())}";
  html += "function goToAngle(){let angle=document.querySelector('input[type=\"range\"]').value;fetch('/api/go',{method:'POST',body:'angle='+angle})}";
  html += "function updateStatus(){";
  html += "fetch('/api/status').then(r=>r.json()).then(d=>{";
  html += "document.getElementById('angle').innerText=d.angle;";
  html += "document.getElementById('current').innerText=d.current.toFixed(0);";
  html += "});";
  html += "fetch('/api/logs').then(r=>r.json()).then(d=>{";
  html += "let html='';d.slice(-5).reverse().forEach(e=>{html+='<div class=\"log-entry\">'+e.action+' - '+e.angle+'° - '+e.current.toFixed(0)+'mA</div>'});";
  html += "document.getElementById('logsContainer').innerHTML=html;";
  html += "});";
  html += "}";
  html += "updateStatus();setInterval(updateStatus,1000);";
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "application/json", "{\"error\":\"Not Found\"}");
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
  if (digitalRead(SWITCH_OPEN) == LOW) {
    if (currentAngle != angleOpen) {
      Serial.println("🔓 Switch: Open");
      gripperOpen();
      delay(50);
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
  EEPROM.commit();

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
  Serial.println("  Electric Gripper Control System v2.1");
  Serial.println("           ESP32 WiFi Version");
  Serial.println("==========================================");
  Serial.println("✓ Servo: GPIO 5 (Wheeltech HW020 270°)");
  Serial.println("✓ INA219: I2C (GPIO 21/22)");
  Serial.println("✓ Switch: GPIO 35/34/39 (3-position)");
  Serial.println("✓ Vibration: GPIO 18");
  Serial.println("✓ Power: External 5V 3A");
  Serial.println("✓ Board: ESP32 Dev Module");
  Serial.println("✓ WiFi: " + String(wifiConnected ? "Connected ✓" : "Not connected ✗"));
  if (wifiConnected) {
    Serial.print("✓ Web: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
  }
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

  if (wifiConnected) {
    Serial.println("\n📱 WiFi Enabled:");
    Serial.print("  Web Dashboard: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
    Serial.println("  API: http://<IP>/api/<endpoint>");
  }

  Serial.println("=====================================\n");
}
