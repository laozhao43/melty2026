# 使用 NSSM 将 Mosquitto 注册为 Windows 服务

### 1. 环境准备与服务初始化
首先确保已参考 [Mosquitto Windows 安装教程](https://cedalo.com/blog/how-to-install-mosquitto-mqtt-broker-on-windows/) 完成程序安装。下载并解压 [NSSM](https://nssm.cc/download) 工具
### 2. 配置文件管理
mosquitto.conf
passwd
使用以上两个配置文件，下载到某一个目录中，并配置broker启动
nssm set mosquitto AppParameters -c "C:\Users\myuser\cedalo\mosquitto.conf"
