# The Eye of AI / Embedded AI Reality Bridge

## 项目简介

**The Eye of AI（AI 之眼）** 是一个面向 C++ 大作业与嵌入式原型展示的多模态 AI 系统。

项目的核心出发点不是“做一副真正量产的智能眼镜”，而是做出一个**能够验证智能眼镜核心能力的原型机**：  
让原本被困在 IDE、网页聊天框、桌面 Agent 中的 AI，第一次能够通过摄像头、麦克风、按键、屏幕和灯光，与现实物理世界发生真实交互。

换句话说，这个项目不是单纯做一个聊天机器人，也不是单纯做一个硬件小车或传感器实验，而是尝试搭一座桥：

```text
数字世界中的 AI 推理能力  <->  现实世界中的感知、触发与反馈
```

在当前实现中：

- **Qwen** 负责视觉理解与语音识别；
- **DeepSeek** 负责自然语言推理、整理回答和连续对话；
- **Raspberry Pi 5** 负责运行桥接程序、摄像头/麦克风采集、调用云端模型；
- **NUCLEO-F446RE** 负责按键、串口协议、外设触发和本地反馈；
- **Windows Control Center** 负责桌面演示、日志、历史记录、系统诊断和 Agent 对话体验。

## Windows 端启动入口（推荐）

如果你只是想直接运行桌面应用，优先使用下面两个入口：

1. 直接双击：

```text
dist/windows_control_center/embedded_ai_control_center.exe
```

2. 或在仓库根目录执行：

```powershell
.\run-windows-control-center.ps1
```

其中：

- `embedded_ai_control_center.exe` 是最终可分发主程序；
- `run-windows-control-center.ps1` 会自动定位上面的可执行文件并启动；
- 如果 `dist/windows_control_center/` 不存在，说明当前机器还没有完成 Windows 端构建，需要先执行 README 下方的构建步骤。

## 这不是“智能眼镜成品”

请特别注意本项目的定位：

- 当前硬件形态是**原型机**；
- 目标是验证“智能眼镜该有的能力”，不是制造最终眼镜外形；
- 当前已经验证的能力包括：
  - 看见当前画面；
  - 识别语音指令；
  - 对当前现实场景作出分析；
  - 将 AI 回答同步输出到本地 LCD、树莓派终端和 Windows 桌面应用；
  - 通过物理按键触发完整交互闭环。

## 系统架构

### 主链路

```text
三键键盘 / NUCLEO 按钮
        ->
    NUCLEO-F446RE
        ->
 Raspberry Pi Bridge
        ->
 摄像头 / 麦克风采集
        ->
 Qwen (ASR / Vision)
        ->
 DeepSeek (Reasoning / Final Answer)
        ->
 LCD / 三色灯 / 桌面控制中心 / 日志与历史记录
```

### 职责划分

#### 1. Raspberry Pi 5

- 连接 Logitech C270 摄像头与麦克风；
- 运行核心桥接程序 `embedded_ai_pc_bridge`；
- 负责调用 Qwen 与 DeepSeek API；
- 保存拍照、录音、日志、会话记录；
- 通过串口与 NUCLEO 通信；
- 可注册为开机自启动服务。

#### 2. NUCLEO-F446RE

- 接收物理输入（板载蓝色按钮 / 三键键盘等）；
- 控制 LED、LCD、其他外设；
- 与树莓派通过串口握手；
- 作为“现实触发器”和“本地快速反馈层”。

#### 3. Windows Control Center

- 通过 SSH 获取树莓派状态、日志、图片、会话；
- 展示实时对话、历史记录、连接诊断、摄像头画面；
- 提供桌面 Agent 对话入口；
- 提供一键完整检查、验收报告导出、历史会话恢复。

## 仓库目录结构

```text
Embedded_AI/
  hardware/
    firmware/
      nucleo_pingpong/             # NUCLEO-F446RE 固件（C++）
    pi_bridge/                     # 树莓派 / PC 桥接程序（C++）

  software/
    windows_control_center/        # Qt 6 + C++ + Widgets 桌面应用（主展示端）

  scripts/                         # 构建、自启动、SSH、HUD 相关脚本
  docs/                            # 装配、演示、自启动、接线说明文档
  assets/                          # 概念图、接线图、辅助图片
  captures/                        # 运行时保留的图片/录音/历史素材（保留）
  outputs/                         # 文档、报告、导出产物（保留）
  The_Eye_of_AI_generated_figures/ # 已生成的论文/答辩插图（保留）
  config/                          # 本地配置与密钥文件（注意不要提交真实密钥）
  dist/                            # Windows 可分发程序目录
```

