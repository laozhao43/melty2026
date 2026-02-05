# AM32 电调配置指南 (Arduino/ESP32 Passthrough)

本指南旨在帮助你通过 Arduino 或 ESP32 建立 **Serial Passthrough（串口转发）** 通道，从而利用桌面端配置软件对 AM32 电调进行参数调优与固件升级。

---

## 🛠️ 环境配置与安装
* **项目仓库**: [BrushlessPower/BlHeli-Passthrough](https://github.com/BrushlessPower/BlHeli-Passthrough/tree/main)
### 1. Arduino IDE 网络代理
由于部分 Arduino 开发板支持包（Core）及第三方库服务器位于海外，下载时若遇到网络超时，建议配置代理：

* **操作路径**：`文件 (File)` -> `首选项 (Preferences)` -> `网络 (Network)`。
* **配置方法**：勾选 **“手动设置代理”**，填入你本机的代理服务器地址（通常为 `127.0.0.1`）及对应端口（如 `7890`）。
### 2. 包安装
板子库和依赖库安装请参考上方链接
### 3. 电调引脚配置 (Pin Configuration)
在将代码烧录到开发板之前，必须根据你的硬件实际连线修改信号引脚定义：

* **修改文件**：打开项目中的 `Global.h`。
* **代码位置**：
    ```cpp
    // 修改下方数字以匹配你连接电调信号线（PWM/TLM）的引脚
    #define ESC_PIN 3 
    ```

---

## ⚙️ 推荐调试工具

> [!CAUTION]
> **兼容性警告**：
> 经实测，**在线配置工具 (Web Configurator)** 存在未知的通信协议兼容性问题。虽然有时能识别到电机，但往往无法成功执行“写入（Write）”或“保存（Save）”操作。

**强烈建议下载并使用官方桌面版工具：**
1.  **官方下载页面**：访问 [am32.ca/downloads](https://am32.ca/downloads)
2.  **定位工具**：点击页面上的 `Downloads` -> `Tools` 目录。
3.  **选择版本**：根据你的操作系统（Windows / Linux / MacOS）下载对应的 `.zip` 或 `.exe` 安装包。

---

## 📖 操作指南

### 第一步：固件烧录
1. 使用 Arduino IDE 打开本项目。
2. 确认 `Global.h` 中的 `ESC_PIN` 设置与你的物理接线一致。
3. 在 `工具` 菜单中选择你的开发板型号（例如：**ESP32-S3 Dev Module**）。
4. 点击 **上传 (Upload)** 按钮。

### 第二步：硬件连接
ESP32 与电调链接，与pin对应



### 第三步：参数读取与调试
1. 启动下载好的 **AM32 Configurator** 桌面程序。
2. 在端口列表中选择对应的 **串口号 (COM Port)**。
3. 点击 **Connect** 按钮。
4. 连接成功后，点击 **Read Setup** 读取电调内部参数。

> [!TIP]
> **小贴士**：
> * **仅读写参数**：通常无需接入动力电池，仅靠信号线的逻辑电平即可完成。
> * **固件更新/电机测试**：必须确保电调已接入动力电池。
> * **安全第一**：在进行任何电机转向测试前，**务必拆卸螺旋桨**。



