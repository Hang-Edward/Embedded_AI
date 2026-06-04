# Firmware

这里放置 STM32 / NUCLEO 固件源码。

## nucleo_pingpong

`nucleo_pingpong` 是当前已经烧录并验证过的最小测试固件：

- 目标开发板：NUCLEO-F446RE
- 串口：USART2 / ST-LINK VCP / `115200 8N1`
- 测试命令：`PING` -> `PONG`
- LED 命令：`LEDON` / `LEDOFF`

`build/` 目录是本地编译产物，已经由根目录 `.gitignore` 排除。
