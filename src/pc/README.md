# PC C++ Application

这里是 PC 端 C++ 主程序。当前版本已经包含控制台 UI、OpenCV 摄像头抓拍、mock 视觉分析、NUCLEO 串口控制、二进制审计日志和随机文件更新，用来验证“电脑负责视觉与 AI，NUCLEO 负责硬件反馈”的第一阶段原型。

## Build

摄像头模块依赖 MSYS2 UCRT64 版本的 OpenCV。不要把 Scoop 安装的 VC16 OpenCV 和 MinGW/UCRT64 混用。

```powershell
cd "D:\VScode Projects\Embedded_AI"
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && cmake -S . -B build -G Ninja && cmake --build build"
```

## Run

默认连接 `COM11`：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build/src/pc/embedded_ai_pc_bridge.exe COM11"
```

如果 COM 口变化，可以手动指定：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build/src/pc/embedded_ai_pc_bridge.exe COM12"
```

非交互 demo 模式：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build/src/pc/embedded_ai_pc_bridge.exe COM11 --demo"
```

指定摄像头索引：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build/src/pc/embedded_ai_pc_bridge.exe COM11 --camera 1"
```

## Console UI

当前菜单包含：

- 测试 NUCLEO 串口连接。
- 打开或关闭板载 LED。
- 模拟 AI 任务。
- 抓取摄像头画面并保存到 `captures/latest-frame.jpg`。
- 抓取摄像头画面并执行 mock 视觉分析。
- 查看历史审计日志。
- 按记录 ID 随机更新日志状态。

mock 视觉分析包含三类任务：

- `SCENE_DESCRIPTION`：描述当前画面，低风险，OLED 显示 `SCENE OK`。
- `PROBLEM_SOLVING`：模拟解题提示，中风险，点亮 LED，OLED 显示 `HINT READY`。
- `RISK_ALERT`：模拟安全检查，高风险，点亮 LED，开启蜂鸣器和震动状态，OLED 显示 `RISK ALERT`。

## OOP Structure

- `App`：控制台菜单和应用流程。
- `Console`：终端输入输出封装。
- `SerialPort`：Windows 串口读写封装。
- `HardwareBridge`：统一硬件协议封装。
- `CameraService` / `OpenCvCameraService`：摄像头抓帧抽象与 OpenCV 实现。
- `AiVisionService` / `MockAiVisionService`：视觉 AI 抽象与本地 mock 实现，后续可替换为千问 API 实现。
- `QwenVisionConfig` / `QwenVisionService` / `QwenResponseParser`：千问视觉 API 的配置读取、请求体骨架和响应解析。
- `AuditLogStore`：二进制审计日志，支持随机读取和随机更新。
- `SceneTask`：AI 场景任务数据结构。
- `DeviceState`：预留硬件状态对象。
- `DeviceComponent` / `OutputDevice` / `Nucleo...Device`：硬件输出设备继承层级。

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

## Next API Step

下一步接入千问视觉 API 时，推荐新增一个 `QwenVisionService`，实现 `AiVisionService::analyzeImage()`。这样主程序、摄像头、日志、NUCLEO 控制都不需要大改。

当前已经提供 `QwenVisionService` 骨架。真实配置文件不要提交到 Git：

```powershell
Copy-Item .\config\qwen-vision.example.ini .\config\qwen-vision.ini
```

推荐不要把 API Key 写进 PowerShell 全局环境变量。当前本地配置使用项目专用 key 文件：

```text
config/qwen-vision.key
```

这个文件只写一行 API Key，不要加引号，不要加 `api_key=`，例如：

```text
sk-xxxxxxxxxxxxxxxx
```

`config/qwen-vision.ini` 和 `config/qwen-vision.key` 都在 `.gitignore` 中，不会上传到 GitHub。仓库里只提交 `config/qwen-vision.example.ini`。

使用 Qwen 服务入口：

```powershell
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && ./build/src/pc/embedded_ai_pc_bridge.exe COM11 --qwen"
```

当前 `--qwen` 会在菜单选择“Capture and analyze current frame”或运行 `--demo` 时发起真实 Qwen 视觉请求。真实请求会产生 token 计费，演示前请确认账户余额、免费额度或费用预警。
