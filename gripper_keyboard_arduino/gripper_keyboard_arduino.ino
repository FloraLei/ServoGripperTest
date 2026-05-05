/*
 * Electric Gripper Control - Keyboard Version (Arduino)
 *
 * 键盘控制（Serial Monitor 设置为 Newline 模式，每次按完键再按 Enter）：
 *   'a'  →  打开夹爪
 *   'q'  →  关闭夹爪
 *   不输入  →  保持当前位置（target 不变）
 *
 * 硬件接线：
 *   舵机信号线       → PIN 8
 *   振动马达模块：
 *     IN             → PIN 6
 *     VCC            → 5V
 *     GND            → GND
 *   INA219（串联在舵机电源线上，测量舵机实际电流）：
 *     VCC            → 3.3V 或 5V
 *     GND            → GND
 *     SDA            → A4
 *     SCL            → A5
 *     VIN+           → 外部 5V 电源正极
 *     VIN-           → 舵机 VCC（红线）
 *
 * 内存说明：Arduino Uno 只有 2KB SRAM。
 *   字符串字面量全部用 F() 宏存入 flash，节省 SRAM。
 *   日志缓冲区限制 30 条（约 660 字节）。
 */

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <EEPROM.h>

// =========================================================
// PIN 定义
// =========================================================
const int SERVO_PIN     = 8;   // 舵机 PWM 信号
const int VIBRATION_PIN = 6;   // 振动马达 IN 脚

// =========================================================
// 舵机物理量程（Wheeltech HW020，实测 270° 型）
// =========================================================
const int ANGLE_MAX = 270;     // 舵机最大角度，用于 constrain() 边界检查

// =========================================================
// EEPROM 地址布局（每个 int 占 2 字节）
//   0  angleOpen
//   2  angleClose
//   4  moveSpeed
//   6  currentThresh
//   8  vibrateTime
//   10 gripThresh
// =========================================================
const int EEPROM_ADDR_OPEN     = 0;
const int EEPROM_ADDR_CLOSE    = 2;
const int EEPROM_ADDR_SPEED    = 4;
const int EEPROM_ADDR_CURR_THR = 6;
const int EEPROM_ADDR_VIBTIME  = 8;
const int EEPROM_ADDR_GRIP_THR = 10;

// =========================================================
// 默认值（const）——改这里，然后烧录后执行 "reset config" 即可生效
// =========================================================
const int DEF_ANGLE_OPEN     = 130;  // 打开限位（实测）
const int DEF_ANGLE_CLOSE    = 200;  // 关闭限位（实测）
const int DEF_MOVE_SPEED     = 10;   // 速度：10 → 全程约 0.7s
const int DEF_CURRENT_THRESH = 300;  // 突变警报阈值 (mA)
const int DEF_VIBRATE_TIME   = 1000; // 振动时长 (ms)
const int DEF_GRIP_THRESH    = 200;  // 夹持停止阈值 (mA)

// =========================================================
// 可调参数（运行时通过 set 命令修改；EEPROM 持久化）
// =========================================================
int angleOpen     = DEF_ANGLE_OPEN;
int angleClose    = DEF_ANGLE_CLOSE;
int moveSpeed     = DEF_MOVE_SPEED;
int currentThresh = DEF_CURRENT_THRESH;
int vibrateTime   = DEF_VIBRATE_TIME;
int gripThresh    = DEF_GRIP_THRESH;

// =========================================================
// 运行状态变量
// =========================================================
// 初始值在 setup() 里 loadConfig() 之后会被覆盖为 angleOpen
int   currentAngle = 0;
int   targetAngle  = 0;

float currentCurrent = 0;   // INA219 最新电流读数（mA）
float currentVoltage = 0;   // INA219 最新总线电压（V）
float lastCurrent    = 0;   // 上次电流读数，用于计算 delta

bool  vibrating   = false;  // 振动马达是否正在运行
bool  monitorMode = false;  // 实时监控模式开关（每 500ms 自动打印一行）
bool  closing     = false;  // 当前是否处于关闭动作中（用于夹持保护的触发条件）
bool  gripLocked  = false;  // 夹持锁定：检测到夹持后置 true，必须先执行 open 才能再次关闭
                            // 防止按住 'q' 键自动重复时绕过夹持保护
