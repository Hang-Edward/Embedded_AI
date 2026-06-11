# 树莓派 LCD、红绿灯与旋钮接线指南

本文档用于把 1.8 寸 `128x160 RGB TFT LCD`、红绿灯 LED 模块和 5 针旋钮模块接到 Raspberry Pi 5。

目标效果：

- LCD 显示最新 AI 回复，标题为 `AI REPLY 0`。
- 旋钮左转/右转可以查看最近 10 次回复，历史页显示为 `AI REPLY -1` 到 `AI REPLY -9`。
- NUCLEO 蓝色按钮开始 5 秒语音输入；当前旋钮模块只负责翻阅历史。
- 红绿灯状态：
  - 绿灯：系统就绪，可以按旋钮。
  - 黄灯：录音、语音识别、拍照或 AI 响应中。
  - 红灯：故障，例如摄像头失败、AI 响应失败、录音失败等。

## 0. 安全规则

1. 接线前先关闭树莓派电源，拔掉 USB-C 供电线。
2. 本方案全部使用树莓派 `3.3V` 逻辑，LCD 和旋钮的 `VCC/UCC` 不要接 `5V`。
3. 所有模块的 `GND` 必须接到树莓派 `GND`。
4. LCD 顶部 8 个孔如果没有焊排针，需要先焊好排针，否则杜邦线接触会非常不稳定。
5. 红绿灯模块通常已经自带限流电阻，不需要再额外串电阻。如果灯明显发热，立刻断电检查。

## 1. 树莓派 40-pin 方向

让树莓派 USB 口和网口朝外侧，40-pin 排针在你面前时，常用物理针脚如下：

```text
左列奇数                         右列偶数
Pin 1  3.3V                       Pin 2  5V
Pin 3  GPIO2                      Pin 4  5V
Pin 5  GPIO3                      Pin 6  GND
Pin 7  GPIO4                      Pin 8  GPIO14
Pin 9  GND                        Pin10  GPIO15
Pin11  GPIO17                     Pin12  GPIO18
Pin13  GPIO27                     Pin14  GND
Pin15  GPIO22                     Pin16  GPIO23
Pin17  3.3V                       Pin18  GPIO24
Pin19  GPIO10 MOSI                Pin20  GND
Pin21  GPIO9 MISO                 Pin22  GPIO25
Pin23  GPIO11 SCLK                Pin24  GPIO8 CE0
Pin25  GND                        Pin26  GPIO7 CE1
Pin29  GPIO5                      Pin31  GPIO6
Pin36  GPIO16
```

## 2. LCD 接线表

你的 LCD 顶部丝印是：

```text
GND VCC SCL SDA RES DC CS BL
```

按下面接到树莓派：

| LCD 引脚 | 接到树莓派物理针脚 | 树莓派 BCM | 作用 |
|---|---:|---:|---|
| `GND` | Pin 6 | GND | 地线 |
| `VCC` | Pin 1 | 3.3V | 屏幕供电 |
| `SCL` | Pin 23 | GPIO11 | SPI 时钟 SCLK |
| `SDA` | Pin 19 | GPIO10 | SPI 数据 MOSI |
| `RES` | Pin 22 | GPIO25 | LCD 复位 |
| `DC` | Pin 18 | GPIO24 | 命令/数据选择 |
| `CS` | Pin 24 | GPIO8 | SPI 片选 CE0 |
| `BL` | Pin 12 | GPIO18 | 背光控制 |

注意：这个屏幕虽然写着 `SCL/SDA`，但这里按 SPI 接，不是 I2C。`SDA` 接 MOSI，`SCL` 接 SCLK。

## 3. 红绿灯模块接线表

你的红绿灯模块底部丝印是：

```text
G Y R GND
```

| 红绿灯引脚 | 接到树莓派物理针脚 | 树莓派 BCM | 状态 |
|---|---:|---:|---|
| `G` | Pin 11 | GPIO17 | 绿灯，系统就绪 |
| `Y` | Pin 13 | GPIO27 | 黄灯，AI 处理中 |
| `R` | Pin 15 | GPIO22 | 红灯，故障 |
| `GND` | Pin 9 | GND | 地线 |

