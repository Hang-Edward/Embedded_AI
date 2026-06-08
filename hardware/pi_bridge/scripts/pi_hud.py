#!/usr/bin/env python3
"""Raspberry Pi GPIO/TFT HUD for the Embedded AI prototype.

Pins use BCM numbering:
- LED module: G=GPIO17, Y=GPIO27, R=GPIO22, GND=GND
- ST7735 TFT: SPI0 MOSI/SCLK/CE0 plus RES=GPIO25, DC=GPIO24, BL=GPIO18
"""

from __future__ import annotations

import os
import subprocess
import sys
import textwrap
import time
from dataclasses import dataclass

WIDTH = 128
HEIGHT = 160

LED_GREEN = 17
LED_YELLOW = 27
LED_RED = 22

LCD_RST = 25
LCD_DC = 24
LCD_BL = 18
LCD_SPI_BUS = 0
LCD_SPI_DEVICE = 0
LCD_SPI_HZ = 16_000_000


def pinctrl(pin: int, level: bool) -> None:
    subprocess.run(
        ["pinctrl", "set", str(pin), "op", "dh" if level else "dl"],
        check=False,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def set_led(status: str) -> None:
    pinctrl(LED_GREEN, status == "ready")
    pinctrl(LED_YELLOW, status == "busy")
    pinctrl(LED_RED, status == "error")


def rgb565(r: int, g: int, b: int) -> bytes:
    value = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
    return bytes([(value >> 8) & 0xFF, value & 0xFF])


@dataclass
class Theme:
    bg: tuple[int, int, int]
    accent: tuple[int, int, int]
    title: str


THEMES = {
    "ready": Theme((6, 35, 22), (34, 197, 94), "READY"),
    "busy": Theme((48, 34, 8), (245, 158, 11), "AI BUSY"),
    "error": Theme((55, 16, 22), (239, 68, 68), "ERROR"),
    "reply": Theme((7, 18, 35), (59, 130, 246), "AI REPLY"),
}


class ST7735:
    def __init__(self) -> None:
        import spidev  # type: ignore

        self.spi = spidev.SpiDev()
        self.spi.open(LCD_SPI_BUS, LCD_SPI_DEVICE)
        self.spi.max_speed_hz = LCD_SPI_HZ
        self.spi.mode = 0

    def command(self, value: int, data: list[int] | bytes = b"") -> None:
        pinctrl(LCD_DC, False)
        self.spi.writebytes([value])
        if data:
            pinctrl(LCD_DC, True)
            self.spi.writebytes(list(data))

    def init(self) -> None:
        pinctrl(LCD_BL, True)
        pinctrl(LCD_RST, True)
        time.sleep(0.05)
        pinctrl(LCD_RST, False)
        time.sleep(0.05)
        pinctrl(LCD_RST, True)
        time.sleep(0.12)

        self.command(0x01)
        time.sleep(0.12)
        self.command(0x11)
        time.sleep(0.12)
        self.command(0x3A, [0x05])
        self.command(0x36, [0xC8])
        self.command(0x29)
        time.sleep(0.05)

    def set_window(self, x0: int, y0: int, x1: int, y1: int) -> None:
        self.command(0x2A, [0, x0, 0, x1])
        self.command(0x2B, [0, y0, 0, y1])
        self.command(0x2C)

    def show_rgb565(self, payload: bytes) -> None:
        self.set_window(0, 0, WIDTH - 1, HEIGHT - 1)
        pinctrl(LCD_DC, True)
        chunk = 4096
        for offset in range(0, len(payload), chunk):
            self.spi.writebytes(list(payload[offset : offset + chunk]))


def draw_screen(status: str, text: str) -> None:
    from PIL import Image, ImageDraw, ImageFont  # type: ignore

    theme = THEMES.get(status, THEMES["reply"])
    img = Image.new("RGB", (WIDTH, HEIGHT), theme.bg)
    draw = ImageDraw.Draw(img)

    try:
        title_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16)
        body_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 12)
    except OSError:
        title_font = ImageFont.load_default()
        body_font = ImageFont.load_default()

    draw.rounded_rectangle((4, 4, WIDTH - 5, 28), radius=4, fill=theme.accent)
    draw.text((10, 8), theme.title, fill=(255, 255, 255), font=title_font)

    clean = " ".join(text.replace("\n", " ").split())
    if not clean:
        clean = "等待蓝色按钮触发"
    wrapped = textwrap.wrap(clean, width=13)[:9]
    y = 38
    for line in wrapped:
        draw.text((8, y), line, fill=(235, 245, 255), font=body_font)
        y += 13

    pixels = bytearray()
    for r, g, b in img.getdata():
        pixels.extend(rgb565(r, g, b))

    lcd = ST7735()
    lcd.init()
    lcd.show_rgb565(bytes(pixels))


def main() -> int:
    mode = sys.argv[1] if len(sys.argv) > 1 else "ready"
    text = sys.argv[2] if len(sys.argv) > 2 else ""

    if mode == "reply":
        set_led("ready")
        draw_screen("reply", text)
    elif mode in {"ready", "busy", "error"}:
        set_led(mode)
        draw_screen(mode, text)
    else:
        set_led("error")
        draw_screen("error", f"Unknown HUD mode: {mode}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # Keep the main C++ assistant alive even if HUD hardware is missing.
        os.makedirs("/tmp", exist_ok=True)
        with open("/tmp/embedded-ai-hud.log", "a", encoding="utf-8") as log:
            log.write(f"{time.strftime('%F %T')} HUD error: {exc}\n")
        raise SystemExit(0)