float peakCurrent = 0;      // 本次关闭过程中采样到的最大电流（触发时打印）

unsigned long vibrationEndTime    = 0;
unsigned long lastMoveTime        = 0;
unsigned long lastCurrentReadTime = 0;
unsigned long lastMonitorTime     = 0;

// 各任务执行间隔（ms）
const int MOVE_INTERVAL         = 100;  // 舵机步进周期
const int CURRENT_READ_INTERVAL = 20;   // INA219 采样周期（越短响应越快，最小约20ms）
const int MONITOR_INTERVAL      = 500;  // 实时监控打印周期

// =========================================================
// INA219 电流传感器
// =========================================================
Adafruit_INA219 ina219;
bool ina219Ready = false;   // 初始化成功后置 true，失败则跳过所有电流相关逻辑

// =========================================================
// 日志（环形缓冲，满了从头覆盖）
// =========================================================
struct LogEntry {
  unsigned long timestamp;  // 秒
  int   angle;
  float current;
  char  action[12];
};

#define MAX_LOGS 30          // Uno SRAM 限制，30 条约占 660 字节
LogEntry logs[MAX_LOGS];
int logCount = 0;

// =========================================================
// SETUP
// =========================================================
void setup() {
  Serial.begin(9600);
  delay(500);

  pinMode(SERVO_PIN,     OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);

  Wire.begin();
  if (ina219.begin()) {
    ina219Ready = true;
    // 启动后立刻读总线电压，用于确认 VIN+ 接线是否正常
    // 正常应接近供电电压（约 4.8–5.0V）；接近 0 说明 VIN+ 未接
    float v = ina219.getBusVoltage_V();
    Serial.print(F("[OK] INA219 ready  Bus voltage: "));
    Serial.print(v, 2);
    Serial.println(F(" V"));
    if (v < 1.0) Serial.println(F("[WARN] Bus voltage very low - check VIN+ wiring"));
  } else {
    Serial.println(F("[WARN] INA219 not found - check SDA/SCL/VCC"));
  }

  loadConfig();

  // 上电后立刻物理移动到打开限位（130°）
  // 注意：moveToAngle() 只设变量，loop() 才发脉冲
  // 所以这里直接发 PWM，确保 setup 结束前舵机就已到位
  currentAngle = angleOpen;
  targetAngle  = angleOpen;
  {
    int pw = getPulseForAngle(angleOpen);
    // 100 个脉冲 ≈ 2 秒，足够舵机从任意位置物理移动到 130°
    for (int i = 0; i < 100; i++) {
      digitalWrite(SERVO_PIN, HIGH);
      delayMicroseconds(pw);
      digitalWrite(SERVO_PIN, LOW);
      delayMicroseconds(20000 - pw);
    }
  }
  Serial.print(F("-> Parked at open limit: "));
  Serial.print(angleOpen);
  Serial.println(F("deg"));

  printBanner();
  printFullStatus();   // 上电后打印一次完整状态，方便确认配置和硬件
}

// =========================================================
// MAIN LOOP
// =========================================================
void loop() {
  unsigned long now = millis();

  // 每 100ms 步进一次舵机（步长 = moveSpeed 度）
  if (now - lastMoveTime >= MOVE_INTERVAL) {
    updateServoPosition();
    lastMoveTime = now;
  }

  // 每 100ms 读一次电流，先做夹持保护再做突变检测
  // 顺序重要：grip 检测优先，避免同一次采样被 spike 逻辑先消费掉
  if (now - lastCurrentReadTime >= CURRENT_READ_INTERVAL) {
    readCurrent();
    checkGripDetect();
    checkCurrentSpike();
    lastCurrentReadTime = now;
  }

  // 振动到时自动停止
  if (vibrating && now >= vibrationEndTime) {
    stopVibration();
  }

  // 实时监控模式：每 500ms 打印一行当前状态
  if (monitorMode && now - lastMonitorTime >= MONITOR_INTERVAL) {
    printLiveStatus();
    lastMonitorTime = now;
  }

  if (Serial.available() > 0) {
    handleSerial();
  }

  delay(10);
}

