# Arduino → ESP32 迁移指南

## 概述

本指南帮助你从 Arduino Uno 迁移到 ESP32 开发板。代码 99% 兼容，只需修改 PIN 定义和部分配置。

---

## 硬件对比

### PIN 管脚对应表

| 功能 | Arduino | ESP32 | 备注 |
|------|---------|-------|------|
| **舵机控制** | PIN 8 | GPIO 5 | 可改 |
| **打开按键** | PIN 2 | GPIO 35 | **输入专用** |
| **关闭按键** | PIN 3 | GPIO 34 | **输入专用** |
| **保持按键** | PIN 4 | GPIO 39 | **输入专用** |
| **振动反馈** | PIN 6 | GPIO 18 | 可改 |
| **I2C SCL** | A5 (PIN 19) | GPIO 22 | 固定 |
| **I2C SDA** | A4 (PIN 18) | GPIO 21 | 固定 |

### 关键差异

| 方面 | Arduino | ESP32 |
|------|---------|-------|
| 主频 | 16 MHz | 240 MHz |
| 内存 | 2 KB RAM | 520 KB RAM |
| 存储 | 1 KB EEPROM | 4 KB EEPROM (可用) |
| I2C | 需配置 | GPIO 21/22 (默认) |
| 串口波特率 | 9600 | 115200 (推荐) |
| 输入专用引脚 | 无 | GPIO 34/35/39 |
| 电压 | 5V | 3.3V |

---

## 硬件连接变更

### 舵机连接
```
Arduino:                  ESP32:
PIN 8 → Servo Signal      GPIO 5 → Servo Signal
5V → Servo Power          5V → Servo Power
GND → Servo GND           GND → Servo GND
(同样需要外部 5V 电源)      (同样需要外部 5V 电源)
```

### 按键连接（重要！）
```
Arduino:                  ESP32:
PIN 2 ─┬─ GND            GPIO 35 ─┬─ GND
       │ (through SW)            │ (through SW)
       └─ 5V (4.7kΩ)            └─ 3.3V* (4.7kΩ)

PIN 3 ─┬─ GND            GPIO 34 ─┬─ GND
       │ (through SW)            │ (through SW)
       └─ 5V (4.7kΩ)            └─ 3.3V* (4.7kΩ)

PIN 4 ─┬─ GND            GPIO 39 ─┬─ GND
       │ (through SW)            │ (through SW)
       └─ 5V (4.7kΩ)            └─ 3.3V* (4.7kΩ)

* 注意：GPIO 34/35/39 是 3.3V 输入专用引脚
```

**重要：** GPIO 34/35/39 是输入专用引脚，不能用于输出，也不支持 INPUT_PULLUP。

### I2C 连接（无需改动）
```
SCL (A5) → GPIO 22
SDA (A4) → GPIO 21
```

### 振动马达（可选改动）
```
Arduino:                  ESP32:
PIN 6 → 振动马达正极        GPIO 18 → 振动马达正极
GND → 振动马达负极         GND → 振动马达负极
```

---

## 代码改动详解

### 1. PIN 定义

**Arduino 版本：**
```cpp
const int SERVO_PIN = 8;
const int SWITCH_OPEN = 2;
const int SWITCH_CLOSE = 3;
const int SWITCH_HOLD = 4;
const int VIBRATION_PIN = 6;
```

**ESP32 版本：**
```cpp
const int SERVO_PIN = 5;          // GPIO 5
const int SWITCH_OPEN = 35;       // GPIO 35 (input-only)
const int SWITCH_CLOSE = 34;      // GPIO 34 (input-only)
const int SWITCH_HOLD = 39;       // GPIO 39 (input-only)
const int VIBRATION_PIN = 18;     // GPIO 18

// I2C Pins (GPIO 21 = SDA, GPIO 22 = SCL)
```

### 2. pinMode 改动

**Arduino 版本：**
```cpp
pinMode(SWITCH_OPEN, INPUT_PULLUP);
pinMode(SWITCH_CLOSE, INPUT_PULLUP);
pinMode(SWITCH_HOLD, INPUT_PULLUP);
```

