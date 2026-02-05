# AM32 电调配置简明指南

### 1. Arduino 环境配置与固件烧写
首先，通过 [BlHeli-Passthrough 项目仓库](https://github.com/BrushlessPower/BlHeli-Passthrough/tree/main) 获取转发固件源码。在 Arduino IDE 中，请先于 `文件 -> 首选项` 中配置好网络代理以确保 ESP32 支持包顺利下载。烧录前，务必打开项目中的 `Global.h` 文件，将 `#define ESC_PIN` 后的数字修改为你实际连接电调信号线的引脚编号，随后选择对应的板型（如 ESP32-S3）完成上传。

### 2. AM32 Configuration Tool 工具下载
为了保证配置过程的稳定性，请避开兼容性较差的在线版工具，直接前往 [AM32 官方下载页面](https://am32.ca/downloads) 获取桌面客户端。进入页面后点击 `Downloads -> Tools` 目录，根据你的操作系统选择对应的安装包。使用桌面版工具可以有效避免在线版常见的识别成功但无法保存或写入配置（Write Flash）的问题。

### 3. AM32 Configuration Tool 调参使用
工具准备就绪后，可参考 [AM32 详细调参教程](https://combatrobotics.co.nz/pages/programming-am32-escs) 进行操作。启动桌面程序并选择正确的串口号点击 `Connect`，成功后即可通过 `Read Setup` 读取电调参数。请注意，仅读写参数通常无需动力电池，但若涉及固件更新或电机转向测试，则**必须接入动力电池**，且**务必提前固定好电机**。