// =========================================================
// 串口输入处理
// =========================================================
void handleSerial() {
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();
  if (cmd.length() == 0) return;

  // 单键快捷指令
  if (cmd == "a") {
    gripperOpen();
  } else if (cmd == "q") {
    gripperClose();
  } else if (cmd == "h" || cmd == "hold" || cmd == "stop" || cmd == "s") {
    gripperHold();
  } else if (cmd == "o" || cmd == "open") {
    gripperOpen();
  } else if (cmd == "c" || cmd == "close") {
    gripperClose();

  // 直接指定角度
  } else if (cmd.startsWith(F("go "))) {
    int angle = cmd.substring(3).toInt();
    if (angle >= 0 && angle <= ANGLE_MAX) moveToAngle(angle);
    else Serial.println(F("[ERR] Angle must be 0-270"));

  // 参数设置（修改后自动保存到 EEPROM）
  } else if (cmd.startsWith(F("set open "))) {
    angleOpen = constrain(cmd.substring(9).toInt(), 0, ANGLE_MAX);
    Serial.print(F("[OK] Open = ")); Serial.print(angleOpen); Serial.println(F("deg"));
    saveConfig();
  } else if (cmd.startsWith(F("set close "))) {
    angleClose = constrain(cmd.substring(10).toInt(), 0, ANGLE_MAX);
    Serial.print(F("[OK] Close = ")); Serial.print(angleClose); Serial.println(F("deg"));
    saveConfig();
  } else if (cmd.startsWith(F("set speed "))) {
    // 1=最慢，10=最快；全程时间 = 行程÷speed×0.1 秒
    moveSpeed = constrain(cmd.substring(10).toInt(), 1, 10);
    Serial.print(F("[OK] Speed = ")); Serial.print(moveSpeed); Serial.println(F("deg/100ms"));
    saveConfig();
  } else if (cmd.startsWith(F("set threshold "))) {
    currentThresh = constrain(cmd.substring(14).toInt(), 50, 1000);
    Serial.print(F("[OK] Threshold = ")); Serial.print(currentThresh); Serial.println(F("mA"));
    saveConfig();
  } else if (cmd.startsWith(F("set vibrate "))) {
    vibrateTime = constrain(cmd.substring(12).toInt(), 50, 1000);
    Serial.print(F("[OK] Vibrate = ")); Serial.print(vibrateTime); Serial.println(F("ms"));
    saveConfig();
  } else if (cmd.startsWith(F("set grip "))) {
    gripThresh = constrain(cmd.substring(9).toInt(), 50, 600);
    Serial.print(F("[OK] Grip threshold = ")); Serial.print(gripThresh); Serial.println(F("mA"));
    saveConfig();

  // 状态查询
  } else if (cmd == "pos" || cmd == "position") {
    Serial.print(F("Angle: ")); Serial.print(currentAngle);
    Serial.print(F("deg  |  "));
    Serial.print(currentVoltage, 2); Serial.print(F("V  "));
    Serial.print(currentCurrent, 0); Serial.println(F("mA"));
  } else if (cmd == "current") {
    if (ina219Ready) {
      Serial.print(F("Voltage: ")); Serial.print(currentVoltage, 2);
      Serial.print(F("V  Current: ")); Serial.print(currentCurrent, 0); Serial.println(F("mA"));
    } else {
      Serial.println(F("[ERR] INA219 not available"));
    }
  } else if (cmd == "mon" || cmd == "monitor") {
    monitorMode = !monitorMode;
    Serial.print(F("[MON] "));
    Serial.println(monitorMode ? F("ON  (angle|V|mA every 500ms)") : F("OFF"));

  } else if (cmd == "status" || cmd == "st") {
    printFullStatus();
  } else if (cmd == "config") {
    showConfig();
  } else if (cmd == "log") {
    showLogs();
  } else if (cmd == "clear log") {
    logCount = 0;
    Serial.println(F("[OK] Log cleared"));
  } else if (cmd == "test vibrate") {
    startVibration();
  } else if (cmd == "reset config") {
    resetToDefaults();
  } else if (cmd == "?" || cmd == "help") {
    printBanner();
  } else {
    Serial.print(F("[?] Unknown: ")); Serial.println(cmd);
  }
}

