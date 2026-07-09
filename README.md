# The Eye of AI

## 项目简介

**The Eye of AI（AI 之眼）** 是一个面向 C++ 课程设计与嵌入式原型展示的多模态 AI 系统。

本项目的核心目标，不是直接制造一副可量产的智能眼镜成品，而是构建一个**能够验证“智能眼镜核心能力”是否可行的原型机**。它试图回答一个更本质的问题：

> 当 AI 已经从 IDE 中的代码补全工具，发展到网页聊天机器人，再发展到能够操作文件和浏览网页的桌面 Agent 之后，能否真正走出屏幕，与现实世界建立闭环交互？

围绕这个问题，The Eye of AI 通过摄像头、麦克风、物理按键、LCD、状态灯和桌面控制中心，搭建了一条从**现实输入**到**AI 感知与推理**，再到**本地反馈输出**的完整链路。

## 项目定位

请特别注意，本项目当前是一个**功能原型机**：

- 目标是验证智能眼镜所需的关键交互能力；
- 重点是“看见现实、理解现实、对现实作出反馈”；
- 并不追求当前阶段就做出最终可穿戴外形。

因此，本项目更适合被理解为：

```text
现实世界 <-> 多模态 AI 推理系统 <-> 本地反馈设备
```

而不是单纯的聊天机器人项目，或者单纯的硬件实验项目。

## 系统组成

### 1. 感知与推理

- **Qwen**：负责语音识别、图像理解与视觉场景分析；
- **DeepSeek**：负责最终文本回答、多轮上下文推理与自然语言组织。

### 2. 运行与控制

- **Raspberry Pi 5**：运行桥接程序，采集摄像头/麦克风数据，调用模型 API，并管理日志、图片和会话；
- **NUCLEO-F446RE**：负责物理触发、串口通信、外设联动和本地反馈；
- **Windows Control Center**：负责桌面端展示、历史记录、连接诊断、会话浏览与答辩演示。

### 3. 输出与反馈

- LCD 显示 AI 回复；
- 状态灯显示设备当前状态；
- 三键键盘或按键触发新一轮交互；
- 桌面端同步显示图像、语音转文本、模型回复和历史会话。

## 仓库结构

```text
Embedded_AI/
├─ hardware/                         # 固件与树莓派桥接程序
│  ├─ firmware/
│  └─ pi_bridge/
├─ software/                         # Windows 桌面应用
│  └─ windows_control_center/
├─ scripts/                          # 构建、部署、自启动与辅助脚本
├─ docs/                             # 接线、装配、演示与说明文档
├─ assets/                           # 项目图片与界面资源
├─ captures/                         # 历史拍照、录音与运行素材
├─ outputs/                          # 报告、导出文档与结果文件
├─ dist/
│  └─ windows_control_center/        # 已打包好的 Windows 可执行程序
├─ config/                           # 本地配置模板
├─ CMakeLists.txt
├─ README.md
├─ run-pc.ps1
├─ run-qwen.ps1
└─ run-windows-control-center.ps1
```

## Windows 桌面端启动方式

### 推荐方式：直接运行分发版

本仓库已经包含可直接启动的 Windows 桌面应用。克隆仓库后，优先使用下面任一方式启动：

#### 方式 A：双击可执行文件

直接打开：

```text
dist/windows_control_center/embedded_ai_control_center.exe
```

#### 方式 B：在仓库根目录运行启动脚本

```powershell
.\run-windows-control-center.ps1
```

这个脚本会自动定位：

```text
dist/windows_control_center/embedded_ai_control_center.exe
```

并以正确工作目录启动程序。

## 如果 `dist` 不存在怎么办

正常情况下，本仓库应当自带：

```text
dist/windows_control_center/
```

如果你的本地副本缺少该目录，通常说明：

- 当前拿到的是未包含分发目录的旧拷贝；
- 或者仓库内容未完整同步。

此时请优先重新获取完整项目目录，而不是直接假定需要手动构建。

只有在确实需要重新生成分发版时，才执行下面的构建脚本：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows-control-center.ps1
```

## Windows Control Center 功能

桌面应用当前支持：

- 实时对话展示；
- 历史会话浏览与恢复；
- 连接诊断；
- 摄像头画面同步；
- 原始日志查看；
- 设置页；
- 本地 Agent 对话；
- 与树莓派桥接程序同步会话内容。

## 树莓派原型运行方式

在树莓派端，桥接程序负责采集摄像头/麦克风数据，并与 NUCLEO 协同工作。

典型运行链路如下：

```text
物理按键 / 三键键盘
        ->
    NUCLEO-F446RE
        ->
 Raspberry Pi Bridge
        ->
 摄像头 / 麦克风采集
        ->
 Qwen 识图 / 语音识别
        ->
 DeepSeek 生成最终回答
        ->
 LCD / 状态灯 / 桌面控制中心同步反馈
```

## 环境与构建

虽然仓库已经包含 Windows 分发版，但如果需要重新构建，下面是完整流程。

### Windows 端构建桌面应用

建议环境：

- Windows 10 / 11
- PowerShell
- CMake
- Ninja
- Qt 6
- MSYS2 UCRT64
- Git

构建命令：

```powershell
cmake -S . -B build-qt -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=ON
cmake --build build-qt
```

或者直接运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows-control-center.ps1
```

构建完成后，分发目录位于：

```text
dist/windows_control_center/
```

### Raspberry Pi 端构建桥接程序

建议环境：

- Raspberry Pi OS 64-bit
- cmake
- ninja-build
- g++
- pkg-config
- OpenCV
- libcurl
- ffmpeg
- alsa-utils

构建命令示例：

```bash
cmake -S . -B build-pi -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF
cmake --build build-pi
```

### NUCLEO 固件

建议使用 STM32CubeIDE 或 STM32CubeProgrammer 构建与烧录。

固件目录：

```text
hardware/firmware/nucleo_pingpong
```

## 配置文件

本项目使用本地密钥文件，不应将真实密钥提交到仓库。

示例配置包括：

```text
config/qwen-vision.example.ini
config/qwen-vision.ini
config/qwen-vision.key
config/deepseek.key
```

其中：

- `qwen-vision.key`：文件中仅保留一行真实 Qwen key；
- `deepseek.key`：文件中仅保留一行真实 DeepSeek key。

## 建议保留的项目内容

为了保证项目可运行、可展示、可追踪，建议保留：

- `hardware/`
- `software/`
- `scripts/`
- `docs/`
- `assets/`
- `captures/`
- `outputs/`
- `dist/`
- `config/qwen-vision.example.ini`
- `README.md`
- `CMakeLists.txt`
- `run-pc.ps1`
- `run-qwen.ps1`
- `run-windows-control-center.ps1`

## 项目意义

The Eye of AI 的意义，不在于当前已经拥有一副工业化成品眼镜，而在于它验证了以下事情：

- AI 可以不再只停留在屏幕内；
- AI 可以通过视觉、语音、触发器与现实世界建立闭环；
- AI 可以从“数字工具”进一步演进为“现实世界接口”。

如果说 IDE 补全工具、网页聊天机器人、桌面 Agent 是 AI 发展的几个阶段，那么 **The Eye of AI** 所做的工作，就是让 AI 开始真正具备“看见现实并对现实作出反应”的能力。
