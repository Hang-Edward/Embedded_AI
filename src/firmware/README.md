# Firmware

这里存放 STM32 / NUCLEO 固件源码。为了配合 C++ 大作业要求，当前固件源码也使用 C++ 编写。

## nucleo_pingpong

`nucleo_pingpong` 是当前已经烧录并验证过的 C++ 测试固件：

- 目标开发板：`NUCLEO-F446RE`
- 串口：`USART2 / ST-LINK VCP / 115200 8N1`
- 测试命令：`PING` -> `PONG`
- LED 命令：`LED:ON` / `LED:OFF`
- 状态命令：`STATUS?`
- 预留命令：`BUZZER:ON/OFF`、`VIB:ON/OFF`、`OLED:TEXT=...`

当前真实可控的是 `PA5 / D13`，也就是板载绿色 LED `LD2`。外接 LED 也可以通过 `D13` 加限流电阻接入。

当前蜂鸣器、震动、OLED 命令会先保存固件状态并返回确认；等外设接线完成后，再把这些状态映射到真实 GPIO / I2C 设备。

## Local Build Outputs

`build/` 目录是本地编译产物，已经由根目录 `.gitignore` 排除，不需要提交到 GitHub。