// =========================================================
// 夹爪动作
// =========================================================
void gripperOpen() {
  closing    = false;
  gripLocked = false;   // 执行开启动作时解锁，允许下次重新关闭
  Serial.println(F("[OPEN]"));
  moveToAngle(angleOpen);
  logAction(angleOpen, "open");
}

void gripperClose() {
  // 允许在 gripLocked 状态下继续按 'q' 来增加夹紧力度
  // 如果已经检测到夹持，继续关闭时会再次进入 checkGripDetect() 逻辑
  closing     = true;   // 标记进入关闭过程，使能夹持保护检测
  
  if (!gripLocked) {
    // 首次关闭动作
    peakCurrent = 0;
    Serial.println(F("[CLOSE]"));
    moveToAngle(angleClose);
  } else {
    // 已夹持，继续关闭：目标角度增加 3 度，逐步增加夹力
    int nextAngle = min(targetAngle + 3, angleClose + 10);  // 最多增加10度以防损坏
    Serial.print(F("[CLOSE+] Increase grip to "));
    Serial.print(nextAngle);
    Serial.println(F("deg"));
    moveToAngle(nextAngle);
  }
  logAction(targetAngle, "close");
}

void gripperHold() {
  closing     = false;
  targetAngle = currentAngle;   // 原地冻结，不修改 gripLocked
  Serial.print(F("[HOLD] ")); Serial.print(currentAngle); Serial.println(F("deg"));
  logAction(currentAngle, "hold");
}

// =========================================================
// 舵机控制
// =========================================================

// 将角度线性插值为对应的 PWM 脉宽（微秒）
// 校准数据来自 Wheeltech HW020 实测（7 个等间距锚点）
int getPulseForAngle(int angle) {
  if (angle <= 0)   return 500;
  if (angle >= 270) return 2600;

  const int breakpoints[] = {0, 45, 90, 135, 180, 225, 270};
  const int pulses[]      = {500, 850, 1200, 1550, 1900, 2250, 2600};

  for (int i = 0; i < 6; i++) {
    if (angle >= breakpoints[i] && angle <= breakpoints[i + 1]) {
      int span  = breakpoints[i + 1] - breakpoints[i];
      int delta = angle - breakpoints[i];
      return pulses[i] + ((pulses[i + 1] - pulses[i]) * delta / span);
    }
  }
  return 1550;   // 不应到达，保险返回中位值
}

// 发送 20 个 PWM 脉冲（约 400ms）维持舵机位置
// 数量少于原版（100个）是为了让 loop() 每 100ms 能及时响应其他任务
void sendPWMPulse(int angle) {
  int pw = getPulseForAngle(angle);
  for (int i = 0; i < 20; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pw);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pw);  // 20ms 周期减去高电平时间
  }
}

// 每 100ms 调用一次，按 moveSpeed 步进靠近目标角度
void updateServoPosition() {
  if (currentAngle < targetAngle) {
    currentAngle = min(currentAngle + moveSpeed, targetAngle);
  } else if (currentAngle > targetAngle) {
    currentAngle = max(currentAngle - moveSpeed, targetAngle);
  } else if (closing && !gripLocked) {
    // 自然走到 angleClose 但没触发夹持保护，说明没夹到东西，清除 closing 标志
    // 如果已经 gripLocked，保持 closing 状态，允许用户继续按 'q' 来尝试
    closing = false;
  }
  sendPWMPulse(currentAngle);
}

