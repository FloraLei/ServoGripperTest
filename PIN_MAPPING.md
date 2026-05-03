# Arduino vs ESP32 - PIN 管脚对照表

## 快速参考

### 电动夹爪系统 PIN 映射

| 组件 | 功能 | Arduino | ESP32 | 电压 | 备注 |
|------|------|---------|-------|------|------|
| 舵机 | 信号线 | PIN 8 | GPIO 5 | 3.3V | 可改 |
| 打开按键 | 输入 | PIN 2 | GPIO 35 | 3.3V | 输入专用 |
| 关闭按键 | 输入 | PIN 3 | GPIO 34 | 3.3V | 输入专用 |
| 保持按键 | 输入 | PIN 4 | GPIO 39 | 3.3V | 输入专用 |
| 振动马达 | 驱动 | PIN 6 | GPIO 18 | 3.3V | 可改 |
| INA219 SCL | I2C 时钟 | A5 (PIN 19) | GPIO 22 | 3.3V | 固定 |
| INA219 SDA | I2C 数据 | A4 (PIN 18) | GPIO 21 | 3.3V | 固定 |

---

## 详细对比

### Arduino Uno PIN 功能表

```
┌─────────────────────────────────────────────┐
│          Arduino Uno Pin Layout             │
├─────────────────────────────────────────────┤
│ 数字引脚                                    │
│ PIN 0:  RX (串口接收)                       │
│ PIN 1:  TX (串口发送)                       │
│ PIN 2:  ✓ SWITCH_OPEN (打开按键)           │
│ PIN 3:  ✓ SWITCH_CLOSE (关闭按键)          │
│ PIN 4:  ✓ SWITCH_HOLD (保持按键)           │
│ PIN 5:  PWM 输出                            │
│ PIN 6:  ✓ VIBRATION_PIN (振动马达)         │
│ PIN 7:  数字 I/O                            │
│ PIN 8:  ✓ SERVO_PIN (舵机信号)             │
│ PIN 9:  PWM 输出                            │
│ PIN 10: PWM 输出                            │
│ PIN 11: PWM 输出                            │
│ PIN 12: 数字 I/O                            │
│ PIN 13: LED (内置)                          │
│                                             │
│ 模拟引脚                                    │
│ A0-A3:  模拟输入                            │
│ A4:     ✓ INA219 SDA (I2C 数据)            │
│ A5:     ✓ INA219 SCL (I2C 时钟)            │
└─────────────────────────────────────────────┘
```

### ESP32 GPIO PIN 功能表

```
┌─────────────────────────────────────────────┐
│          ESP32 Pin Layout                   │
├─────────────────────────────────────────────┤
│ GPIO 0:   启动模式选择（不用）               │
│ GPIO 1:   TX (UART0 发送)                   │
│ GPIO 2:   内部 LED                          │
│ GPIO 3:   RX (UART0 接收)                   │
│ GPIO 4:   数字 I/O                          │
│ GPIO 5:   ✓ SERVO_PIN (舵机信号)           │
│ GPIO 6-11: 内部使用（闪存控制）              │
│ GPIO 12:  启动模式选择（不用）               │
│ GPIO 13:  数字 I/O                          │
│ GPIO 14:  数字 I/O                          │
│ GPIO 15:  启动模式选择（不用）               │
│ GPIO 16:  数字 I/O                          │
│ GPIO 17:  数字 I/O                          │
│ GPIO 18:  ✓ VIBRATION_PIN (振动马达)       │
│ GPIO 19:  数字 I/O                          │
│ GPIO 20:  数字 I/O                          │
│ GPIO 21:  ✓ INA219 SDA (I2C 数据) - 固定   │
│ GPIO 22:  ✓ INA219 SCL (I2C 时钟) - 固定   │
│ GPIO 23:  数字 I/O                          │
│ GPIO 25:  数字 I/O                          │
│ GPIO 26:  数字 I/O                          │
│ GPIO 27:  数字 I/O                          │
│ GPIO 32:  ADC 输入                          │
│ GPIO 33:  ADC 输入                          │
│ GPIO 34:  ✓ SWITCH_CLOSE (关闭按键)        │
│           输入专用，无内部上拉               │
│ GPIO 35:  ✓ SWITCH_OPEN (打开按键)         │
│           输入专用，无内部上拉               │
│ GPIO 36:  输入专用                          │
│ GPIO 37:  输入专用                          │
│ GPIO 38:  输入专用                          │
│ GPIO 39:  ✓ SWITCH_HOLD (保持按键)         │
│           输入专用，无内部上拉               │
└─────────────────────────────────────────────┘

特殊说明：
- GPIO 6-11: 用于 SPI Flash，无法用户使用
- GPIO 34/35/36/37/38/39: 输入专用
- GPIO 0, 2, 12, 15: 启动模式选择引脚，使用受限
```