如果测试时颜色不对应，只交换 `G/Y/R` 三根信号线，不要动 `GND`。

## 4. 旋钮模块接线表

你的旋钮模块 5 个针脚是：

```text
GND UCC A C B
```

根据模块 PCB、官方功能说明与实测结果：`A/B` 是两路正交旋转输出，`C` 是旋转编码器公共端，`UCC`（即 `VCC`）为模块供电。这一型号没有独立的按键输出；旋钮轴可以产生轴向活动，但不能作为电子按键使用。

| 旋钮引脚 | 接到树莓派物理针脚 | 树莓派 BCM | 作用 |
|---|---:|---:|---|
| `GND` | Pin 14 | GND | 模块公共地 |
| `UCC` | Pin 17 | 3.3V | 模块供电，不要接 5V |
| `A` | Pin 29 | GPIO5 | 旋转信号 A |
| `B` | Pin 31 | GPIO6 | 旋转信号 B |
| `C` | Pin 20 | GND | 旋转编码器公共端 |

旋转方向规则：

- 逆时针/向左：查看更旧回复，`AI REPLY -1`、`AI REPLY -2`，直到 `AI REPLY -9`。
- 顺时针/向右：查看更新回复，最终回到 `AI REPLY 0`。
- 按下 NUCLEO 蓝色按钮：开始 5 秒语音输入。

如果左右方向反了，把旋钮的 `A` 和 `B` 两根线互换即可。

不要再把 `C` 接到 Pin 36/GPIO16，否则 GPIO16 会一直读到低电平。若需要旋钮按压触发，必须更换带独立 `SW`/`KEY` 引脚的编码器模块，或另接一个按钮模块。

## 5. 接线示意图

```mermaid
flowchart LR
    Pi["Raspberry Pi 5<br/>40-pin GPIO"]:::pi
    LCD["1.8 TFT LCD<br/>GND VCC SCL SDA RES DC CS BL"]:::lcd
    LED["红绿灯模块<br/>G Y R GND"]:::led
    Knob["旋钮模块<br/>GND UCC A C B"]:::knob
    Nucleo["NUCLEO F446RE<br/>USB 串口备用触发"]:::nucleo

    Pi -- "SPI0: GPIO10/11/8<br/>GPIO25/24/18<br/>3.3V + GND" --> LCD
    Pi -- "GPIO17/27/22 + GND" --> LED
    Pi -- "GPIO5/6/16 + 3.3V + GND" --> Knob
    Pi -- "USB 串口读取备用按钮事件" --> Nucleo

    classDef pi fill:#172554,stroke:#60a5fa,color:#ffffff;
    classDef lcd fill:#0f766e,stroke:#5eead4,color:#ffffff;
    classDef led fill:#7f1d1d,stroke:#facc15,color:#ffffff;
    classDef knob fill:#78350f,stroke:#fbbf24,color:#ffffff;
    classDef nucleo fill:#312e81,stroke:#a5b4fc,color:#ffffff;
```

## 6. 上电测试

接好线后，在树莓派终端执行：

```bash
cd ~/Embedded_AI
bash scripts/setup-pi-hud.sh
```

这个脚本会安装依赖、开启 SPI，并测试 LCD 与红绿灯。更新代码后执行：

```bash
cd ~/Embedded_AI
git pull
cmake --build build-pi
systemctl --user restart embedded-ai.service
tail -f logs/embedded-ai.log
```

预期现象：

1. 服务启动后绿灯亮，LCD 显示系统就绪。
2. 按下 NUCLEO 蓝色按钮后黄灯亮，LCD 显示录音 5 秒倒计时。
3. 录音结束后 LCD 显示 `AI 响应中`。
4. AI 完成后绿灯亮，LCD 显示 `AI REPLY 0` 和最新回复摘要。
5. 左右旋转旋钮可以查看 `AI REPLY -1` 到 `AI REPLY -9`。
6. 如果摄像头、录音或 AI 请求失败，红灯亮，LCD 显示具体错误。
