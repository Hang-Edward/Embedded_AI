# PC C++ Application

这里是 PC 端 C++ 主程序。它负责摄像头抓拍、Qwen 视觉 API 调用、控制台 UI、审计日志，以及通过串口控制 NUCLEO 的硬件反馈。

## Directory Layout

```text
src/pc/
  CMakeLists.txt
  README.md
  include/     # 头文件：类声明、接口、数据结构
  src/         # C++ 实现文件：函数实现和 main.cpp
```

头文件和 `.cpp` 已经分离，便于老师检查面向对象结构，也便于后续继续扩展模块。

## Build

摄像头模块依赖 MSYS2 UCRT64 版本的 OpenCV 和 libcurl。不要把 Scoop 安装的 VC16 OpenCV、普通 MinGW 和 UCRT64 OpenCV 混用。

```powershell
cd "D:\VScode Projects\Embedded_AI"
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && cmake -S . -B build-msys2-shell -G Ninja && cmake --build build-msys2-shell"
```

## Run

推荐使用项目根目录的便捷脚本。脚本只会临时把 MSYS2 UCRT64 的 DLL 路径加入当前进程，不会修改系统环境变量。

```powershell
.\run-pc.ps1 COM11
```

使用 Qwen：

```powershell
.\run-qwen.ps1 COM11
```

默认连接 `COM11`：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build-msys2-shell/src/pc/embedded_ai_pc_bridge.exe COM11"
```

如果 COM 口变化，可以手动指定：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build-msys2-shell/src/pc/embedded_ai_pc_bridge.exe COM12"
```

非交互 demo 模式：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build-msys2-shell/src/pc/embedded_ai_pc_bridge.exe COM11 --demo"
```

指定摄像头索引：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build-msys2-shell/src/pc/embedded_ai_pc_bridge.exe COM11 --camera 1"
```

## Console UI

当前菜单包含：

- 测试 NUCLEO 串口连接。
- 打开或关闭板载/外接 LED。
- 模拟 AI 任务。
- 抓取摄像头画面并保存到 `captures/latest-frame.jpg`。
- 抓取摄像头画面并执行视觉分析。
- 查看历史审计日志。
- 按记录 ID 随机更新日志状态。

## Qwen Vision

真实配置文件不要提交到 Git。当前推荐使用项目专用 key 文件：

```text
config/qwen-vision.key
```

这个文件只写一行 API Key，不要加引号，不要加 `api_key=`。例如：

```text
sk-xxxxxxxxxxxxxxxx
```

`config/qwen-vision.ini` 和 `config/qwen-vision.key` 都在 `.gitignore` 中，不会上传到 GitHub。仓库里只提交 `config/qwen-vision.example.ini`。

使用 Qwen 服务入口：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build-msys2-shell/src/pc/embedded_ai_pc_bridge.exe COM11 --qwen"
```

注意：`--qwen` 在菜单选择 “Capture and analyze current frame” 或运行 `--demo` 时会发起真实 Qwen 视觉请求，真实请求会产生 token 计费。

## OOP Structure

- `App`：控制台菜单和应用流程。
- `Console`：终端输入输出封装。
- `SerialPort`：Windows 串口读写封装。
- `HardwareBridge`：统一硬件协议封装。
- `CameraService` / `OpenCvCameraService`：摄像头抓帧抽象与 OpenCV 实现。
- `AiVisionService` / `MockAiVisionService` / `QwenVisionService`：视觉 AI 抽象、本地 mock 和真实 Qwen 实现。
- `HttpClient` / `CurlHttpClient`：HTTP 抽象和 libcurl 实现。
- `AuditLogStore`：二进制审计日志，支持随机读取和随机更新。
- `DeviceComponent` / `OutputDevice` / `Nucleo...Device`：硬件输出设备继承层级。
- `SceneTask`：AI 场景任务数据结构。

## Random File Processing

`AuditLogStore` 使用固定长度二进制记录写入 `audit-log.dat`。更新记录状态时，程序根据 `recordId` 计算偏移量，然后用 `seekg/seekp` 直接定位到指定记录并覆盖状态字段。这满足课程中“写入、读取、更新随机文件记录”的要求。

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

`HardwareBridge` 会封装这些串口命令，OpenCV、AI API、控制台 UI 不需要直接拼接底层协议字符串。
