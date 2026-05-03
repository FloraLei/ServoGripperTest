# 电动夹爪项目 - 文件夹结构说明

## 整体结构

```
ElectricGripperTest/
│
├── Documentation/              # 📚 所有文档
│   ├── HARDWARE_ARCHITECTURE.md          # 英文硬件架构
│   ├── 硬件架构与接线指南.md              # 中文硬件架构
│   ├── DEBUGGING_GUIDE.md                # 英文调试指南
│   ├── 调试指南.md                       # 中文调试指南
│   ├── 参数配置指南.md                    # 参数配置指南
│   ├── ESP32_MIGRATION_GUIDE.md          # ESP32 迁移指南
│   └── PIN_MAPPING.md                    # PIN 管脚对照表
│
├── Arduino/                    # 🔵 Arduino 版本（当前运行）
│   ├── gripper_main.ino                  # 主程序 v2.0
│   ├── simple_pulse_test.ino             # 简单脉冲测试
│   ├── fine_search.ino                   # 精细范围搜索
│   └── README_ARDUINO.md                 # Arduino 版本说明
│
├── ESP32/                      # 🟠 ESP32 版本（新开发）
│   ├── gripper_main_esp32.ino            # 主程序（ESP32 版）
│   ├── simple_pulse_test_esp32.ino       # 简单脉冲测试（ESP32）
│   ├── fine_search_esp32.ino             # 精细范围搜索（ESP32）
│   └── README_ESP32.md                   # ESP32 版本说明
│
└── Archive/                    # 📦 历史文件和测试
    ├── 各种测试目录
    └── 参考文件
```

---

## 文件说明

### 📚 Documentation（文档）
| 文件 | 说明 | 语言 |
|------|------|------|
| HARDWARE_ARCHITECTURE.md | 完整硬件架构设计 | 英文 |
| 硬件架构与接线指南.md | 硬件架构中文版 | 中文 |
| DEBUGGING_GUIDE.md | 调试和故障排除 | 英文 |
| 调试指南.md | 调试指南中文版 | 中文 |
| 参数配置指南.md | 5 个参数详解 | 中文 |
| ESP32_MIGRATION_GUIDE.md | Arduino → ESP32 迁移 | 中文 |
| PIN_MAPPING.md | Arduino vs ESP32 PIN 对比 | 中文/英文 |

### 🔵 Arduino 版本
| 文件 | 用途 |
|------|------|
| gripper_main.ino | 完整功能版（推荐使用） |
| simple_pulse_test.ino | 基础舵机测试 |
| fine_search.ino | 精细舵机校准 |
| README_ARDUINO.md | Arduino 特定说明 |

### 🟠 ESP32 版本
| 文件 | 用途 |
|------|------|
| gripper_main_esp32.ino | 完整功能版（ESP32） |
| simple_pulse_test_esp32.ino | 基础舵机测试（ESP32） |
| fine_search_esp32.ino | 精细舵机校准（ESP32） |
| README_ESP32.md | ESP32 特定说明 |

---

## 快速开始

### 当前使用 Arduino
```
1. 打开 Arduino/gripper_main.ino
2. 选择 Board: Arduino Uno
3. 上传
4. 参考 Documentation 中的调试指南
```

### 迁移到 ESP32
```
1. 阅读 Documentation/ESP32_MIGRATION_GUIDE.md
2. 打开 ESP32/gripper_main_esp32.ino
3. 选择 Board: ESP32 Dev Module
4. 根据 PIN_MAPPING 调整硬件连接
5. 上传并测试
```

---

## 关键的硬件差异

| 功能 | Arduino | ESP32 |
|------|---------|-------|
| 舵机控制 | PIN 8 | GPIO 5 |
| 打开按键 | PIN 2 | GPIO 35 |
| 关闭按键 | PIN 3 | GPIO 34 |
| 保持按键 | PIN 4 | GPIO 39 |
| 振动反馈 | PIN 6 | GPIO 18 |
| I2C SCL | A5 (PIN 19) | GPIO 22 |
| I2C SDA | A4 (PIN 18) | GPIO 21 |
| 存储 | EEPROM (1KB) | SPIFFS / EEPROM (4KB) |

详见 `Documentation/PIN_MAPPING.md`

---

## 文件清理指南

如果你想清理旧的测试文件和目录：

```
保留：
  ✓ Arduino/ 目录及所有文件
  ✓ ESP32/ 目录及所有文件
  ✓ Documentation/ 目录及所有文件
  ✓ 这个 README_Organization.md

可以归档到 Archive/:
  ├── find_real_range/
  ├── gripper_main/
  ├── servo_calibrate_find_range/
  ├── servo_control_simple/
  ├── servo_pulse_tester/
  ├── servo_reciprocal_test/
  ├── WHEELTEC_*.zip
  ├── 舵机控制代码.zip
  ├── *.cpp / *.h 文件
  └── ARCHITECTURE.md 等旧版本
```

---

## 版本历史

| 版本 | 平台 | 状态 | 说明 |
|------|------|------|------|
| v1.0 | Arduino | ✓ 完成 | 基础功能测试版 |
| v2.0 | Arduino | ✓ 完成 | 完整功能版（当前） |
| v2.0 ESP32 | ESP32 | ✓ 完成 | ESP32 完整功能版 |

---

## 下一步

1. ✓ Arduino 版本稳定运行
2. ✓ 创建 ESP32 版本
3. 🔄 硬件迁移到 ESP32
4. 📝 添加 WebSocket/网络功能（可选）
5. 📱 开发配置 App（可选）
