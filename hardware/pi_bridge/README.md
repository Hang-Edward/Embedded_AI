# Hardware Pi Bridge

这里是树莓派/PC 侧的 C++ 桥接程序。它负责摄像头抓拍、麦克风录音、Qwen API 调用、控制台菜单、审计日志，以及通过串口控制 NUCLEO-F446RE。

## 目录

```text
hardware/pi_bridge/
  CMakeLists.txt
  include/   # 头文件：接口、类声明、数据结构
  src/       # C++ 实现文件和 main.cpp
```

头文件和 `.cpp` 已分离，方便展示面向对象结构，也方便后续继续扩展硬件模块。

## 构建

Windows/MSYS2 UCRT64：

```powershell
cd "D:\VScode Projects\Embedded_AI"
& "$env:USERPROFILE\scoop\apps\msys2\current\msys2_shell.cmd" -defterm -no-start -ucrt64 -c "cd '/d/VScode Projects/Embedded_AI' && cmake -S . -B build-msys2-console -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF -DBUILD_LEGACY_IMGUI_GUI=OFF && cmake --build build-msys2-console"
```

树莓派：

```bash
cd ~/Embedded_AI
cmake -S . -B build-pi -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF -DBUILD_LEGACY_IMGUI_GUI=OFF
cmake --build build-pi
```

## 运行

Windows 推荐用根目录脚本：

```powershell
.\run-pc.ps1 COM11
.\run-qwen.ps1 COM11
```

树莓派菜单模式：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen --menu
```

树莓派按钮语音助手模式：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen
```

Mock 模式不会调用真实 API：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --menu
```

## Qwen 配置

真实配置文件不要提交到 Git：

```text
config/qwen-vision.ini
config/qwen-vision.key
```

`config/qwen-vision.key` 只写一行 API key，不要加引号，不要写 `api_key=`。

推荐配置：

```ini
enabled=true
model=qwen3-vl-8b-instruct
asr_model=qwen3-asr-flash
audio_device=plughw:2,0
api_key_file=config/qwen-vision.key
```

`audio_device` 要以树莓派上 `arecord -l` 的结果为准。Logitech C270 之前验证过的值是 `plughw:2,0`。

## 主要类

- `App`：控制台菜单和应用流程。
- `Console`：终端输入输出封装。
- `SerialPort`：Windows/Linux 串口读写封装。
- `HardwareBridge`：统一硬件协议封装。
- `CameraService` / `OpenCvCameraService`：摄像头抽象和 OpenCV 实现。
- `AudioRecorder` / `ShellAudioRecorder`：录音抽象和 `arecord` 实现。
- `AiVisionService` / `MockAiVisionService` / `QwenVisionService`：视觉 AI 抽象、本地 mock 和真实 Qwen 实现。
- `QwenAsrService`：Qwen 语音识别服务。
- `HttpClient` / `CurlHttpClient`：HTTP 抽象和 libcurl 实现。
- `AuditLogStore`：二进制审计日志，支持随机访问更新。
- `DeviceComponent` / `OutputDevice` / `NucleoOutputDevices`：硬件设备抽象层级。

## 串口协议

```text
PING -> PONG
LED:ON -> OK LED ON
LED:OFF -> OK LED OFF
BUZZER:ON -> OK BUZZER ON
BUZZER:OFF -> OK BUZZER OFF
VIB:ON -> OK VIB ON
VIB:OFF -> OK VIB OFF
OLED:TEXT=... -> OK OLED TEXT
STATUS? -> STATUS LED=...;BUZZER=...;VIB=...;OLED=...
EVENT BUTTON PRESSED -> NUCLEO 蓝色按钮事件
```

`HardwareBridge` 会封装这些命令，上层摄像头、AI API、控制台 UI 不需要直接拼底层协议字符串。