// 设置目标角度，打印移动意图和当前电流（方便观察下令瞬间的负载状态）
void moveToAngle(int angle) {
  targetAngle = constrain(angle, 0, ANGLE_MAX);
  Serial.print(F("-> ")); Serial.print(targetAngle);
  Serial.print(F("deg  |  now: "));
  Serial.print(currentVoltage, 2); Serial.print(F("V  "));
  Serial.print(currentCurrent, 0); Serial.println(F("mA"));
}

// =========================================================
// 夹持保护
// =========================================================

// 只在 closing==true 时生效，避免开启或保持过程中误触发
// 首次检测到夹持后会设 gripLocked=true 并振动
// 如果在已夹持状态下用户继续按 'q'（closing=true），可以继续向 angleClose 靠近
void checkGripDetect() {
  if (!closing || !ina219Ready) return;

  // 持续追踪本次关闭过程中的电流峰值
  if (currentCurrent > peakCurrent) peakCurrent = currentCurrent;

  if (currentCurrent > gripThresh) {
    if (!gripLocked) {
      // 首次检测到夹持：停住 + 振动反馈
      gripLocked  = true;
      targetAngle = currentAngle;   // 就地停住
      Serial.print(F("[GRIP] Object at "));
      Serial.print(currentAngle);
      Serial.print(F("deg  detect="));
      Serial.print(currentCurrent, 0);
      Serial.print(F("mA  peak="));
      Serial.print(peakCurrent, 0);
      Serial.println(F("mA  (press q again to squeeze harder or a to open)"));
      startVibration();
      logAction(currentAngle, "grip");
    } else {
      // 已经夹持，继续按 'q' 时如果电流仍然过高，保持停住
      // 但不再改变 targetAngle，让用户可以决定下一步（再按 q 或按 a 打开）
      Serial.print(F("[GRIP+] Still at "));
      Serial.print(currentAngle);
      Serial.print(F("deg  current="));
      Serial.print(currentCurrent, 0);
      Serial.println(F("mA"));
    }
  }
}

// =========================================================
// 电流监控
// =========================================================
void readCurrent() {
  if (!ina219Ready) return;
  currentCurrent = ina219.getCurrent_mA();
  currentVoltage = ina219.getBusVoltage_V();
}

// 检测电流突变（与夹持保护无关，捕捉意外冲击或机械故障）
// 用 delta 而非绝对值，对不同负载环境更鲁棒
void checkCurrentSpike() {
  float delta = abs(currentCurrent - lastCurrent);
  if (delta >= currentThresh) {
    Serial.print(F("[SPIKE] delta=")); Serial.print(delta, 0);
    Serial.print(F("mA  abs=")); Serial.print(currentCurrent, 0);
    Serial.print(F("mA  ")); Serial.print(currentVoltage, 2); Serial.println(F("V"));
    startVibration();
    logAction(currentAngle, "spike");
  }
  lastCurrent = currentCurrent;
}

// 实时监控模式下的单行状态输出（紧凑格式，方便观察电流曲线）
void printLiveStatus() {
  Serial.print(currentAngle);
  Serial.print(F("deg | "));
  Serial.print(currentVoltage, 2);
  Serial.print(F("V | "));
  Serial.print(currentCurrent, 0);
  Serial.println(F("mA"));
}

// =========================================================
// 振动马达
// =========================================================
// 振动马达模块触发极性：大多数模块 HIGH=ON，但部分 NPN 模块 LOW=ON
// 如果 test vibrate 没反应，把下面两行的 HIGH/LOW 对调
#define VIB_ON  HIGH
#define VIB_OFF LOW

void startVibration() {
  if (!vibrating) {
    digitalWrite(VIBRATION_PIN, VIB_ON);
    vibrating        = true;
    vibrationEndTime = millis() + vibrateTime;
  }
}

void stopVibration() {
  digitalWrite(VIBRATION_PIN, VIB_OFF);
  vibrating = false;
}

// =========================================================
// 日志
// =========================================================

