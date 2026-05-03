# ESP32 WiFi 版本使用指南

## 功能介绍

v2.1 WiFi 版本添加了以下功能：

- 📡 **WiFi 连接**：可连接到家庭或办公室网络
- 🌐 **Web 仪表盘**：在浏览器中实时控制和监控
- 📊 **REST API**：用于集成到其他系统
- 📱 **远程控制**：从任何有网络的地方控制
- 📈 **实时状态**：角度、电流、运行时间等

---

## 硬件需求

完全相同，无额外硬件需要：
- ESP32 Dev Board
- Wheeltech HW020 舵机
- INA219 电流传感器
- 三档位开关
- 振动马达

---

## 配置 WiFi

### 步骤 1：修改 WiFi 凭据

打开 `gripper_main_esp32_wifi.ino`，修改第 24-25 行：

```cpp
const char* ssid = "YOUR_SSID";           // 修改为你的 WiFi 名称
const char* password = "YOUR_PASSWORD";   // 修改为你的 WiFi 密码
```

**例如：**
```cpp
const char* ssid = "MyHomeWiFi";
const char* password = "MyPassword123";
```

### 步骤 2：上传代码

```
1. 选择 Board: ESP32 Dev Module
2. 点击上传
```

### 步骤 3：验证连接

打开串口监视器（115200 波特率），应该看到：

```
📡 Connecting to WiFi...
SSID: MyHomeWiFi
........
✓ WiFi connected!
IP address: 192.168.1.100
Port: 80

✓ Web server started
```

---

## 使用 Web 仪表盘

### 访问仪表盘

在浏览器中输入：
```
http://192.168.1.100/
```

（将 IP 地址替换为实际的 ESP32 IP 地址）

### 仪表盘功能

```
📱 电动夹爪控制
├─ 📊 实时状态
│  ├─ 当前角度
│  └─ 电流值
├─ 🔘 控制按钮
│  ├─ 🔓 Open
│  ├─ 🔒 Close
│  ├─ ⏸ Stop
│  └─ 📳 Test Vibrate
├─ 📐 角度控制
│  └─ 滑块选择 0-270°，点击 Go
└─ 📝 最近活动
   └─ 显示最后 5 条操作记录
```

---

## REST API 端点

所有 API 调用都返回 JSON 格式的响应。

### 1. 获取状态 (GET)

```
http://192.168.1.100/api/status
```

**响应：**
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

**参数说明：**
- `angle`: 当前角度 (0-270°)
- `current`: 当前电流 (mA)
- `target`: 目标角度
- `uptime`: 运行时间 (秒)
- `logs_count`: 日志条数
- `wifi`: WiFi 连接状态

### 2. 获取日志 (GET)

```
http://192.168.1.100/api/logs
```

**响应：**
```json
[
  {
    "time": 123,
    "angle": 0,
    "current": 85.0,
    "action": "open"
  },
  {
    "time": 125,
    "angle": 270,
    "current": 410.0,
    "action": "spike_detected"
  }
]
```

### 3. 获取配置 (GET)

```
http://192.168.1.100/api/config
```

**响应：**
```json
{
  "open_angle": 0,
  "close_angle": 270,
  "speed": 5,
  "threshold": 300,
  "vibrate_time": 200
}
```

### 4. 更新配置 (POST)

```
http://192.168.1.100/api/config
?open=10&close=260&speed=6&threshold=250&vibrate=150
```

**参数：**
- `open`: 打开角度 (0-270)
- `close`: 关闭角度 (0-270)
- `speed`: 运动速度 (1-10)
- `threshold`: 电流阈值 (50-1000)
- `vibrate`: 振动时长 (50-1000)

### 5. 控制命令

#### 打开 (POST)
```
http://192.168.1.100/api/open
```

#### 关闭 (POST)
```
http://192.168.1.100/api/close
```

#### 停止 (POST)
```
http://192.168.1.100/api/stop
```

#### 转到指定角度 (POST)
```
http://192.168.1.100/api/go?angle=90
```

#### 测试振动 (POST)
```
http://192.168.1.100/api/vibrate
```

---

## 使用示例

### cURL 示例

**1. 获取当前状态：**
```bash
curl http://192.168.1.100/api/status
```

**2. 打开夹爪：**
```bash
curl -X POST http://192.168.1.100/api/open
```

**3. 转到 90°：**
```bash
curl -X POST "http://192.168.1.100/api/go?angle=90"
```

**4. 更新配置：**
```bash
curl -X POST "http://192.168.1.100/api/config?speed=7&threshold=250"
```

### Python 示例

```python
import requests

BASE_URL = "http://192.168.1.100"

# 获取状态
response = requests.get(f"{BASE_URL}/api/status")
print(response.json())

# 打开夹爪
requests.post(f"{BASE_URL}/api/open")

# 转到指定角度
requests.post(f"{BASE_URL}/api/go?angle=90")

# 获取日志
logs = requests.get(f"{BASE_URL}/api/logs").json()
for log in logs:
    print(f"{log['action']}: {log['angle']}° at {log['time']}s")
```

### JavaScript 示例

