# Embedded AI Reality Bridge

这是一个面向 C++ 大作业和嵌入式原型展示的项目。目标是做出一个类似 AI 智能眼镜核心能力的桌面原型：摄像头读取画面，按键或语音触发提问，PC 端 C++ 程序调用视觉/多模态 API，NUCLEO-F446RE 负责 LED、蜂鸣器、OLED、震动等硬件反馈。

当前阶段优先实现电脑连接原型机：

- Windows 电脑运行 C++ 主程序、网页展示层和后续 API 接入。
- Logitech C270 或电脑内置摄像头作为画面输入。
- NUCLEO-F446RE 通过 USB 虚拟串口和电脑通信。
- STM32 固件使用 C++ 编写，提供 `PING`、LED、蜂鸣器、震动、OLED 文本等串口命令。

## Project Structure

```text
Embedded_AI/
  README.md
  CMakeLists.txt
  server.js
  assets/
    diagrams/
    images/
  docs/
    project-specification.pdf
    project-specification-extracted.txt
  src/
    firmware/
      nucleo_pingpong/
        main.cpp
        linker.ld
    pc/
      CMakeLists.txt
      README.md
      src/
    web/
      index.html
      product.html
      guide.html
      styles.css
      script.js
      product.js
```

## PC Prototype

PC 端程序位于 `src/pc`，目前已经实现：

- 控制台 UI。
- OpenCV 摄像头抓拍。
- mock 视觉 AI 分析。
- Qwen 视觉 API 配置、图片 base64 编码、HTTP 调用和响应解析。
- 默认视觉模型：`qwen3-vl-8b-instruct`。
- NUCLEO 串口控制。
- 二进制审计日志。
- 按记录 ID 的随机文件读取和更新。

构建和运行命令见 `src/pc/README.md`。

## Web Preview

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
- `COM11 @ 115200` 可通信。
- 发送 `PING` 返回 `PONG`。
- 发送 `LED:ON` / `LED:OFF` 可控制板载 LED。
- 发送 `STATUS?` 可读取固件侧状态。

## Roadmap

- 增加更细的 API 错误提示和费用/调用次数保护。
- 增加语音输入或按键触发。
- 把 mock 视觉分析替换为真实图像理解。
- 接入真实 OLED、蜂鸣器和震动模块。
- 继续扩展 C++ 面向对象结构和作业要求覆盖。
