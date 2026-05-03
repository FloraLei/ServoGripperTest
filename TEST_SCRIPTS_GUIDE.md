# Test Scripts Guide

## 📋 可用的测试脚本

### 1. **test_reciprocal_motion.ino** - 往复运动测试
自动让舵机在 0° 和 180° 之间不断往复运动，用于检验完整运动范围。

**特点：**
- ✓ 自动循环运动
- ✓ 实时输出角度
- ✓ 无需用户交互

**使用方法：**
```
1. 打开 test_reciprocal_motion.ino
2. 上传到 Arduino
3. 打开串口监视器（9600 baud）
4. 观看舵机往复运动和角度输出
```

**预期输出：**
```
=== Servo Reciprocal Motion Test ===
Attempting to attach servo...
✓ Servo attached successfully!

Starting reciprocal motion (0° ↔ 180°)...

>>> Moving 0° → 180° (Close)
  0°
  10°
  20°
  ...
  180°
✓ Reached 180°

<<< Moving 180° → 0° (Open)
  180°
  170°
  160°
  ...
  0°
✓ Reached 0°

>>> Moving 0° → 180° (Close)
...（循环继续）
```

**调整速度：**
修改第 13 行的 `STEP_DELAY` 值：
```cpp
const int STEP_DELAY = 30;  // 毫秒（更小 = 更快）
```
- `10` - 很快
- `30` - 中等（默认）
- `50` - 较慢
- `100` - 非常慢

---

### 2. **servo_interactive_control.ino** - 交互式控制
通过串口输入来精确控制舵机运动到指定角度，方便测试不同位置。

**特点：**
- ✓ 实时控制舵机
- ✓ 可调节运动速度
- ✓ 显示当前角度
- ✓ 交互式命令

**使用方法：**

1. **打开并上传脚本**
```
打开 servo_interactive_control.ino
上传到 Arduino
打开串口监视器（9600 baud）
```

2. **发送命令（在串口监视器中输入）**

#### 基础角度命令：
```
0      → 舵机移动到 0°
45     → 舵机移动到 45°
90     → 舵机移动到 90°
180    → 舵机移动到 180°
```

#### 设置移动速度：
```
speed 5    → 非常慢（5°/步）
speed 10   → 较慢（10°/步）
speed 20   → 中等（20°/步）
speed 45   → 较快（45°/步）
```

#### 查看当前位置：
```
pos        → 显示当前角度和目标角度
```

#### 显示帮助：
```
h          → 显示命令列表
?          → 显示命令列表
help       → 显示命令列表
```

**交互示例：**
```
输入: h
输出:
========== COMMANDS ==========
Angle Control:
  0-180        Move to angle (e.g., 'go 45')
  Examples: 0, 45, 90, 180

Settings:
  speed N      Set movement speed (1-180)
               (lower = smoother, higher = faster)
  pos          Show current position

Help:
  h, ?, help   Show this help message
==============================

Current settings - Speed: 10°/step, Current angle: 90°
Enter angle or command: 

输入: 0
输出:
Moving to 0°...
  0°
✓ Reached 0°
Enter angle or command: 

输入: speed 5
输出:
✓ Speed set to 5°/step (36ms delay)
Enter angle or command: 

输入: 180
输出:
Moving to 180°...
  0°
  5°
  10°
  ...
  180°
✓ Reached 180°
```

---

## 🎯 测试流程建议

### 第一步：验证舵机硬件
1. 上传 `test_reciprocal_motion.ino`
2. 观看舵机是否能从 0° 运动到 180° 往复
3. 检查串口输出是否连续

**可能的问题：**
- 舵机不动 → 检查电源和接线
- 舵机抖动 → 检查 Arduino 是否在复位，可能需要独立电源
- 只能动一部分 → 可能需要校准角度范围

### 第二步：找到合适的角度范围
1. 上传 `servo_interactive_control.ino`
2. 测试不同角度，找到舵机全开和全合的位置
3. 记录下这两个角度（例如 15° 和 165°）

```
输入: 0     → 观察舵机位置（应该是最开）
输入: 180   → 观察舵机位置（应该是最合）
输入: 45    → 观察舵机位置（中间位置）
```

### 第三步：调整运动速度
1. 用 `speed` 命令测试不同的速度
2. 找到最适合的速度（不要太快以避免抖动）

```
输入: speed 5     → 很慢，平稳
输入: speed 20    → 中等速度
输入: speed 45    → 较快
```

### 第四步：更新配置
将测试结果更新到 `gripper_control.ino`：
```cpp
#define SERVO_MIN_ANGLE 15    // 你测试出的全开角度
#define SERVO_MAX_ANGLE 165   // 你测试出的全合角度
#define SERVO_SPEED 10        // 你测试出的合适速度
```

---

## 🔧 脚本对比

| 特性 | 往复运动 | 交互控制 |
|------|--------|--------|
| 自动运动 | ✓ | ✗ |
| 手动控制 | ✗ | ✓ |
| 调整速度 | 需修改代码 | 命令实时调整 |
| 测试单一角度 | 困难 | 容易 |
| 完整范围测试 | 容易 | 需多次输入 |
| 用途 | 验证硬件 | 精确校准 |

---

## 💡 使用技巧

### 保存测试结果
在串口监视器中，记录以下信息：
```
测试日期: 2026-05-03
舵机型号: [你的舵机型号]
最低角度: 15° (全开)
最高角度: 165° (全合)
最佳速度: 10°/步
功耗表现: [USB供电/独立电源]
```

### 快速测试关键位置
```
输入: 0
输入: pos
输入: 90
输入: pos
输入: 180
输入: pos
```

### 检查舵机供电
如果看到以下现象，需要用独立电源：
- 舵机运动时 Arduino 重启
- 舵机抖动或响应迟缓
- 串口输出断断续续

尝试：
```
输入: speed 5      # 降低速度，减少电流
```

---

## 📝 故障排除

| 问题 | 原因 | 解决 |
|------|------|------|
| 舵机不响应任何命令 | 串口未连接 | 检查 COM 端口和波特率 |
| 输入角度后无反应 | Arduino 掉电 | 用独立电源供舵机 |
| 舵机运动不平滑 | 速度设置太高 | `speed 5` 或 `speed 10` |
| 无法达到某些角度 | 超出舵机范围 | 大多数舵机是 0-180° |
| 角度显示但舵机不动 | 舵机断电 | 检查舵机电源 |

