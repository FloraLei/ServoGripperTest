# 文件清理和组织指南

## 当前项目结构

经过整理，项目现在有以下文件：

### ✅ 保留（核心文件）

#### 主程序代码
```
✓ gripper_main.ino                  - Arduino 版本（v2.0）
✓ gripper_main_esp32.ino            - ESP32 版本（v2.0）
✓ gripper_main_esp32_wifi.ino       - ESP32 WiFi 版本（v2.1，推荐）
✓ simple_pulse_test.ino             - Arduino 舵机测试
✓ simple_pulse_test_esp32.ino       - ESP32 舵机测试
✓ fine_search.ino                   - 精细校准测试
✓ fine_search_esp32.ino             - ESP32 精细校准测试
```

#### 文档
```
✓ README_COMPLETE.md                - 完整项目指南
✓ README_ORGANIZATION.md            - 文件结构说明
✓ HARDWARE_ARCHITECTURE.md          - 英文硬件架构
✓ 硬件架构与接线指南.md             - 中文硬件架构
✓ DEBUGGING_GUIDE.md                - 英文调试指南
✓ 调试指南.md                       - 中文调试指南
✓ 参数配置指南.md                   - 参数详解
✓ ESP32_MIGRATION_GUIDE.md          - ESP32 迁移指南
✓ PIN_MAPPING.md                    - PIN 对照表
✓ README_ESP32_WIFI.md              - WiFi 版本说明
```

#### 工具和辅助文件
```
✓ migrate_to_esp32.py               - 自动化迁移脚本
✓ web_control.html                  - 独立 Web 控制界面
```

### 🗑️ 可删除/归档（旧文件）

这些是之前调试过程中生成的临时文件，可以整理到 Archive 目录：

```
❌ ARCHITECTURE.md                   - 旧版本（已被替代）
❌ CONFIG_GUIDE.md                   - 旧版本（已被替代）
❌ README.md                         - 旧版本（已被替代）
❌ TEST_SCRIPTS_GUIDE.md             - 旧版本（已被替代）

❌ button_handler.cpp/h              - 未使用的模块
❌ gripper_state_machine.cpp/h       - 未使用的模块
❌ servo_controller.cpp/h            - 未使用的模块

❌ 整个目录和 zip 文件：
   - find_real_range/
   - gripper_main/
   - servo_calibrate_find_range/
   - servo_control_simple/
   - servo_pulse_tester/
   - servo_reciprocal_test/
   - WHEELTEC_*.zip
   - 舵机控制代码.zip
```

---

## 建议的清理步骤

### 步骤 1：创建 Archive 目录

```bash
mkdir Archive
```

### 步骤 2：移动旧文件到 Archive

```bash
# 移动旧 markdown 文件
mv ARCHITECTURE.md Archive/
mv CONFIG_GUIDE.md Archive/
mv README.md Archive/
mv TEST_SCRIPTS_GUIDE.md Archive/

# 移动旧模块文件
mv *.cpp Archive/
mv *.h Archive/

# 移动旧目录
mv find_real_range/ Archive/
mv gripper_main/ Archive/
mv servo_calibrate_find_range/ Archive/
mv servo_control_simple/ Archive/
mv servo_pulse_tester/ Archive/
mv servo_reciprocal_test/ Archive/
mv WHEELTEC_*/ Archive/

# 移动 zip 文件
mv *.zip Archive/
```

### 步骤 3：验证最终结构

执行清理后，根目录应该如下所示：

```
ElectricGripperTest/
├── 📚 主程序代码
│   ├── gripper_main.ino
│   ├── gripper_main_esp32.ino
│   ├── gripper_main_esp32_wifi.ino
│   ├── simple_pulse_test.ino
│   ├── simple_pulse_test_esp32.ino
│   ├── fine_search.ino
│   └── fine_search_esp32.ino
│
├── 📖 完整文档
│   ├── README_COMPLETE.md
│   ├── README_ORGANIZATION.md
│   ├── HARDWARE_ARCHITECTURE.md
│   ├── 硬件架构与接线指南.md
│   ├── DEBUGGING_GUIDE.md
│   ├── 调试指南.md
│   ├── 参数配置指南.md
│   ├── ESP32_MIGRATION_GUIDE.md
│   ├── PIN_MAPPING.md
│   └── README_ESP32_WIFI.md
│
├── 🛠️ 工具
│   ├── migrate_to_esp32.py
│   └── web_control.html
│
├── 📦 Archive（旧文件）
│   ├── 旧版本文档
│   ├── 测试脚本目录
│   ├── 旧的模块代码
│   └── 参考资料
│
└── CLEANUP_GUIDE.md（本文件）
```

---

## 各版本的选择指南

### 选择哪个版本使用？

#### 🔵 Arduino 版本（gripper_main.ino）
**适合：**
- 初学者学习基础功能
- 简单的本地使用
- 不需要网络功能

**优点：**
- 简单可靠
- 代码易理解
- 所有功能都有

**缺点：**
- 无网络功能
- 无 Web 界面