**ESP32 版本：**
```cpp
pinMode(SWITCH_OPEN, INPUT);      // 不用 INPUT_PULLUP
pinMode(SWITCH_CLOSE, INPUT);     // GPIO 34/35/39 输入专用
pinMode(SWITCH_HOLD, INPUT);      // 需要外部上拉电阻
```

**原因：** GPIO 34/35/39 是输入专用引脚，不支持内部上拉，需要硬件上拉电阻。

### 3. I2C 初始化

**Arduino 版本：**
```cpp
Wire.begin();
```

**ESP32 版本：**
```cpp
Wire.begin(21, 22);  // SDA=GPIO 21, SCL=GPIO 22
```

### 4. 串口波特率

**Arduino 版本：**
```cpp
Serial.begin(9600);
```

**ESP32 版本：**
```cpp
Serial.begin(115200);  // ESP32 推荐 115200
```

### 5. EEPROM 改动

**Arduino 版本：**
```cpp
const int EEPROM_SIZE = 1024;  // 1KB
// 直接使用 EEPROM.get/put
```

**ESP32 版本：**
```cpp
const int EEPROM_SIZE = 512;   // 使用 512 字节
EEPROM.begin(EEPROM_SIZE);     // 初始化
// ... 使用 EEPROM.get/put ...
EEPROM.commit();               // 保存改动（重要！）
```

**关键点：** 每次修改后必须调用 `EEPROM.commit()`，否则不会保存。

### 6. 按键读取逻辑

由于 GPIO 34/35/39 不支持 INPUT_PULLUP，需要外部上拉电阻。读取逻辑相同：

```cpp
if (digitalRead(SWITCH_OPEN) == LOW) {  // 按下时为 LOW
  // 处理按键
}
```

---

## 迁移步骤

### 步骤 1：准备硬件

**需要购买/准备：**
- ESP32 Dev Board (30-50 元)
- USB Type-C 线（或 Micro USB 适配器）
- 4.7kΩ 电阻 3 个（用于按键上拉）

**硬件检查：**
```
□ 舵机： GPIO 5
□ 打开按键： GPIO 35 + 4.7kΩ 上拉到 3.3V
□ 关闭按键： GPIO 34 + 4.7kΩ 上拉到 3.3V
□ 保持按键： GPIO 39 + 4.7kΪ 上拉到 3.3V
□ INA219： GPIO 21 (SDA) + GPIO 22 (SCL)
□ 振动马达： GPIO 18
□ 共享 GND 和 5V 电源
```

### 步骤 2：安装 ESP32 支持

**Arduino IDE 中：**
```
1. 文件 → 首选项
2. 附加开发板管理器网址：
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
3. 工具 → 开发板 → 开发板管理器
4. 搜索 "ESP32"
5. 安装 "ESP32 by Espressif Systems"
6. 工具 → 开发板 → ESP32 Dev Module
```

### 步骤 3：上传代码

```
1. 打开 gripper_main_esp32.ino
2. 工具 → 开发板 → ESP32 Dev Module
3. 工具 → 端口 → COM X (根据实际)
4. 上传
```

**如果上传失败：**
```
1. 检查 USB 线是否是数据线（不是充电线）
2. 尝试按住 BOOT 按钮再上传
3. 检查串口是否被其他程序占用
4. 更新 USB 驱动程序
```

### 步骤 4：测试

```
1. 打开串口监视器 (115200 波特率)
2. 应该看到横幅消息
3. 执行命令测试：
   - open   （测试打开）
   - close  （测试关闭）
   - current（读取电流）
   - config （显示配置）
4. 按下各个按键，检查响应
5. 执行 log 查看日志
```

---

## 常见问题

### 问题 1：INA219 读不出值

**症状：** 串口显示 `✗ INA219 not found!`