---

## 硬件连接示意图

### Arduino Uno 连接

```
Arduino          元件
────────────────────────────────────────
5V ───┬─ Servo Power (红)
      ├─ INA219 5V
      └─ 按键上拉 4.7kΩ

GND ──┬─ Servo GND (黑)
      ├─ INA219 GND
      ├─ 按键 GND

PIN 2 ─ 打开按键
PIN 3 ─ 关闭按键
PIN 4 ─ 保持按键

PIN 8 ─ 舵机信号 (白)

PIN 6 ─ 振动马达 (+)
      └─ GND (-)

A4 ─ INA219 SDA
A5 ─ INA219 SCL

注：所有元件需要共享 GND
```

### ESP32 连接（关键改动）

```
ESP32           元件
────────────────────────────────────────
3.3V ──┬─ INA219 3.3V
       └─ 按键上拉 4.7kΩ

5V ──── ─ Servo Power (红) - 独立 5V 电源
        └ 振动马达 5V（如果使用 ULN2003）

GND ──┬─ Servo GND (黑)
      ├─ INA219 GND
      ├─ 按键 GND
      ├─ 振动马达 GND

GPIO 5 ─ 舵机信号 (白)

GPIO 35 ─ 打开按键
GPIO 34 ─ 关闭按键
GPIO 39 ─ 保持按键
          （各有 4.7kΩ 上拉到 3.3V）

GPIO 18 ─ 振动马达 (+)
         └─ GND (-)

GPIO 21 ─ INA219 SDA
GPIO 22 ─ INA219 SCL

注意事项：
1. GPIO 34/35/39 只能输入，不能输出
2. 按键需要外部上拉电阻（4.7kΩ）
3. 舵机仍需 5V 单独电源
4. ESP32 本体是 3.3V
5. GND 必须相连
```

---

## 代码中的差异

### Arduino 代码

```cpp
// PIN 定义
const int SERVO_PIN = 8;
const int SWITCH_OPEN = 2;
const int SWITCH_CLOSE = 3;
const int SWITCH_HOLD = 4;
const int VIBRATION_PIN = 6;

// 初始化
void setup() {
  Serial.begin(9600);           // Arduino 波特率
  
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(SWITCH_OPEN, INPUT_PULLUP);   // 使用内部上拉
  pinMode(SWITCH_CLOSE, INPUT_PULLUP);
  pinMode(SWITCH_HOLD, INPUT_PULLUP);
  
  Wire.begin();                 // 默认 I2C PIN
}
```

### ESP32 代码

```cpp
// PIN 定义
const int SERVO_PIN = 5;
const int SWITCH_OPEN = 35;
const int SWITCH_CLOSE = 34;
const int SWITCH_HOLD = 39;
const int VIBRATION_PIN = 18;

// 初始化
void setup() {
  Serial.begin(115200);         // ESP32 推荐波特率
  
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(SWITCH_OPEN, INPUT);           // 无内部上拉
  pinMode(SWITCH_CLOSE, INPUT);          // 需要外部上拉
  pinMode(SWITCH_HOLD, INPUT);
  
  Wire.begin(21, 22);           // 指定 I2C PIN
  
  EEPROM.begin(512);            // 初始化 EEPROM
}

// 保存数据时
void saveConfig() {
  EEPROM.put(...);
  EEPROM.commit();              // 必须提交
}
```

---

## 电压兼容性

### 3.3V vs 5V

| 项目 | Arduino | ESP32 | 说明 |
|------|---------|-------|------|
| 逻辑电压 | 5V | 3.3V | 不兼容 |
| 舵机信号 | 5V 接受 | 3.3V 可用 | ESP32 3.3V 足够驱动舵机 |
| INA219 | 5V 供电 | 3.3V 供电 | INA219 支持 3.3V |
| 按键电压 | 5V | 3.3V | GPIO 34/35/39 最大 3.3V |
| 振动马达 | 5V | 5V (独立) | 需要单独 5V 电源 |

**关键点：** ESP32GPIO 引脚是 3.3V 的，但大多数 5V 传感器可以接受 3.3V 信号。

---

## 可选 PIN 替换

如果默认 PIN 有冲突，可以改为其他 GPIO：

### 舵机信号（可改）

```
当前：GPIO 5
可选：GPIO 12, 13, 14, 16, 17, 18, 19, 23, 25, 26, 27 等
避免：GPIO 0, 2, 4, 15（启动模式相关）
```

