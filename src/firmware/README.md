# Firmware

这里放置 STM32 / NUCLEO 固件源码。为了匹配 C++ 大作业要求，固件源码也使用 C++ 编写。

## nucleo_pingpong

`nucleo_pingpong` 是当前已经烧录并验证过的 C++ 测试固件：

- 目标开发板：NUCLEO-F446RE
- 串口：USART2 / ST-LINK VCP / `115200 8N1`
- 测试命令：`PING` -> `PONG`
- LED 命令：`LED:ON` / `LED:OFF`
- 状态命令：`STATUS?`
- 预留命令：`BUZZER:ON/OFF`、`VIB:ON/OFF`、`OLED:TEXT=...`

`build/` 目录是本地编译产物，已经由根目录 `.gitignore` 排除。

当前蜂鸣器、震动、OLED 命令会先保存固件状态并返回确认；等外设接线完成后，再把这些状态映射到真实 GPIO / I2C 设备。
