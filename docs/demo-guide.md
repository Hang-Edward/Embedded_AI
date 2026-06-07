# Embedded AI Reality Bridge 演示文档

本文档用于课堂展示、答辩演示和现场排障。当前演示版本已经在树莓派 5 上跑通：摄像头采集画面，C++ 程序调用 Qwen 视觉模型分析画面，并通过串口与 NUCLEO-F446RE 通信。

## 演示目标

本项目展示一个“AI 智能眼镜/视觉助手”的桌面原型。它暂时不做成眼镜外形，而是先验证核心能力：

- 摄像头实时获取当前画面。
- 树莓派运行 C++ 主程序，负责摄像头、麦克风、API 调用和演示交互。
- Qwen 视觉模型理解画面，返回场景描述、解题辅助或安全风险分析。
- NUCLEO-F446RE 作为嵌入式执行端，通过串口接收命令，处理板载蓝色按钮事件和 LED 等硬件反馈。
- 程序保存审计日志，展示 C++ 文件读写、随机访问更新和面向对象设计。

## 当前硬件状态

```text
Logitech C270 摄像头/麦克风 -> USB
Raspberry Pi 5 8GB
    -> Linux / C++ 主程序 / OpenCV / libcurl
    -> Qwen 视觉 API / Qwen ASR
    -> USB 串口
NUCLEO-F446RE
    -> 接收 PING、LED、STATUS 等命令
    -> 发送蓝色按钮事件
    -> 板载 LED 反馈
```

当前不要把 Gravity IO 扩展板插到树莓派 40-pin 上。Gravity IO 是 Arduino/NUCLEO 兼容扩展板，不是树莓派 HAT。后续如果接 OLED、按钮、蜂鸣器、震动模块，应先接到 NUCLEO 的 Arduino 排针侧，再由 NUCLEO 负责底层硬件控制。

## 上台前检查清单

- 树莓派已接电并正常进入 Linux。
- Logitech C270 已插入树莓派 USB 口。
- NUCLEO-F446RE 已通过 USB 插入树莓派。
- 树莓派可以联网，校园网没有掉认证。
- `config/qwen-vision.ini` 存在。
- `config/qwen-vision.key` 存在，并且只在本地保存，不提交 Git。
- 项目已经完成编译，存在 `build-pi/hardware/pi_bridge/embedded_ai_pc_bridge`。

在树莓派终端中执行：

```bash
cd ~/Embedded_AI
lsusb
ls /dev/video*
ls /dev/ttyACM*
```

期望看到 Logitech 摄像头、`/dev/video0` 或类似设备，以及 `/dev/ttyACM0` 或 `/dev/ttyACM1`。

## 启动方式

菜单调试模式：

```bash
cd ~/Embedded_AI
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen --menu
```

按钮语音助手模式：

```bash
cd ~/Embedded_AI
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen
```

Mock 模式不调用真实 API：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --menu
```

## 推荐演示流程

### 1. 硬件连接测试

菜单模式下选择：

```text
1
```

预期现象：

- 程序显示 `Hardware connection OK.`
- 能读取设备状态，例如 LED、蜂鸣器、震动、OLED 状态。
- 说明树莓派与 NUCLEO 的 USB 串口通信正常。

### 2. LED 反馈测试

菜单模式下选择：

```text
2
```

预期现象：

- NUCLEO 板载绿色 LED 亮起。
- 程序显示 `LED command OK.`

然后选择：

```text
3
```

预期现象：NUCLEO 板载绿色 LED 熄灭。

### 3. 摄像头抓拍

菜单模式下选择：

```text
7
```

预期现象：

- 程序显示摄像头抓拍成功。
- 图片保存到 `captures/latest-frame.jpg`。

### 4. Qwen 视觉描述

菜单模式下选择：

```text
8
1
```

含义：

- `8`：抓取当前画面并分析。
- `1`：让模型描述当前场景。

预期现象：

- 程序先保存一张摄像头图片。
- 程序调用 `qwen3-vl-8b-instruct`。
- 终端输出模型对当前画面的描述。
- 审计日志写入一条视觉分析记录。

### 5. 按钮语音助手

按钮模式下，直接按 NUCLEO 蓝色按钮：

```text
按蓝色按钮 -> 说话 -> 自动拍照 -> Qwen 根据语音和图片回答
```

如果语音为空或识别失败，系统会自动 fallback 到场景描述。

## 作业要求对应关系

- 面向对象设计：`CameraService`、`AiVisionService`、`HardwareBridge`、`PrototypeDeviceSet`、`AuditLogStore` 等类。
- 继承和多态：Mock 视觉服务和 Qwen 视觉服务实现同一接口；摄像头、录音、HTTP、输出设备也通过接口抽象。
- 文件读写：`audit-log.dat` 保存演示记录。
- 随机访问文件：审计日志支持按记录 ID 更新状态。
- UI：当前有控制台菜单，新的 Windows 展示端使用 Qt 6 Widgets。
- C++ 代码规模：硬件桥接、固件和 Qt 应用都使用 C++ 编写，可继续扩展到 2000 行以上。

## 常见问题

### 找不到 NUCLEO 串口

```text
ERROR: Cannot open serial port /dev/ttyACM0
```

检查：

```bash
ls /dev/ttyACM*
```

如果出现 `/dev/ttyACM1`，运行时改用：

```bash
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM1 --qwen --menu
```

### 找不到摄像头

```text
camera capture failed: no readable camera found
```

检查：

```bash
lsusb
ls /dev/video*
```

单独抓拍测试：

```bash
mkdir -p ~/camera-test
ffmpeg -y -f v4l2 -video_size 1280x720 -i /dev/video0 -frames:v 1 -update 1 ~/camera-test/test.jpg
```

### Qwen API 调用失败

确认配置文件存在：

```bash
ls config/qwen-vision.ini config/qwen-vision.key
```

不要把 `qwen-vision.key` 内容发到聊天、截图或 GitHub。

如果校园网掉认证，树莓派可能无法访问 API。先测试网络：

```bash
ping -c 4 223.5.5.5
ping -c 4 dashscope.aliyuncs.com
```

## 最小成功演示

如果现场时间紧，只需要完成这几步：

```text
1 -> 确认 NUCLEO 通信正常
2 -> 点亮 NUCLEO 板载 LED
7 -> 摄像头抓拍成功
8 -> 1 -> Qwen 描述当前画面
```

只要这四步成功，就能证明项目完成了从画面采集、AI 理解、C++ 调度到嵌入式硬件反馈的完整闭环。