```javascript
const baseURL = "http://192.168.1.100";

// 获取状态
fetch(`${baseURL}/api/status`)
  .then(res => res.json())
  .then(data => console.log(data));

// 打开夹爪
fetch(`${baseURL}/api/open`, {method: 'POST'});

// 转到指定角度
fetch(`${baseURL}/api/go?angle=90`, {method: 'POST'});
```

---

## 常见问题

### Q1: 如何找到 ESP32 的 IP 地址？

**方法 1：串口监视器**
```
打开串口监视器（115200 波特率）
在启动时可以看到：
IP address: 192.168.1.100
```

**方法 2：查看路由器**
在路由器管理界面中查找连接的设备

**方法 3：IP 扫描工具**
使用工具如 Advanced IP Scanner 扫描网络

### Q2: 连接不到 WiFi 怎么办？

**检查清单：**
```
□ WiFi 名称（SSID）是否正确？
□ 密码是否正确？
□ WiFi 是否开启 2.4GHz（ESP32 不支持 5GHz）？
□ ESP32 距离路由器是否太远？
□ 修改后是否重新上传代码？
```

**调试方法：**
查看串口输出是否显示 WiFi 连接失败，修改代码中的 SSID 和密码。

### Q3: 无法访问 Web 仪表盘？

**检查清单：**
```
□ ESP32 WiFi 是否已连接？
□ IP 地址是否正确？
□ Web 服务器是否已启动（看是否有 "Web server started" 消息）？
□ 防火墙是否阻止了 80 端口？
```

**解决方法：**
```
1. 重启 ESP32
2. 检查串口输出
3. 尝试 ping ESP32：ping 192.168.1.100
4. 尝试访问：http://IP 地址/
```

### Q4: API 返回 404 Not Found？

**可能的原因：**
```
❌ 拼写错误：/api/staus → /api/status
❌ HTTP 方法错误：GET 用于查询，POST 用于控制
❌ 参数格式错误：缺少 ?
❌ URL 编码错误：特殊字符需要编码
```

**例如：**
```
错误：http://IP/api/config?open=10 close=270
正确：http://IP/api/config?open=10&close=270
```

---

## 性能和限制

### 性能指标

| 指标 | 值 |
|------|-----|
| 响应时间 | <100ms |
| 最大并发连接 | 4 个 |
| Web 传输速度 | ~1Mbps |
| API 吞吐量 | 10 req/s |

### 限制

| 限制 | 说明 |
|------|------|
| 网络延迟 | 本地网络通常 <50ms |
| 日志容量 | 最多 100 条记录 |
| WiFi 范围 | ~50m（取决于环境） |
| 内存 | 不影响核心功能 |

---

## 安全建议

⚠️ **这个版本没有身份验证。仅用于本地网络！**

### 安全使用建议

1. **仅在私有网络中使用**
   - 不要暴露到公网
   - 确保家中 WiFi 有密码

2. **限制访问**
   - 注意谁能访问你的网络
   - 不在不信任的网络中使用

3. **未来改进（计划）**
   - 添加 API 密钥认证
   - HTTPS 支持
   - 访问控制列表

---

## 故障排除日志

### 启动日志示例

```
✓ INA219 initialized
📡 Connecting to WiFi...
SSID: MyWiFi
..........
✓ WiFi connected!
IP address: 192.168.1.100
Port: 80
✓ Web server started

==========================================
  Electric Gripper Control System v2.1
             ESP32 WiFi Version
==========================================
✓ Servo: GPIO 5 (Wheeltech HW020 270°)
✓ INA219: I2C (GPIO 21/22)
✓ Switch: GPIO 35/34/39 (3-position)
✓ Vibration: GPIO 18
✓ Power: External 5V 3A
✓ Board: ESP32 Dev Module
✓ WiFi: Connected ✓
✓ Web: http://192.168.1.100/
==========================================
```

### 连接失败日志

```
📡 Connecting to WiFi...
SSID: MyWiFi
................
✗ WiFi connection failed!
Continuing in local mode...
```

**解决方案：**
- 检查 SSID 拼写
- 检查密码
- 重启路由器和 ESP32
- 查看路由器是否开启 2.4GHz

---

## 与 Arduino 版本的区别

| 功能 | Arduino v2.0 | ESP32 WiFi v2.1 |
|------|--------------|-----------------|
| 基本控制 | ✓ | ✓ |
| 电流监测 | ✓ | ✓ |
| 参数存储 | ✓ | ✓ |
| WiFi | ✗ | ✓ |
| Web 仪表盘 | ✗ | ✓ |
| REST API | ✗ | ✓ |
| 远程控制 | ✗ | ✓ |

---

## 未来计划

- [ ] 蓝牙支持
- [ ] MQTT 集成
- [ ] 云端存储
- [ ] 移动 APP
- [ ] 数据可视化
- [ ] 定时任务
- [ ] 多设备控制

---

## 相关文档

- 基础 API：参考 REST API 部分
- 硬件配置：参考 PIN_MAPPING.md
- 故障排除：参考 调试指南.md
- 参数说明：参考 参数配置指南.md

---

## 支持和反馈

如果遇到问题：
1. 检查串口输出信息
2. 阅读"常见问题"部分
3. 查看故障排除日志
4. 验证 WiFi 连接和 IP 地址

祝你使用愉快！🚀