// 环形缓冲：满 30 条后丢弃最旧的一条
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
  Serial.println(F("\nTime(s)|Angle|Current(mA)|Action"));
  Serial.println(F("-------|-----|-----------|------"));
  for (int i = 0; i < logCount; i++) {
    Serial.print(logs[i].timestamp); Serial.print(F("|"));
    Serial.print(logs[i].angle);     Serial.print(F("|"));
    Serial.print(logs[i].current, 0);Serial.print(F("|"));
    Serial.println(logs[i].action);
  }
  Serial.println();
}

// =========================================================
// 配置持久化（EEPROM）
// =========================================================
void loadConfig() {
  EEPROM.get(EEPROM_ADDR_OPEN,     angleOpen);
  EEPROM.get(EEPROM_ADDR_CLOSE,    angleClose);
  EEPROM.get(EEPROM_ADDR_SPEED,    moveSpeed);
  EEPROM.get(EEPROM_ADDR_CURR_THR, currentThresh);
  EEPROM.get(EEPROM_ADDR_VIBTIME,  vibrateTime);
  EEPROM.get(EEPROM_ADDR_GRIP_THR, gripThresh);

  // EEPROM 首次使用时可能读到全 0xFF，需要回退到代码默认值（DEF_* 常量）
  if (angleOpen     < 0   || angleOpen     > ANGLE_MAX) angleOpen     = DEF_ANGLE_OPEN;
  if (angleClose    < 0   || angleClose    > ANGLE_MAX) angleClose    = DEF_ANGLE_CLOSE;
  if (moveSpeed     < 1   || moveSpeed     > 10)        moveSpeed     = DEF_MOVE_SPEED;
  if (currentThresh < 50  || currentThresh > 1000)      currentThresh = DEF_CURRENT_THRESH;
  if (vibrateTime   < 50  || vibrateTime   > 1000)      vibrateTime   = DEF_VIBRATE_TIME;
  if (gripThresh    < 50  || gripThresh    > 600)        gripThresh    = DEF_GRIP_THRESH;
}

void saveConfig() {
  EEPROM.put(EEPROM_ADDR_OPEN,     angleOpen);
  EEPROM.put(EEPROM_ADDR_CLOSE,    angleClose);
  EEPROM.put(EEPROM_ADDR_SPEED,    moveSpeed);
  EEPROM.put(EEPROM_ADDR_CURR_THR, currentThresh);
  EEPROM.put(EEPROM_ADDR_VIBTIME,  vibrateTime);
  EEPROM.put(EEPROM_ADDR_GRIP_THR, gripThresh);
  Serial.println(F("[OK] Config saved"));
}

// 将所有参数重置为代码里的 DEF_* 常量，并写入 EEPROM
// 使用场景：烧录新代码后，EEPROM 里还存着旧值时，执行一次即可同步
void resetToDefaults() {
  angleOpen     = DEF_ANGLE_OPEN;
  angleClose    = DEF_ANGLE_CLOSE;
  moveSpeed     = DEF_MOVE_SPEED;
  currentThresh = DEF_CURRENT_THRESH;
  vibrateTime   = DEF_VIBRATE_TIME;
  gripThresh    = DEF_GRIP_THRESH;
  saveConfig();
  Serial.println(F("[OK] All params reset to code defaults"));
  showConfig();
}

