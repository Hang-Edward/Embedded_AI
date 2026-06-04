# PC C++ Application

这里放置 PC 端 C++ 主程序。当前版本是最小串口桥接测试，用来验证 Windows C++ 程序可以和 NUCLEO-F446RE 通信。

## Build

```powershell
cd "D:\VScode Projects\Embedded_AI\src\pc"
cmake -S . -B build
cmake --build build
```

## Run

默认连接 `COM11`：

```powershell
.\build\embedded_ai_pc_bridge.exe
```

如果 COM 口变化，可以手动指定：

```powershell
.\build\embedded_ai_pc_bridge.exe COM12
```

## Current Test Commands

- `PING` -> `PONG`
- `LEDON` -> `OK LED ON`
- `LEDOFF` -> `OK LED OFF`

后续计划职责：

- 调用 OpenCV 读取 Logitech C270 摄像头画面。
- 接收语音输入或按键触发事件。
- 调用千问视觉/多模态 API。
- 解析模型返回的结构化结果。
- 通过串口向 NUCLEO-F446RE 发送 LED、蜂鸣器、OLED、震动等反馈命令。
- 记录审计日志，并实现课程要求中的随机文件读写更新。
