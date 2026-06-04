# PC C++ Application

这里放置 PC 端 C++ 主程序。当前版本已经包含控制台 UI、硬件桥接类和二进制审计日志，用来验证 Windows C++ 程序可以和 NUCLEO-F446RE 通信。

## Build

```powershell
cd "D:\VScode Projects\Embedded_AI"
cmake -S . -B build
cmake --build build
```

## Run

默认连接 `COM11`：

```powershell
.\build\src\pc\embedded_ai_pc_bridge.exe
```

如果 COM 口变化，可以手动指定：

```powershell
.\build\src\pc\embedded_ai_pc_bridge.exe COM12
```

非交互验证模式：

```powershell
.\build\src\pc\embedded_ai_pc_bridge.exe COM11 --demo
```

## Console UI

当前菜单包含：

- 测试硬件连接。
- 打开 LED。
- 关闭 LED。
- 模拟 AI 场景识别结果。
- 查看历史审计日志。
- 按记录 ID 更新日志状态。

## OOP Structure

- `App`：控制台菜单和应用流程。
- `Console`：终端输入输出封装。
- `SerialPort`：Windows 串口读写封装。
- `HardwareBridge`：统一硬件协议封装。
- `AuditLogStore`：二进制审计日志，支持随机读取和随机更新。
- `SceneTask`：模拟 AI 场景任务。
- `DeviceState`：预留硬件状态对象。

## Random File Processing

`AuditLogStore` 使用固定长度二进制记录写入 `audit-log.dat`。更新记录状态时，程序会根据 `recordId` 计算偏移量，然后用 `seekg/seekp` 直接定位到指定记录并覆盖状态字段。这满足课程里“写入、读取、更新随机文件记录”的要求。

## Current Protocol

- `PING` -> `PONG`
- `LED:ON` -> `OK LED ON`
- `LED:OFF` -> `OK LED OFF`
- `BUZZER:ON` -> `OK BUZZER ON`
- `BUZZER:OFF` -> `OK BUZZER OFF`
- `VIB:ON` -> `OK VIB ON`
- `VIB:OFF` -> `OK VIB OFF`
- `OLED:TEXT=...` -> `OK OLED TEXT`
- `STATUS?` -> `STATUS LED=...;BUZZER=...;VIB=...;OLED=...`

`HardwareBridge` 类会封装这些串口命令，后续 OpenCV、UI、API 模块不需要直接拼接底层协议字符串。

后续计划职责：

- 调用 OpenCV 读取 Logitech C270 摄像头画面。
- 接收语音输入或按键触发事件。
- 调用千问视觉/多模态 API。
- 解析模型返回的结构化结果。
- 通过串口向 NUCLEO-F446RE 发送 LED、蜂鸣器、OLED、震动等反馈命令。
- 记录审计日志，并实现课程要求中的随机文件读写更新。
