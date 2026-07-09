# Raspberry Pi 自启动服务说明

本文档用于把 Embedded AI Reality Bridge 配置为树莓派开机自启动。配置完成后，树莓派通电启动并连接好 NUCLEO 与 Logitech C270 后，不需要键盘输入命令，只要按 NUCLEO 蓝色按钮即可触发：

```text
按钮事件 -> 录音 -> Qwen ASR -> 摄像头拍照 -> Qwen-VL 分析 -> 写入日志
```

如果接着显示器或通过 SSH 登录，可以实时查看日志；如果没有显示器和键盘，服务也会在后台运行。

## 前置条件

在树莓派项目目录中确认：

```bash
cd ~/Embedded_AI
cmake --build build-pi
ls build-pi/hardware/pi_bridge/embedded_ai_pc_bridge
ls config/qwen-vision.ini config/qwen-vision.key
```

确认 NUCLEO、摄像头和麦克风设备存在：

```bash
ls /dev/ttyACM*
ls /dev/video*
arecord -l
```

`config/qwen-vision.ini` 建议包含：

```ini
enabled=true
model=qwen3-vl-8b-instruct
asr_model=qwen3-asr-flash
audio_device=plughw:2,0
api_key_file=config/qwen-vision.key
```

其中 `audio_device` 要以 `arecord -l` 的结果为准。当前 Logitech C270 曾验证为：

```ini
audio_device=plughw:2,0
```

## 安装自启动服务

在树莓派项目目录执行：

```bash
cd ~/Embedded_AI
bash scripts/install-pi-service.sh
```

安装脚本会创建用户级 systemd 服务：

```text
~/.config/systemd/user/embedded-ai.service
```

服务会执行：

```text
scripts/run-pi-button-assistant.sh
```

日志会写入：

```text
logs/embedded-ai.log
```

## 查看状态

```bash
systemctl --user status embedded-ai.service
tail -f ~/Embedded_AI/logs/embedded-ai.log
```

这就是展示时可以打开的“终端输出窗口”。没有显示器时，服务也会继续写日志。

## 停止和重启

停止服务：

```bash
systemctl --user stop embedded-ai.service
```

重启服务：

```bash
systemctl --user restart embedded-ai.service
```

启用开机自启动：

```bash
systemctl --user enable embedded-ai.service
```

## 卸载服务

```bash
cd ~/Embedded_AI
bash scripts/uninstall-pi-service.sh
```

卸载只删除 systemd 服务文件，不删除项目代码、配置文件、API key 或日志。

## 常见问题

### 找不到可执行文件

重新编译：

```bash
cd ~/Embedded_AI
cmake -S . -B build-pi -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=OFF
cmake --build build-pi
```

然后重新安装或重启服务：

```bash
bash scripts/install-pi-service.sh
```

### 找不到 NUCLEO 串口

检查：

```bash
ls /dev/ttyACM*
```

服务脚本会自动寻找串口，优先级如下：

```text
1. EMBEDDED_AI_PORT 环境变量
2. /dev/serial/by-id 中包含 STLink/STMicro/NUCLEO 的设备
3. /dev/ttyACM* 中排序最靠前的设备
```

如果自动识别失败，可以临时指定端口：

```bash
systemctl --user stop embedded-ai.service
EMBEDDED_AI_PORT=/dev/ttyACM1 bash scripts/run-pi-button-assistant.sh
```

### 校园网掉认证

服务可能仍会启动，但 API 调用会失败。检查网络：

```bash
ping -c 4 223.5.5.5
ping -c 4 dashscope.aliyuncs.com
```

### 进入菜单调试

先停止服务：

```bash
systemctl --user stop embedded-ai.service
```

手动运行菜单模式：

```bash
cd ~/Embedded_AI
./build-pi/hardware/pi_bridge/embedded_ai_pc_bridge /dev/ttyACM0 --qwen --menu
```

调试完成后恢复服务：

```bash
systemctl --user start embedded-ai.service
```
