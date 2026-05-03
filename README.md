# ⚙️ Electric Gripper Control System

![Version](https://img.shields.io/badge/version-2.1-green.svg)
![Arduino](https://img.shields.io/badge/Arduino-Uno-blue.svg)
![ESP32](https://img.shields.io/badge/ESP32-Dev%20Module-orange.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

一个完整的电动夹爪控制系统，支持 Arduino Uno 和 ESP32 平台，具有 WiFi、Web 仪表盘和 REST API。

## 🎯 主要特性

### 核心功能
- 🎮 **舵机控制** - Wheeltech HW020 270° 舵机，平滑运动，可调速度
- 📊 **电流监测** - INA219 传感器实时监测，自动峰值检测
- 🔘 **三档位开关** - 打开/关闭/保持位置，机械开关集成
- 📳 **振动反馈** - 电流峰值自动触发，可调时长
- 💾 **参数存储** - EEPROM 持久化配置，5 个可调参数

### 平台支持
- ✅ **Arduino Uno** - 成熟稳定，代码简洁
- ✅ **ESP32** - 速度快，功能完整
- ✅ **ESP32 WiFi** - 内置 Web 界面和 REST API（推荐）

### WiFi & Web (v2.1)
- 🌐 **Web 仪表盘** - 现代化响应式界面，实时控制
- 📱 **移动支持** - PC、平板、手机完全适配
- 📊 **REST API** - 9 个 HTTP 端点，完整 API 文档
- ⚡ **实时更新** - 1 秒刷新频率，低延迟控制

## 📁 项目结构

```
ElectricGripperTest/
├── 📜 代码文件
│   ├── gripper_main.ino                    # Arduino Uno 版本
│   ├── gripper_main_esp32.ino              # ESP32 标准版
│   ├── gripper_main_esp32_wifi.ino         # ESP32 WiFi 版本（推荐）
│   ├── simple_pulse_test.ino               # 舵机基础测试
│   ├── simple_pulse_test_esp32.ino         # ESP32 舵机测试
│   ├── fine_search.ino                     # 精细校准脚本
│   └── fine_search_esp32.ino               # ESP32 精细校准
│
├── 📚 完整文档（4000+ 行）
│   ├── README_COMPLETE.md                  # 综合项目指南
│   ├── HARDWARE_ARCHITECTURE.md            # 英文硬件设计
│   ├── 硬件架构与接线指南.md                # 中文硬件指南
│   ├── DEBUGGING_GUIDE.md                  # 英文调试指南
│   ├── 调试指南.md                         # 中文调试指南
│   ├── 参数配置指南.md                      # 参数详解（中文）
│   ├── ESP32_MIGRATION_GUIDE.md            # 迁移指南（中文）
│   ├── PIN_MAPPING.md                      # 引脚对照表
│   ├── README_ESP32_WIFI.md                # WiFi 版本说明
│   ├── README_ORGANIZATION.md              # 文件结构说明
│   ├── CLEANUP_GUIDE.md                    # 文件整理指南
│   └── COMPLETION_SUMMARY.md               # 项目完成总结
│
├── 🛠️ 工具和资源
│   ├── migrate_to_esp32.py                 # 自动化迁移脚本（Python）
│   ├── web_control.html                    # 独立 Web 控制界面（HTML5）
│   └── .gitignore                          # Git 配置
│
└── 📦 Archive/                             # 历史文件备份
```

## 🚀 快速开始

### 选择你的平台

#### 🔵 Arduino Uno（初学者推荐）

```bash
1. 打开 Arduino IDE
2. 打开 → gripper_main.ino
3. 工具 → 开发板 → Arduino Uno
4. 上传代码
5. 工具 → 串口监视器 (9600 波特率)
```

**特点：** 简单可靠，功能完整，代码易理解

#### 🟠 ESP32 WiFi 版本（功能最完整，推荐升级）

```bash
1. 安装 ESP32 驱动：
   文件 → 首选项 → 附加开发板管理器网址
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

2. 工具 → 开发板 → 开发板管理器 → 搜索 ESP32 → 安装

3. 编辑代码：
   打开 → gripper_main_esp32_wifi.ino
   找到第 24-25 行，修改 WiFi SSID 和密码：
   const char* ssid = "YOUR_SSID";
   const char* password = "YOUR_PASSWORD";

4. 工具 → 开发板 → ESP32 Dev Module
5. 上传代码
6. 工具 → 串口监视器 (115200 波特率)
7. 在浏览器打开 web_control.html
8. 输入 ESP32 的 IP 地址（在串口输出中可见）
```

**新增功能：** WiFi 远程控制 + Web 仪表盘 + REST API

### 从 Arduino 迁移到 ESP32

```bash
# 方式 1: 使用自动化脚本（推荐）
python migrate_to_esp32.py gripper_main.ino
# 输出：gripper_main_esp32.ino

# 方式 2: 直接使用预生成的版本
打开 → gripper_main_esp32.ino 或 gripper_main_esp32_wifi.ino
```

## 📊 版本对比

| 功能 | Arduino v2.0 | ESP32 v2.0 | ESP32 WiFi v2.1 |
|------|--------------|-----------|-----------------|
| 基础控制 | ✅ | ✅ | ✅ |
| 舵机运动 | ✅ | ✅ | ✅ |
| 电流监测 | ✅ | ✅ | ✅ |
| 参数存储 | ✅ | ✅ | ✅ |
| 三档位开关 | ✅ | ✅ | ✅ |
| 振动反馈 | ✅ | ✅ | ✅ |
| **WiFi 连接** | ❌ | ❌ | ✅ |
| **Web 仪表盘** | ❌ | ❌ | ✅ |
| **REST API** | ❌ | ❌ | ✅ |
| **远程控制** | ❌ | ❌ | ✅ |
| 代码行数 | 510 | 520 | 620 |

## 📖 文档导航

| 你想要做什么？ | 阅读这个文档 |
|---|---|
| 快速上手和完整参考 | [README_COMPLETE.md](README_COMPLETE.md) |
| 查看硬件设计和接线 | [HARDWARE_ARCHITECTURE.md](HARDWARE_ARCHITECTURE.md) |
| 中文硬件和接线指南 | [硬件架构与接线指南.md](硬件架构与接线指南.md) |
| 调试问题和故障排除 | [调试指南.md](调试指南.md) |
| 配置参数详解和应用 | [参数配置指南.md](参数配置指南.md) |
| 从 Arduino 迁移到 ESP32 | [ESP32_MIGRATION_GUIDE.md](ESP32_MIGRATION_GUIDE.md) |
| Arduino ↔ ESP32 引脚对照 | [PIN_MAPPING.md](PIN_MAPPING.md) |
| WiFi 和 Web 功能说明 | [README_ESP32_WIFI.md](README_ESP32_WIFI.md) |
| 项目完成总结 | [COMPLETION_SUMMARY.md](COMPLETION_SUMMARY.md) |

## 🔧 REST API 快速参考

### 基础端点

```bash
# 获取实时状态
curl http://192.168.1.100/api/status

# 打开夹爪
curl -X POST http://192.168.1.100/api/open

# 关闭夹爪
curl -X POST http://192.168.1.100/api/close

# 转到 90 度
curl -X POST http://192.168.1.100/api/go?angle=90

# 获取配置
curl http://192.168.1.100/api/config

# 更新参数
curl -X POST "http://192.168.1.100/api/config?speed=6&threshold=250"
```

### API 响应示例

```json
{
  "angle": 135,
  "current": 95.5,
  "target": 135,
  "uptime": 3600,
  "logs_count": 42,
  "wifi": true
}
```

完整的 API 文档见 [README_ESP32_WIFI.md](README_ESP32_WIFI.md)。

## ⚙️ 硬件配置速查

### Arduino Uno
```
PIN 8  → 舵机信号线（白）
PIN 2  → 打开开关
PIN 3  → 关闭开关
PIN 4  → 保持开关
PIN 6  → 振动马达
A4     → INA219 SDA（数据线）
A5     → INA219 SCL（时钟线）
GND    → 所有 GND（共用）
5V     → 舵机电源 + INA219（独立 5V 3A 电源）
```

### ESP32 Dev Module
```
GPIO 5  → 舵机信号线（白）
GPIO 35 → 打开开关 + 4.7kΩ 上拉电阻
GPIO 34 → 关闭开关 + 4.7kΩ 上拉电阻
GPIO 39 → 保持开关 + 4.7kΩ 上拉电阻
GPIO 18 → 振动马达
GPIO 21 → INA219 SDA（数据线）
GPIO 22 → INA219 SCL（时钟线）
GND    → 所有 GND（共用）
5V     → 舵机电源（独立 5V 3A 电源）
3.3V   → INA219 电源 + 按键上拉
```

详见 [PIN_MAPPING.md](PIN_MAPPING.md) 获取完整的对照表。

## 🛠️ 开发工具

### 自动化迁移脚本

```bash
python migrate_to_esp32.py gripper_main.ino
```

**功能：** 自动转换 Arduino 代码到 ESP32（280 行 Python）
- ✅ 自动替换 PIN 定义（8 个）
- ✅ 修改波特率（9600 → 115200）
- ✅ 配置 I2C 初始化
- ✅ 处理 INPUT_PULLUP 改动
- ✅ 添加 EEPROM.commit()

### Web 控制界面

```
打开 web_control.html 在任何浏览器中
```

**特性：**
- 📱 响应式设计（移动端完美适配）
- ⚡ 实时数据更新（每秒自动刷新）
- 🎨 现代化 UI（渐变色、平滑动画）
- 🔧 完整的控制面板
- ⚙️ 高级设置（可调参数）
- 📋 日志和状态显示

## 📊 项目规模

```
代码文件：         2,820 行
文档文件：        4,000+ 行
工具和脚本：        730 行
━━━━━━━━━━━━━━━━━━━━━━━━━
总计：            7,500+ 行

13 个文档
7 个代码文件
2 个工具文件
完整的项目体系
```

## 💡 常见问题

**Q: 应该用哪个版本？**
A: 初学者用 Arduino，想要WiFi功能用 ESP32 WiFi 版本。

**Q: 如何从 Arduino 迁移到 ESP32？**
A: 使用 `migrate_to_esp32.py` 脚本自动转换，或查看 [ESP32_MIGRATION_GUIDE.md](ESP32_MIGRATION_GUIDE.md)。

**Q: WiFi 无法连接怎么办？**
A: 检查 SSID 和密码，查看 [README_ESP32_WIFI.md](README_ESP32_WIFI.md) 的故障排除部分。

**Q: 舵机不动怎么办？**
A: 查看 [调试指南.md](调试指南.md) 的故障排除章节。

更多问题见各文档的 FAQ 部分。

## 📱 Web 仪表盘截图

Web 仪表盘提供：
- 🎮 直观的控制按钮（打开/关闭/停止/测试）
- 📊 实时角度和电流显示
- 📐 可视化角度选择滑块
- 📝 日志查看和操作历史
- ⚙️ 参数配置面板
- 🔄 自动数据刷新

## 🚀 下一步

### 即刻开始
1. 选择你的平台（Arduino 或 ESP32）
2. 按照快速开始说明操作
3. 查看相应的文档

### 功能增强
- [ ] 整理旧文件（见 [CLEANUP_GUIDE.md](CLEANUP_GUIDE.md)）
- [ ] 硬件迁移到 ESP32
- [ ] 配置 WiFi 功能
- [ ] 在 Web 界面控制

### 长期计划
- 蓝牙支持
- MQTT 集成
- 云端存储
- 移动 APP

## 📞 获取帮助

1. **阅读文档** - 99% 的问题都在文档中有答案
2. **查看示例** - 代码中有详细注释
3. **检查日志** - 串口输出提供诊断信息
4. **提交 Issue** - 如果是 Bug 请提交 Issue

## 📄 许可证

MIT License - 自由使用和修改

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## ✨ 关键特性总结

```
✅ 完整的硬件控制系统
✅ Arduino 和 ESP32 双平台
✅ WiFi 远程控制能力
✅ Web 仪表盘和 REST API
✅ 自动化迁移工具
✅ 完整文档（中英文，4000+ 行）
✅ 高质量代码（2800+ 行，有注释）
✅ 生产就绪
✅ 易于使用和扩展
✅ 活跃开发维护
```

---

## 🎯 快速导航

| 我想... | 去这里 |
|---|---|
| 快速开始使用 | [README_COMPLETE.md](README_COMPLETE.md) |
| 了解硬件细节 | [HARDWARE_ARCHITECTURE.md](HARDWARE_ARCHITECTURE.md) / [硬件架构与接线指南.md](硬件架构与接线指南.md) |
| 配置参数 | [参数配置指南.md](参数配置指南.md) |
| 解决问题 | [调试指南.md](调试指南.md) / [DEBUGGING_GUIDE.md](DEBUGGING_GUIDE.md) |
| 迁移到 ESP32 | [ESP32_MIGRATION_GUIDE.md](ESP32_MIGRATION_GUIDE.md) |
| 使用 WiFi 功能 | [README_ESP32_WIFI.md](README_ESP32_WIFI.md) |
| 对比版本 | [COMPLETION_SUMMARY.md](COMPLETION_SUMMARY.md) |

---

**🚀 准备好了吗？** → [点击这里开始](README_COMPLETE.md)

---

**项目版本：** v2.1 (WiFi Edition)  
**最后更新：** 2026-05-03  
**维护状态：** ✅ 活跃开发中  
**GitHub：** [FloraLei/ServoGripperTest](https://github.com/FloraLei/ServoGripperTest)