#### 🟠 ESP32 标准版（gripper_main_esp32.ino）
**适合：**
- 迁移到 ESP32
- 功能与 Arduino 版本相同
- 为添加网络功能做准备

**优点：**
- 与 Arduino 版本功能完全相同
- 可扩展性强
- 速度更快

**缺点：**
- 需要额外的上拉电阻（GPIO 34/35/39）
- 无网络功能（需要 WiFi 版本）

#### ✨ ESP32 WiFi 版本（gripper_main_esp32_wifi.ino）**推荐用于未来**
**适合：**
- 需要远程控制
- 需要 Web 仪表盘
- 需要 REST API
- 完整的功能集

**优点：**
- 完整的 Web 界面
- REST API 支持
- 远程控制能力
- 实时监控

**缺点：**
- 需要 WiFi 配置
- 代码更复杂
- 需要改密码

---

## 文件清理 PowerShell 脚本

如果使用 Windows，可以用这个脚本自动化清理：

```powershell
# Create Archive directory
if (!(Test-Path "Archive")) {
    New-Item -ItemType Directory -Name Archive
    Write-Host "Created Archive directory"
}

# Move old files
$oldFiles = @(
    "ARCHITECTURE.md",
    "CONFIG_GUIDE.md",
    "README.md",
    "TEST_SCRIPTS_GUIDE.md"
)

foreach ($file in $oldFiles) {
    if (Test-Path $file) {
        Move-Item -Path $file -Destination Archive/ -Force
        Write-Host "Moved: $file"
    }
}

# Move .cpp and .h files
Get-ChildItem -Filter "*.cpp" | Move-Item -Destination Archive/ -Force
Get-ChildItem -Filter "*.h" | Move-Item -Destination Archive/ -Force
Write-Host "Moved C++ files"

# Move old directories
$oldDirs = @(
    "find_real_range",
    "gripper_main",
    "servo_calibrate_find_range",
    "servo_control_simple",
    "servo_pulse_tester",
    "servo_reciprocal_test",
    "WHEELTEC_C06B*"
)

foreach ($dir in $oldDirs) {
    Get-ChildItem -Filter $dir -Directory | ForEach-Object {
        Move-Item -Path $_.FullName -Destination Archive/ -Force
        Write-Host "Moved: $($_.Name)"
    }
}

# Move zip files
Get-ChildItem -Filter "*.zip" | Move-Item -Destination Archive/ -Force
Write-Host "Moved zip files"

Write-Host "Cleanup complete!"
```

---

## 迁移计划

### 📅 建议的时间表

#### **现在 - 第 1 周**
```
✓ 整理文件结构
✓ 验证 Arduino 版本正常工作
✓ 阅读所有文档
✓ 理解 PIN 对应关系
```

#### **第 2-3 周**
```
□ 购买 ESP32 开发板
□ 安装驱动和库
□ 测试 migrate_to_esp32.py 脚本
□ 转换代码或手动改 PIN 定义
□ 硬件重新连接
```

#### **第 3-4 周**
```
□ 上传 ESP32 标准版
□ 验证所有功能正常
□ 调试问题
□ 测试各种配置
```

#### **第 4-5 周**
```
□ 升级到 ESP32 WiFi 版本
□ 配置 WiFi SSID 和密码
□ 测试 Web 仪表盘
□ 测试 REST API
□ 使用 web_control.html
```

---

## 备份和版本控制

### 推荐的备份策略

```
ElectricGripperTest/
├── main/（当前工作版本）
│   └── gripper_main_esp32_wifi.ino
├── archive/（旧版本备份）
│   ├── v1.0/
│   ├── v2.0/
│   └── experiments/
└── docs/（所有文档）
    └── ...
```

### 使用 Git 管理（可选）

```bash
# 初始化 git 仓库
git init

# 添加所有文件
git add .

# 首次提交
git commit -m "Initial commit: Electric Gripper Control System v2.1"

# 创建分支用于实验
git branch esp32-wifi-features
git checkout esp32-wifi-features
```

---

## 清理后的好处

✅ **项目更整洁**
- 少了混乱的临时文件
- 易于导航和理解

✅ **避免混淆**
- 明确哪个是当前版本
- 新用户不会被旧文件困扰

✅ **便于维护**
- Archive 中保留历史，需要时可查阅
- 主目录只有活跃的代码

✅ **提高效率**
- 更快找到所需文件
- 减少编译时的加载

---

## 恢复旧文件

如果需要查看或使用旧文件：

```
Archive/ 目录中都保留着，可以随时取出使用
只需从 Archive/ 复制回主目录即可
```

---

## 总结

| 步骤 | 操作 | 结果 |
|------|------|------|
| 1 | 新建 Archive 目录 | 准备好目标目录 |
| 2 | 移动旧文件 | 清理主目录 |
| 3 | 验证结构 | 确认完整性 |
| 4 | （可选）初始化 git | 版本控制 |

**现在你有一个整洁、有组织的项目结构！** 🎉

需要帮助吗？参考 README_COMPLETE.md 的"快速开始"部分。
