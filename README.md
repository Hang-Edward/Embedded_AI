# Embedded AI Reality Bridge

这是一个面向 C++ 大作业和嵌入式原型展示的项目。目标是做出一个类似 AI 智能眼镜核心能力的桌面原型：摄像头读取画面，按钮或语音触发提问，树莓派运行 C++ 主程序调用 Qwen 视觉/语音 API，NUCLEO-F446RE 负责按键、LED、OLED、蜂鸣器、震动等底层硬件反馈。

当前项目已经拆成两个主要部分，避免硬件代码和 Windows 展示应用互相污染。

## 目录结构

```text
Embedded_AI/
  hardware/
    firmware/              # NUCLEO-F446RE 固件，C++ 编写
    pi_bridge/             # 树莓派/PC 桥接程序，负责摄像头、语音、Qwen、串口和日志
  software/
    windows_control_center/ # Qt 6 + C++ + Widgets + QSS 桌面展示应用
    web_preview/            # 旧网页预览，仅用于展示和文档
    legacy_imgui_gui/       # 旧 ImGui 原型，默认不构建
  docs/                     # 演示、装配、自启动说明
  assets/                   # 概念图和接线图
  config/                   # Qwen 配置示例；真实 key 不提交
  scripts/                  # 树莓派自启动脚本
```

## 硬件部分

硬件主链路位于 `hardware/pi_bridge` 和 `hardware/firmware`：

- `hardware/pi_bridge`：运行在树莓派或 Windows/MSYS2 上的 C++ 桥接程序。
- `hardware/firmware/nucleo_pingpong`：NUCLEO-F446RE 固件，处理串口命令和板载蓝色按钮事件。
- `scripts/install-pi-service.sh`：把树莓派端程序安装为开机自启动服务。

树莓派端推荐构建：

```bash
cmake -S . -B build-pi -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF -DBUILD_LEGACY_IMGUI_GUI=OFF
cmake --build build-pi
```

运行菜单调试模式：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen --menu
```

运行按钮语音助手模式：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen
```

## Windows 桌面展示应用

新的展示端位于 `software/windows_control_center`，技术路线是：

```text
Qt 6 + C++ + Widgets + QSS
```

第一版目标：

- 主窗口和页面切换。
- LLM 风格对话页面。
- 硬件状态检测页面。
- 摄像头预览页面。
- 日志页面。
- SSH 连接设置页面。
- 假数据 UI，可在接入真 SSH 前先验证界面和响应式布局。

连接策略：

```text
最近一次成功 IP -> ssh ch@172.20.10.6 -> 用户手动输入 ssh ch@ip
```

如果检测到 PC 与树莓派 IP 疑似不在同一局域网，应用会给出提醒。常见家庭/热点网络通常前三段 IP 相同，但严格判断仍以子网掩码为准。

## Web Preview

旧网页展示仍可本地预览：

```powershell
cd "D:\VScode Projects\Embedded_AI"
node server.js
```

然后访问：

```text
http://127.0.0.1:8765/
```

网页代码位于 `software/web_preview`，只是展示用，不作为主要 C++ 作业代码。

## 配置文件

真实 API key 不要提交到 Git。项目只提交示例文件：

```text
config/qwen-vision.example.ini
```

本地使用时创建：

```text
config/qwen-vision.ini
config/qwen-vision.key
```

`qwen-vision.key` 只写一行 API key，不要加引号，不要写 `api_key=`。

## 当前演示能力

- Logitech C270 摄像头拍照。
- Logitech C270 麦克风录音。
- Qwen ASR 语音识别。
- Qwen VL 画面分析。
- NUCLEO-F446RE 串口通信。
- 板载蓝色按钮触发语音输入和拍照分析。
- 审计日志文件读写和按记录 ID 更新。
- 树莓派开机自启动按钮助手服务。

## 后续路线

1. 完成 Qt Windows 展示应用第一版。
2. 用 SSH 接入树莓派真日志、真图片和真硬件状态。
3. 接入 Gravity IO、OLED、外接按钮、蜂鸣器和震动模块。
4. 打包 Windows 安装包或可双击启动目录。