## 技术栈

### 核心语言

- **C++17**

### 桌面端

- **Qt 6**
- Widgets
- QSS
- CMake
- CTest / Qt Test

### 树莓派端

- CMake
- libcurl
- OpenCV
- 串口通信
- systemd user service

### 嵌入式端

- STM32 / NUCLEO-F446RE
- STM32CubeIDE / STM32CubeProgrammer
- 串口协议 + 外设驱动

### 云端模型

- **Qwen**：视觉理解、语音识别
- **DeepSeek**：文本推理、最终回答、连续对话

## 当前已实现的功能

### 感知

- Logitech C270 摄像头拍照
- Logitech C270 麦克风录音
- Qwen ASR 语音识别
- Qwen VL 当前画面理解

### 推理

- DeepSeek 文本回答
- 多轮连续对话
- “结合当前画面”模式下的视觉 + 文本联合推理

### 硬件反馈

- NUCLEO 串口通信
- 三键键盘触发
- LCD 显示 AI 回复
- 状态灯反馈

### 桌面应用

- 实时对话页
- 历史记录页
- 连接诊断页
- 摄像头画面页
- 原始日志页
- 设置页
- 一键完整检查
- 会话保存与恢复

## 开发环境要求

### Windows 端

建议环境：

- Windows 10 / 11
- PowerShell
- CMake >= 3.16
- Ninja
- Qt 6（用于构建 `windows_control_center`）
- MSYS2 UCRT64（用于桥接程序的 Windows 构建）
- Git

### Raspberry Pi 端

建议环境：

- Raspberry Pi 5
- Raspberry Pi OS 64-bit
- `cmake`
- `ninja-build`
- `g++`
- `pkg-config`
- `libopencv-dev`
- `libcurl4-openssl-dev`
- `ffmpeg`
- `alsa-utils`

### 嵌入式端

- NUCLEO-F446RE
- STM32CubeIDE 或 STM32CubeProgrammer
- USB 数据线

## 配置文件说明

请不要把真实 API key 提交到 Git。

项目中常见配置文件：

```text
config/qwen-vision.example.ini   # 示例配置
config/qwen-vision.ini           # 本地实际 Qwen 配置
config/qwen-vision.key           # 本地实际 Qwen key
config/deepseek.key              # 本地实际 DeepSeek key
```

### Qwen key

`qwen-vision.key` 只写一行真实 key，不要加引号，不要写 `api_key=`。

### DeepSeek key

`deepseek.key` 也只写一行真实 key。

## 完整构建流程

下面给出当前项目推荐的完整构建方式。

### 一、构建 NUCLEO 固件

固件目录：

```text
hardware/firmware/nucleo_pingpong
```

推荐流程：

1. 用 STM32CubeIDE 打开该工程；
2. 编译生成固件；
3. 使用 ST-LINK 烧录到 `NUCLEO-F446RE`；
4. 上电后确认串口设备能被树莓派识别。

如果你使用 STM32CubeProgrammer：

1. 先编译得到 `.elf` / `.bin`；
2. 打开 STM32CubeProgrammer；
3. 连接 ST-LINK；
4. 烧录固件到板子。

### 二、在 Raspberry Pi 上构建桥接程序

桥接程序目录：

```text
hardware/pi_bridge
```

在项目根目录执行：

```bash
cmake -S . -B build-pi -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF
cmake --build build-pi
```

生成的核心程序通常位于：

```text
build-pi/hardware/pi_bridge/embedded_ai_pc_bridge
```

### 三、在 Windows 上构建桌面控制中心

Windows 主展示应用目录：

```text
software/windows_control_center
```

在项目根目录执行：

```powershell
cmake -S . -B build-qt -G Ninja -DBUILD_TESTING=ON
cmake --build build-qt
```

如果只想快速构建分发程序，也可以用脚本：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows-control-center.ps1
```

构建成功后，可分发程序位于：

```text
dist/windows_control_center/
```

主程序：

```text
dist/windows_control_center/embedded_ai_control_center.exe
```

### 四、可选：在 Windows/MSYS2 上构建桥接程序

仅用于本地调试时需要，正常最终展示不必依赖它。

你需要先准备好 MSYS2 UCRT64 环境，再执行类似命令：

```powershell
cmake -S . -B build-msys2-console -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF
cmake --build build-msys2-console
```

运行入口脚本：

```powershell
.\run-pc.ps1 COM11
```

## 运行流程

### 方案 A：树莓派原型机独立运行

适合硬件展示。

1. Raspberry Pi 连接：
   - Logitech C270 摄像头/麦克风
   - NUCLEO-F446RE
   - LCD、状态灯、三键键盘
2. 树莓派运行桥接程序：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen
```