// 一屏打印所有状态：硬件、舵机位置、夹爪逻辑状态、配置参数
void printFullStatus() {
  Serial.println(F("\n========== SYSTEM STATUS =========="));

  // ---- 硬件 ----
  Serial.println(F("[硬件]"));
  if (ina219Ready) {
    Serial.print(F("  INA219  : OK  |  "));
    Serial.print(currentVoltage, 2); Serial.print(F("V  |  "));
    Serial.print(currentCurrent, 0); Serial.println(F("mA"));
  } else {
    Serial.println(F("  INA219  : 未连接（电流保护功能不可用）"));
  }

  // ---- 舵机位置 ----
  Serial.println(F("[舵机]"));
  Serial.print(F("  当前角度 : ")); Serial.print(currentAngle); Serial.print(F("deg"));
  if      (currentAngle == angleOpen)  Serial.println(F("  (完全打开)"));
  else if (currentAngle == angleClose) Serial.println(F("  (完全关闭)"));
  else                                  Serial.println(F("  (中间位置)"));
  Serial.print(F("  目标角度 : ")); Serial.print(targetAngle);  Serial.println(F("deg"));
  Serial.print(F("  移动状态 : "));
  if      (currentAngle == targetAngle) Serial.println(F("静止"));
  else if (currentAngle < targetAngle)  Serial.println(F("关闭中 →"));
  else                                   Serial.println(F("打开中 ←"));

  // ---- 夹爪逻辑状态 ----
  Serial.println(F("[夹爪状态]"));
  Serial.print(F("  closing   : ")); Serial.println(closing    ? F("YES（关闭过程中）") : F("NO"));
  Serial.print(F("  gripLocked: ")); Serial.println(gripLocked ? F("YES（已夹持，需先 a 开启）") : F("NO"));

  // ---- 配置参数 ----
  Serial.println(F("[配置参数]"));
  Serial.print(F("  Open     : ")); Serial.print(angleOpen);     Serial.println(F("deg  （打开限位）"));
  Serial.print(F("  Close    : ")); Serial.print(angleClose);    Serial.println(F("deg  （关闭限位）"));

  // 根据当前参数计算实际全程耗时
  int stroke = abs(angleClose - angleOpen);
  float tSec = (float)stroke / moveSpeed * 0.1;
  Serial.print(F("  Speed    : ")); Serial.print(moveSpeed);
  Serial.print(F("deg/100ms  → 全程约 ")); Serial.print(tSec, 1); Serial.println(F("s"));

  Serial.print(F("  Grip thr : ")); Serial.print(gripThresh);    Serial.println(F("mA  （夹持停止）"));
  Serial.print(F("  Spike thr: ")); Serial.print(currentThresh); Serial.println(F("mA  （突变警报）"));
  Serial.print(F("  Vibrate  : ")); Serial.print(vibrateTime);   Serial.println(F("ms"));

  Serial.println(F("====================================\n"));
}

void showConfig() {
  Serial.println(F("\n===== CONFIG ====="));
  Serial.print(F("Open  : ")); Serial.print(angleOpen);     Serial.println(F("deg"));
  Serial.print(F("Close : ")); Serial.print(angleClose);    Serial.println(F("deg"));
  Serial.print(F("Speed : ")); Serial.print(moveSpeed);     Serial.println(F("deg/100ms  (1=慢 10=快)"));
  Serial.print(F("Thr   : ")); Serial.print(currentThresh); Serial.println(F("mA  (突变检测 delta)"));
  Serial.print(F("Grip  : ")); Serial.print(gripThresh);    Serial.println(F("mA  (夹持停止)"));
  Serial.print(F("Vib   : ")); Serial.print(vibrateTime);   Serial.println(F("ms"));
  Serial.println(F("==================\n"));
}

void printBanner() {
  Serial.println(F("\n=== Gripper Keyboard Control (Arduino) ==="));
  Serial.println(F("  a         - 打开夹爪"));
  Serial.println(F("  q         - 关闭夹爪（夹到物体自动停）"));
  Serial.println(F("  (no key)  - 保持当前位置"));
  Serial.println(F("------------------------------------------"));
  Serial.println(F("  go <deg>           - 移动到指定角度 0-270"));
  Serial.println(F("  status / st        - 完整系统状态（一屏总览）"));
  Serial.println(F("  pos                - 当前角度 + 电压 + 电流"));
  Serial.println(F("  current            - 电压 + 电流"));
  Serial.println(F("  mon                - 切换实时监控（每 500ms）"));
  Serial.println(F("  config             - 查看所有参数"));
  Serial.println(F("  log / clear log    - 历史记录"));
  Serial.println(F("  set speed <1-10>   - 调速"));
  Serial.println(F("  set grip <mA>      - 夹持停止阈值"));
  Serial.println(F("  set open/close/threshold/vibrate <n>"));
  Serial.println(F("  reset config            - 恢复代码默认值并写入 EEPROM"));
  Serial.println(F("==========================================\n"));
}
