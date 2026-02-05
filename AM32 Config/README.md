# AM32 电调配置简明指南

### 1. Arduino 环境配置与固件烧写
首先，通过 [BlHeli-Passthrough ](https://github.com/BrushlessPower/BlHeli-Passthrough/tree/main) 获取passthrough固件源码。在 Arduino IDE 中，请先于 `文件 -> 首选项` 中配置好网络代理以确保 ESP32 支持包顺利下载。根据教程下载相关libraries和板子库。烧录前，务必打开项目中的 `Global.h` 文件，将 `#define ESC_PIN` 后的数字修改为你实际连接电调信号线的引脚编号，随后选择对应的板型（如 ESP32-S3）完成上传。

### 2. AM32 Configuration Tool 工具下载
为了保证配置过程的稳定性，请避开兼容性较差的在线版工具，直接前往 [AM32 官方下载页面](https://am32.ca/downloads) 获取桌面客户端。进入页面后点击 `Downloads -> Tools` 目录，根据你的操作系统选择对应的安装包。使用桌面版工具可以有效避免在线版常见的识别成功但无法保存或写入配置（Write Flash）的问题。

### 3. AM32 Configuration Tool 调参使用
工具准备就绪后，可参考 [AM32 详细调参教程](https://combatrobotics.co.nz/pages/programming-am32-escs) 进行操作。启动桌面程序并选择正确的串口号点击 `Connect`，成功后即可通过 `Read Setup` 读取电调参数。请注意，仅读写参数通常无需动力电池，但若涉及固件更新或电机转向测试，则**必须接入动力电池**，且**务必提前固定好电机**。
### 4. Motor beep
以下是为你整理的 **AM32 常用鸣响/提示音信号 (Common AM32 Beep/Sound Signals)** 详尽说明，包含中英双语对照。你可以将其作为项目 README 的补充章节或独立参考手册。

---

# AM32 常用鸣响信号指南 (AM32 Beep Signals Guide)

在使用 AM32 电调进行调试或配置时，电调发出的鸣响声是判断其当前硬件状态和连接情况的重要依据。

### 1. 启动阶段 (Startup Phase)

* **初始上电 (Power-Up Initial)**
* **现象**：电调接收到电源后，会播放一系列由低到高的升调鸣响。
* **Sign**: A series of ascending beeps plays when the ESC receives power.


* **电池/电芯计数 (Battery/Cell Count)**
* **现象**：紧接着初始鸣响后，会有数次鸣响，次数对应电调检测到的电池电芯（S数）。
* **Sign**: Several beeps indicating the detected number of cells.


* **信号检测与解锁 (Signal Detection/Arming)**
* **现象**：一段最终的“解锁”音（通常是升高的音调），表示电调已成功识别到遥控器或飞行控制器的信号。
* **Sign**: A final "arming" beep (often a rising pitch) signifies it has found the radio/FC signal.



### 2. 状态告警与调试 (Alerts & Debugging)

* **无信号告警 (No Signal)**
* **现象**：持续的、缓慢的间歇性鸣响，表示电调已通电但未接收到来自接收机或飞控的任何有效信号。
* **Sign**: Continuous, slow beeping indicates the ESC is powered but not receiving a signal from the radio/flight controller.


* **固件刷新成功 (Firmware Flash Success)**
* **现象**：固件刷写完成后，会发出独特的、急促的三连音提示。
* **Sign**: A distinct triple-tone indicates that the AM32 firmware has been successfully flashed.


* **连接/读取过程 (Unlock Process)**
* **现象**：当配置工具尝试搜索 MCU 时，会发出急促的低频鸣响；如果连接成功，音调会随即升高。
* **Sign**: A rapid, low-pitched beep occurs when the tool is searching for the MCU, followed by a rising pitch if successful.

* **低电压 (Low voltage)**
* **现象**：可能是警笛声？
* **Sign**: Siren sound?

