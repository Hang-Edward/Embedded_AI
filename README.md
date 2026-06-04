# Embedded AI Reality Bridge

这是一个面向 C++ 大作业和嵌入式原型展示的项目。目标是做出一个类似 AI 智能眼镜核心能力的桌面原型：摄像头读取画面，语音或按键触发提问，PC 端 C++ 程序调用视觉/多模态 API，NUCLEO-F446RE 负责 LED、蜂鸣器、OLED、震动等硬件反馈。

当前阶段优先实现：

- Windows 电脑运行 C++ 主程序、网页展示层和 API 接入。
- Logitech C270 摄像头作为画面输入。
- NUCLEO-F446RE 通过 USB 虚拟串口和电脑通信。
- STM32 固件先完成最小 `PING` / `PONG` 串口测试，再逐步扩展硬件模块。

## Project Structure

```text
Embedded_AI/
├─ README.md
├─ server.js
├─ .gitignore
├─ assets/
│  ├─ diagrams/
│  └─ images/
├─ docs/
│  ├─ project-specification.pdf
│  └─ project-specification-extracted.txt
└─ src/
   ├─ firmware/
   │  └─ nucleo_pingpong/
   │     ├─ main.c
   │     ├─ linker.ld
   │     └─ build/              # ignored
   └─ web/
      ├─ index.html
      ├─ product.html
      ├─ guide.html
      ├─ styles.css
      ├─ script.js
      └─ product.js
```

## Local Preview

```powershell
cd "D:\VScode Projects\Embedded_AI"
node server.js
```

然后访问：

```text
http://127.0.0.1:8765/
```

如果端口被占用：

```powershell
$env:PORT=8766
node server.js
```

## Pages

- `src/web/index.html`：AI + C++ 嵌入式控制台 Demo。
- `src/web/product.html`：AI Reality Bridge 产品原型展示。
- `src/web/guide.html`：采购清单、硬件方案、开发步骤和答辩路线。

## Firmware Test

当前测试固件位于：

```text
src/firmware/nucleo_pingpong/
```

已验证功能：

- NUCLEO-F446RE 可通过 ST-LINK 正常烧录。
- `COM11 @ 115200` 可读取启动文本。
- 发送 `PING` 返回 `PONG`。
- 发送 `LEDON` / `LEDOFF` 可控制板载 LED 并返回确认文本。
