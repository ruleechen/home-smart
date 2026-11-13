# OPPLE-FE6300
Base on ESP-8266

## ESP-12F 接线

| LT8920 | ESP-12F GPIO | NodeMCU 标号 |
| --- | --- | --- |
| CLK / SCK | 14 | D5 |
| MOSI | 13 | D7 |
| MISO | 12 | D6 |
| CS | 4 | D2 |
| PKT | 5 | D1 |
| RST | 16 | D0 |
| VDD | 3.3V | 3V3 |
| GND | GND | GND |

与现有硬件规范一致。`main.cpp` 使用 GPIO 编号定义引脚。
RST 接 GPIO16，使用 SDK 的硬件复位流程。当前方案不采用 RST 直接接 3.3V，
也不将 `PIN_RF_RST` 设为 `0`：SDK 构造函数仍会把该编号对应的 GPIO 配置为输出。

PKT 沿用 SDK 默认的高有效配置，发送完成时等待 GPIO5 拉高。
当前实验使用轮询，不安装接收中断；后续接收可按上升沿处理。
引脚可通过 SDK 构造参数指定，接线适配放在应用代码中，不为此修改 SDK。

初始化后每隔 2 秒交替发送 `OppleCommands::LIGHT_ON` / `LIGHT_OFF`。
串口波特率为 115200，发送超时为 100 毫秒。

## 代码结构

- `lib/LT8920`：上游 SDK，保持原样。
- `lib/LT8920Radio`：应用侧封装，负责 SPI 初始化、芯片配置与回读校验、带超时和 PKT 极性的发送。
  `send()` 会自动补长度字节，调用方只传 payload。
- `src/OppleCommands.h`：抓包得到的遥控器指令 payload。
- `src/main.cpp`：引脚、射频参数和业务逻辑。

```sh
pio run -e opple-fe6300
```

射频速率暂用 1 Mbps；同步字暂按寄存器 36～39 的高/低字节顺序写入，
使用 64 位长度。这些通信参数仍需与原遥控器的寄存器抓包核对。
