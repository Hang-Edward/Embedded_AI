# Raspberry Pi 自启动服务说明

本文档用于把 Embedded AI Reality Bridge 配置为树莓派开机自启动。配置完成后，树莓派通电启动并连接好 NUCLEO 与 Logitech C270 后，不需要键盘输入命令，只要按 NUCLEO 蓝色按钮即可触发：

```text
按钮事件 -> 录音 -> Qwen ASR -> 摄像头拍照 -> Qwen-VL 分析 -> 输出日志
```

如果接着显示器或通过 SSH 登录，可以实时查看日志；如果没有显示器和键盘，服务也会在后台运行。

## 1. 前置条件

在树莓派项目目录中确认：

```bash
cd ~/Embedded_AI
cmake --build build-pi
ls build-pi/src/pc/embedded_ai_pc_bridge
ls config/qwen-vision.ini config/qwen-vision.key
```

确认 NUCLEO 和摄像头设备存在：

```bash
ls /dev/ttyACM*
ls /dev/video*
arecord -l
```

`config/qwen-vision.ini` 中建议包含：

```ini
enabled=true
model=qwen3-vl-8b-instruct
asr_model=qwen3-asr-flash
audio_device=plughw:2,0
api_key_file=config/qwen-vision.key
```

其中 `audio_device` 要以 `arecord -l` 的结果为准。当前 Logitech C270 已验证为：

```ini
audio_device=plughw:2,0
```

## 2. 安装自启动服务

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

## 3. 查看运行状态

```bash
systemctl --user status embedded-ai.service
```

实时查看程序输出：

```bash
tail -f ~/Embedded_AI/logs/embedded-ai.log
```

这就是展示时可以打开的“终端输出窗口”。如果没有显示器，它也会继续写日志。

## 4. 手动停止和启动

停止服务：

```bash
systemctl --user stop embedded-ai.service
```

重新启动服务：

```bash
systemctl --user restart embedded-ai.service
```

开机自动启动已经由安装脚本启用：

```bash
systemctl --user enable embedded-ai.service
```

## 5. 卸载服务

```bash
cd ~/Embedded_AI
bash scripts/uninstall-pi-service.sh
```

卸载只删除 systemd 服务文件，不删除项目代码、配置文件、API key 或日志。

## 6. 常见问题

### 6.1 服务启动失败，提示找不到可执行文件

重新编译：

```bash
cd ~/Embedded_AI
cmake -S . -B build-pi -G Ninja -DBUILD_PC_GUI=OFF
cmake --build build-pi
```

然后重新安装或重启服务：

```bash
bash scripts/install-pi-service.sh
```

### 6.2 找不到 NUCLEO 串口

检查：

```bash
ls /dev/ttyACM*
```

默认串口是：

```text
/dev/ttyACM0
```

如果实际是 `/dev/ttyACM1`，可以临时这样启动服务：

```bash
systemctl --user stop embedded-ai.service
EMBEDDED_AI_PORT=/dev/ttyACM1 bash scripts/run-pi-button-assistant.sh
```

后续可以把 `EMBEDDED_AI_PORT` 写入服务环境变量。

### 6.3 校园网掉线

服务会启动，但 API 调用可能失败。查看日志：

```bash
tail -f ~/Embedded_AI/logs/embedded-ai.log
```

检查网络：

```bash
ping -c 4 223.5.5.5
ping -c 4 dashscope.aliyuncs.com
```

### 6.4 想进入菜单调试

先停止服务：

```bash
systemctl --user stop embedded-ai.service
```

手动运行菜单模式：

```bash
cd ~/Embedded_AI
./build-pi/src/pc/embedded_ai_pc_bridge /dev/ttyACM0 --qwen --menu
```

调试完成后恢复服务：

```bash
systemctl --user start embedded-ai.service
```