### 振动马达（可改）

```
当前：GPIO 18
可选：GPIO 12, 13, 14, 16, 17, 19, 23, 25, 26, 27 等
避免：GPIO 0, 2, 4, 15（启动模式相关）
```

### 按键（固定）

```
当前：GPIO 34, 35, 39（输入专用）
如需改动：必须使用其他输入专用引脚
          GPIO 32, 33, 36, 37, 38
          
注意：必须用输入专用引脚，否则启动会失败
```

---

## 实用对照速查表

### "我想连接到 Arduino PIN X，应该用 ESP32 的哪个 PIN？"

| Arduino | 对等 ESP32 | 类型 | 备注 |
|---------|-----------|------|------|
| PIN 0/1 | GPIO 3/1 | UART | 串口 |
| PIN 2 | GPIO 35 | 输入 | 打开按键 |
| PIN 3 | GPIO 34 | 输入 | 关闭按键 |
| PIN 4 | GPIO 39 | 输入 | 保持按键 |
| PIN 5 | GPIO 12 | PWM | 可用 |
| PIN 6 | GPIO 18 | PWM | 振动马达 |
| PIN 7 | GPIO 14 | I/O | 可用 |
| PIN 8 | GPIO 5 | I/O | 舵机控制 |
| PIN 9 | GPIO 25 | PWM | 可用 |
| PIN 10 | GPIO 26 | PWM | 可用 |
| PIN 11 | GPIO 27 | PWM | 可用 |
| PIN 12 | GPIO 13 | I/O | 可用 |
| PIN 13 | GPIO 2 | I/O | 内置 LED |
| A0 | GPIO 36 | ADC | 输入专用 |
| A1 | GPIO 37 | ADC | 输入专用 |
| A2 | GPIO 38 | ADC | 输入专用 |
| A3 | GPIO 32 | ADC | 可用 |
| A4 | GPIO 21 | I2C | SDA (固定) |
| A5 | GPIO 22 | I2C | SCL (固定) |

---

## 调试技巧

### 检查 PIN 状态

**Arduino：**
```cpp
Serial.println(digitalRead(PIN));  // 输出 HIGH 或 LOW
```

**ESP32：**
```cpp
Serial.println(digitalRead(GPIO));  // 同样方式
Serial.println(analogRead(GPIO));   // ADC 读值（仅 ADC 引脚）
```

### 获取 Pin 信息

```cpp
// ESP32 获取当前频率
Serial.println(ESP.getFreqMHz());  // 返回 240 (MHz)

// 获取剩余内存
Serial.println(ESP.getFreeHeap());  // 返回剩余字节数
```

---

## 常见连接错误

### ❌ 错误 1：按键直接接 3.3V

```
错误：GPIO 35 ─ 3.3V
后果：按键不工作，因为无法拉低

正确：GPIO 35 ─┬─ [4.7kΩ] ─ 3.3V
             └─ [按键] ─ GND
```

### ❌ 错误 2：I2C 用错 PIN

```
错误：任意 GPIO 接 INA219
后果：无法通讯，"INA219 not found"

正确：GPIO 21 ─ INA219 SDA
      GPIO 22 ─ INA219 SCL
```

### ❌ 错误 3：舵机信号用 OUTPUT_OPEN_DRAIN

```
错误：使用不正确的 GPIO 模式
后果：舵机无反应

正确：pinMode(GPIO_5, OUTPUT);  // 标准输出
```

### ❌ 错误 4：不调用 EEPROM.commit()

```
错误：
  EEPROM.put(addr, value);
  // 缺少 commit
  
后果：重启后配置丢失

正确：
  EEPROM.put(addr, value);
  EEPROM.commit();
```

---

## 总结检查清单

迁移到 ESP32 前，确认：

- [ ] 舵机信号：GPIO 5
- [ ] 打开按键：GPIO 35 + 4.7kΩ 上拉
- [ ] 关闭按键：GPIO 34 + 4.7kΩ 上拉
- [ ] 保持按键：GPIO 39 + 4.7kΩ 上拉
- [ ] INA219：GPIO 21 (SDA) + GPIO 22 (SCL)
- [ ] 振动马达：GPIO 18
- [ ] 所有 GND 相连
- [ ] 舵机有独立 5V 电源
- [ ] 按键用 INPUT，不用 INPUT_PULLUP
- [ ] I2C 初始化：Wire.begin(21, 22)
- [ ] EEPROM 提交：EEPROM.commit()
- [ ] 波特率：115200

准备好了吗？开始迁移吧！ 🚀
