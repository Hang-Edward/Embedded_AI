#!/usr/bin/env python3
"""Raspberry Pi GPIO/TFT HUD for the Embedded AI prototype.

BCM pins:
- Traffic light: G=GPIO17, Y=GPIO27, R=GPIO22, GND=GND
- ST7735 TFT: SPI0 MOSI/SCLK/CE0 plus RES=GPIO25, DC=GPIO24, BL=GPIO18
"""

from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path

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

STATE_FILE = Path("/tmp/embedded-ai-hud-state.json")
MAX_HISTORY = 10


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
        for offset in range(0, len(payload), 4096):
            self.spi.writebytes(list(payload[offset : offset + 4096]))


def load_state() -> dict:
    if not STATE_FILE.exists():
        return {"history": [], "page": 0}
    try:
        data = json.loads(STATE_FILE.read_text(encoding="utf-8"))
        if not isinstance(data, dict):
            raise ValueError("state is not an object")
        data.setdefault("history", [])
        data.setdefault("page", 0)
        return data
    except Exception:
        return {"history": [], "page": 0}


def save_state(state: dict) -> None:
    STATE_FILE.write_text(json.dumps(state, ensure_ascii=False), encoding="utf-8")


def load_font(size: int, bold: bool = False):
    from PIL import ImageFont  # type: ignore

    candidates = [
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc" if bold else "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    ]
    for path in candidates:
        try:
            return ImageFont.truetype(path, size)
        except OSError:
            continue
    return ImageFont.load_default()


def clean_text(text: str) -> str:
    cleaned = " ".join(text.replace("\r", " ").replace("\n", " ").split())
    return cleaned.split(" Source image:", 1)[0].strip()


def compact_reply(text: str) -> str:
    cleaned = clean_text(text)
    if not cleaned:
        return "暂时没有 AI 回复。"

    # 中文注释：小屏只做展示摘要，完整回答仍保存在日志和 Windows 客户端。
    sentences = [part.strip() for part in re.split(r"(?<=[。！？!?])", cleaned) if part.strip()]
    if sentences:
        cleaned = "".join(sentences[:2])
    max_chars = 76
    return cleaned[:max_chars] + ("..." if len(cleaned) > max_chars else "")


def compact_status(text: str, fallback: str) -> str:
    cleaned = clean_text(text) or fallback
    max_chars = 62
    return cleaned[:max_chars] + ("..." if len(cleaned) > max_chars else "")


def text_width(draw, text: str, font) -> int:
    if not text:
        return 0
    bbox = draw.textbbox((0, 0), text, font=font)
    return bbox[2] - bbox[0]


def truncate_to_width(draw, text: str, font, max_width: int) -> str:
    suffix = "..."
    while text and text_width(draw, text + suffix, font) > max_width:
        text = text[:-1]
    return text + suffix if text else suffix


def wrap_by_pixels(draw, text: str, font, max_width: int, max_lines: int) -> list[str]:
    lines: list[str] = []
    current = ""
    for char in text:
        trial = current + char
        if text_width(draw, trial, font) <= max_width:
            current = trial
            continue
        if current:
            lines.append(current)
        current = char
        if len(lines) >= max_lines:
            lines[-1] = truncate_to_width(draw, lines[-1], font, max_width)
            return lines
    if current:
        lines.append(current)
    if len(lines) > max_lines:
        lines = lines[:max_lines]
        lines[-1] = truncate_to_width(draw, lines[-1], font, max_width)
    return lines


def draw_screen(status: str, text: str, page: int | None = None) -> None:
    from PIL import Image, ImageDraw  # type: ignore

    theme = THEMES.get(status, THEMES["reply"])
    img = Image.new("RGB", (WIDTH, HEIGHT), theme.bg)
    draw = ImageDraw.Draw(img)
    title_font = load_font(12, bold=True)
    body_font = load_font(10 if status == "reply" else 11)

    title = theme.title if page is None else f"{theme.title} {page}"
    draw.rounded_rectangle((4, 4, WIDTH - 5, 24), radius=4, fill=theme.accent)
    draw.text((8, 6), title, fill=(255, 255, 255), font=title_font)

    if status == "reply":
        body = compact_reply(text)
    elif status == "ready":
        body = compact_status(text, "系统就绪，可以按触发按键开始语音输入。")
    elif status == "busy":
        body = compact_status(text, "AI 正在处理中，请稍等。")
    else:
        body = compact_status(text, "系统出现故障，请查看日志。")

    wrapped = wrap_by_pixels(draw, body, body_font, max_width=112, max_lines=9)
    y = 33
    for line in wrapped:
        draw.text((8, y), line, fill=(235, 245, 255), font=body_font)
        y += 13

    pixels = bytearray()
    for r, g, b in img.getdata():
        pixels.extend(rgb565(r, g, b))

    lcd = ST7735()
    lcd.init()
    lcd.show_rgb565(bytes(pixels))


def show_reply(text: str) -> None:
    set_led("ready")
    state = load_state()
    history = [text] + [item for item in state.get("history", []) if item != text]
    state["history"] = history[:MAX_HISTORY]
    state["page"] = 0
    save_state(state)
    draw_screen("reply", state["history"][0], page=0)


def show_page(direction: str) -> None:
    state = load_state()
    history = state.get("history", [])
    if not history:
        draw_screen("ready", "暂无 AI 回复历史。")
        return

    page = int(state.get("page", 0))
    if direction == "older":
        page += 1
    elif direction == "newer":
        page -= 1
    else:
        page += int(direction or "0")

    page = max(0, min(len(history) - 1, page))
    state["page"] = page
    save_state(state)
    draw_screen("reply", history[page], page=-page)


def show_recording_countdown(seconds: int) -> None:
    set_led("busy")
    seconds = max(1, min(seconds, 15))
    for remaining in range(seconds, 0, -1):
        draw_screen("busy", f"录音中：还剩 {remaining} 秒，请对着 Logitech C270 说话。")
        time.sleep(1)


def main() -> int:
    mode = sys.argv[1] if len(sys.argv) > 1 else "ready"
    text = sys.argv[2] if len(sys.argv) > 2 else ""

    if mode == "reply":
        show_reply(text)
    elif mode == "page":
        show_page(text)
    elif mode == "recording":
        show_recording_countdown(int(text or "5"))
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
    except Exception as exc:
        os.makedirs("/tmp", exist_ok=True)
        with open("/tmp/embedded-ai-hud.log", "a", encoding="utf-8") as log:
            log.write(f"{time.strftime('%F %T')} HUD error: {exc}\n")
        raise SystemExit(0)
