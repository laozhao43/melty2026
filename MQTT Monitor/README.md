这份 README 文档将指导你如何部署和配置你提供的 **车辆远程监控系统 - 3D DShot 旗舰版**。

---

# 车辆远程监控系统 (TANK MONITOR) - 3D DShot 旗舰版

这是一个基于 Web 技术的实时遥测仪表盘，专门设计用于监控搭载 AM32 电调（或其他 DShot 协议驱动）的坦克或轮式车辆。它通过 MQTT 协议接收来自 ESP32 的传感器数据和 Python 发出的手柄控制反馈。

## 🌟 核心功能

* **双电机独立监控**：实时展示左右电机的 RPM、电流及驱动器温度。
* **系统状态看板**：可视化显示电池电压、系统解锁状态（ARM/DISARM）及牵引力控制（TCS）参数。
* **手柄状态镜像**：直观同步 Xbox 手柄的摇杆位移、按键触发及 RT 扳机行程。
* **全响应式设计**：适配 PC 与移动端浏览器。

---

## 🛠️ 关键配置指南

为了使网页能够成功连接到你的 MQTT Broker，你需要配置 **Broker 的 IP 地址**。

### 1. 修改 Broker 访问地址

打开 `index3.html` 文件，定位到底部的 `<script>` 部分：

```javascript
// 修改为你的 MQTT Broker 所在的实际局域网 IP
const BROKER_IP = '192.168.1.101'; 

// 默认连接配置（WebSocket 端口 9001）
const client = mqtt.connect(`ws://${BROKER_IP}:9001`, {
    username: 'myuser',
    password: '123',
    reconnectPeriod: 1000
});

```

### 2. 重要参数说明

* **端口说明**：Web 端使用的是 **WebSocket (WS)** 协议。请确保你的 Mosquitto 配置文件 (`mosquitto.conf`) 中开启了 `9001` 端口的 WebSocket 支持。
* **账号匹配**：确保 `username` 和 `password` 与你之前创建的 `passwd` 文件中的用户信息一致。

---

## 📖 部署与使用

### 第一步：启动 MQTT Broker

确保你的 Mosquitto 服务已在后台运行（通过 NSSM 或 CMD 启动），并且配置文件包含以下内容以支持网页连接：

```conf
listener 1883
protocol mqtt

listener 9001
protocol websockets

```

### 第二步：运行遥测网页

由于使用了 WebSocket，你只需在浏览器中打开 `index3.html` 即可：

1. 确认你的电脑/手机与 Broker 处于同一局域网。
2. 观察顶部状态栏，若显示为 **ONLINE (绿色)** 则表示连接成功。
<img width="1866" height="1428" alt="image" src="https://github.com/user-attachments/assets/155c5198-9490-48ab-a73f-54ce3f30ccac" />

### 第三步：数据对接

系统会自动订阅以下两个主题：

* `sensors/esp32`：接收来自车辆端的 JSON 遥测数据（电压、RPM、温度等）。
* `controller/xbox`：接收来自 `mqtthello.py` 转发的手柄模拟量数据。

---

## ⚠️ 调试小贴士

* **控制台排查**：如果一直显示 `DISCONNECTED`，请在浏览器按 `F12` 打开控制台（Console），查看是否有 WebSocket 连接错误。
* **刷新频率**：UI 中的摇杆和 RPM 条采用了 CSS 过渡优化，建议数据发送端的频率控制在 **20Hz - 50Hz** 以获得最佳视觉平滑度。

---

**是否需要我协助你配置 Mosquitto 的 `mosquitto.conf` 文件以确保其完美支持 WebSocket 连接？**