**解决：**
```
1. 检查 GPIO 21 (SDA) 和 GPIO 22 (SCL) 的连接
2. 验证 INA219 有 3.3V 和 GND 连接
3. 运行 I2C 扫描程序检查地址
4. 检查上拉电阻（INA219 模块上应有 4.7kΩ）
```

**I2C 扫描程序：**
```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Serial.println("I2C 扫描中...");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("IIC 设备: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  
  if (nDevices == 0) Serial.println("未找到设备");
  delay(5000);
}
```

### 问题 2：按键无反应

**症状：** 按键不响应

**解决：**
```
1. 检查 4.7kΩ 上拉电阻是否连接正确
2. 验证 GPIO 34/35/39 是否接到 3.3V（通过 4.7kΩ）
3. 用万用表测量按键引脚电压：
   - 未按下： ~3.3V
   - 按下： ~0V
4. 检查按键是否损坏
```

### 问题 3：舵机不动

**症状：** 输入命令后舵机无反应

**解决：**
```
1. 检查 GPIO 5 是否正确连接到舵机信号线
2. 验证舵机电源是否接入 5V
3. 尝试上传 simple_pulse_test_esp32.ino 测试
4. 用示波器检查 GPIO 5 是否有 PWM 信号
5. 检查舵机是否被机械卡住
```

### 问题 4：配置无法保存

**症状：** 重启后设置丢失

**解决：**
```
检查代码中是否有 EEPROM.commit()：
✓ 正确：
  EEPROM.put(EEPROM_ADDR_OPEN, angleOpen);
  EEPROM.commit();  // 必须

✗ 错误：
  EEPROM.put(EEPROM_ADDR_OPEN, angleOpen);
  // 缺少 commit()
```

### 问题 5：串口乱码

**症状：** 串口输出显示乱码

**解决：**
```
1. 检查波特率是否设为 115200
2. Arduino IDE 中：工具 → 串口监视器 → 115200
3. 如果仍有乱码，尝试 921600
```

---

## 性能对比

| 指标 | Arduino | ESP32 |
|------|---------|-------|
| 舵机控制精度 | 相同 | 相同 |
| 响应速度 | 中等 | 快速 (240 MHz) |
| 存储空间 | 有限 | 充足 |
| WiFi/蓝牙 | 无 | ✓ 支持 |
| 可扩展性 | 低 | 高 |
| 功耗 | 低 | 中 |

---

## 下一步计划

### 短期（1-2 周）
```
✓ 完成硬件迁移
✓ 验证所有功能正常
✓ 更新文档
```

### 中期（1-3 个月）
```
□ 添加 WiFi 遥控功能
□ 实现远程日志上传
□ 开发配置 Web 界面
```

### 长期（3-6 个月）
```
□ 蓝牙控制 APP
□ 数据分析和可视化
□ 云端存储和监控
```

---

## 文件清单

使用 ESP32 时需要的文件：

```
✓ gripper_main_esp32.ino          （主程序）
✓ simple_pulse_test_esp32.ino     （舵机测试）
✓ fine_search_esp32.ino           （精细校准）
✓ 所有 Documentation/ 文件         （参考）
  └ PIN_MAPPING.md               （引脚对照）
```

---

## 寻求帮助

如果遇到问题，请检查：

1. **硬件：**
   - 所有连接是否正确？
   - 电压是否正确（3.3V vs 5V）？
   - 上拉电阻是否已连接？

2. **软件：**
   - 是否选择了 ESP32 Board？
   - 是否用了正确的代码版本？
   - 是否调用了 EEPROM.commit()？

3. **文档：**
   - 查看 PIN_MAPPING.md
   - 参考 DEBUGGING_GUIDE.md
   - 检查串口输出信息

---

## 总结

ESP32 迁移是一个简单的过程：

1. **硬件：** 改 PIN 定义，添加上拉电阻
2. **代码：** 99% 代码相同，只需修改 PIN 和初始化
3. **优势：** 更快的速度，更大的内存，可扩展性强
4. **未来：** 可以添加 WiFi、蓝牙等功能

开始迁移吧！ 🚀