或者使用已安装的自启动服务。

3. 用户按下物理按键；
4. 录音 / 拍照 / Qwen 识别 / DeepSeek 回答；
5. LCD 与状态灯输出反馈；
6. 日志和会话保存在树莓派本地。

### 方案 B：树莓派 + Windows Control Center 联合演示

适合答辩展示。

1. 树莓派运行桥接程序或自启动服务；
2. Windows 端启动桌面应用，推荐二选一：

直接双击可执行文件：

```text
dist/windows_control_center/embedded_ai_control_center.exe
```

或者使用仓库根目录启动脚本：

```powershell
.\run-windows-control-center.ps1
```

3. 桌面控制中心通过 SSH 获取：
   - 树莓派连接状态
   - 日志
   - 最近图片
   - 会话记录
   - 硬件诊断结果
4. 如果树莓派硬件触发了新一轮识别，桌面端会同步显示：
   - 用户输入（语音转文字 / 画面上下文）
   - 最新抓拍图片
   - AI 最终回答

## 树莓派自启动

安装：

```bash
bash scripts/install-pi-service.sh
```

卸载：

```bash
bash scripts/uninstall-pi-service.sh
```

说明文档：

```text
docs/pi-autostart.md
```

## 一键检查与验收

Windows Control Center 已支持一键完整检查，能够验证：

- PC 到树莓派 SSH
- `embedded-ai.service`
- `/dev/ttyACM*` 与 NUCLEO 握手
- 摄像头与麦克风
- Qwen 配置与视觉调用
- DeepSeek 配置与文本调用
- LCD / 三色灯 / 三键键盘
- 最近照片、日志、会话文件可读写状态

并导出：

- JSON 验收报告
- 文本验收报告

## 自动化测试

执行：

```powershell
cmake -S . -B build-qt -G Ninja -DBUILD_TESTING=ON
cmake --build build-qt
ctest --test-dir build-qt --output-on-failure
```

当前重点覆盖：

- Markdown / LaTeX / 代码块 / 表格渲染
- Enter 发送、Shift+Enter 换行
- 会话创建、保存、恢复、历史继续对话
- Qwen -> DeepSeek 工作流与失败回退
- SSH 日志与硬件状态解析
- API 超时、空响应、网络异常

## 历史记录与图片保留说明

为了答辩和留档，以下目录/文件建议保留：

- `captures/`
- `outputs/`
- `The_Eye_of_AI_generated_figures/`

这些内容包含：

- 历史拍照
- 录音或运行素材
- 会话相关导出
- 答辩/论文插图

## 文档入口

推荐阅读顺序：

1. [README.md](README.md)
2. `docs/demo-guide.md`
3. `docs/current-nucleo-assembly.md`
4. `docs/pi-autostart.md`
5. `docs/pi-hud-wiring-guide.md`
6. `docs/three-key-ws2812-design.md`

## 项目目录整理建议

为了保留完整、可复现、便于演示的项目内容，建议保留：

- `hardware/`
- `software/`
- `scripts/`
- `docs/`
- `assets/`
- `captures/`
- `outputs/`
- `The_Eye_of_AI_generated_figures/`
- `config/qwen-vision.example.ini`
- `README.md`
- `CMakeLists.txt`
- `run-pc.ps1`
- `run-qwen.ps1`
- `run-windows-control-center.ps1`

通常可以按需清理：

- 各类 `build-*` 构建目录
- 临时构建缓存与本地运行时缓存文件

## 项目总结

The Eye of AI 的意义不在于“已经做出一副成品眼镜”，而在于：

- 它验证了 AI 可以从“只在屏幕里工作”走向“与现实世界形成闭环”；
- 它验证了视觉、语音、推理、物理触发和本地反馈可以组合成一个完整系统；
- 它为未来真正的可穿戴 AI 设备提供了一个可复现实验原型。

如果把传统 IDE 补全工具、网页聊天机器人、桌面 Agent 看作 AI 发展的前几站，那么 **The Eye of AI** 做的事情，就是让 AI 开始真正“看见”并“接触”现实。
