#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "[1/4] Installing Raspberry Pi HUD dependencies..."
sudo apt update
sudo apt install -y python3-pil python3-spidev raspi-utils fonts-noto-cjk

echo "[2/4] Enabling SPI..."
sudo raspi-config nonint do_spi 0 || true

echo "[3/4] Testing traffic-light LEDs..."
python3 hardware/pi_bridge/scripts/pi_hud.py ready "绿色：系统就绪"
sleep 1
python3 hardware/pi_bridge/scripts/pi_hud.py busy "黄色：AI 正在处理"
sleep 1
python3 hardware/pi_bridge/scripts/pi_hud.py error "红色：故障提示"
sleep 1

echo "[4/4] Testing LCD reply output..."
python3 hardware/pi_bridge/scripts/pi_hud.py reply "这是一条 LCD AI 回复显示测试。"

echo
echo "HUD setup finished. If the LCD remains blank, reboot the Raspberry Pi once:"
echo "  sudo reboot"
echo "If a hardware error occurred, inspect:"
echo "  cat /tmp/embedded-ai-hud.log"
