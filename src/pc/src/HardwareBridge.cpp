#include "HardwareBridge.h"

#include <algorithm>

namespace {

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

std::string sanitizeOledText(std::string text) {
    // 中文注释：串口协议按行读取，OLED 文本中不能携带换行符。
    std::replace(text.begin(), text.end(), '\r', ' ');
    std::replace(text.begin(), text.end(), '\n', ' ');
    if (text.size() > 40) {
        text.resize(40);
    }
    return text;
}

} // namespace

HardwareBridge::HardwareBridge(SerialPort& serial)
    : serial_(serial) {
}

bool HardwareBridge::ping() {
    return sendAndExpect("PING", "PONG", 1200);
}

bool HardwareBridge::setLed(bool enabled) {
    return sendAndExpect(enabled ? "LED:ON" : "LED:OFF", enabled ? "OK LED ON" : "OK LED OFF", 800);
}

bool HardwareBridge::setBuzzer(bool enabled) {
    return sendAndExpect(enabled ? "BUZZER:ON" : "BUZZER:OFF", enabled ? "OK BUZZER ON" : "OK BUZZER OFF", 800);
}

bool HardwareBridge::setVibration(bool enabled) {
    return sendAndExpect(enabled ? "VIB:ON" : "VIB:OFF", enabled ? "OK VIB ON" : "OK VIB OFF", 800);
}

bool HardwareBridge::showOledText(const std::string& text) {
    return sendAndExpect("OLED:TEXT=" + sanitizeOledText(text), "OK OLED TEXT", 800);
}

std::string HardwareBridge::readStatus() {
    return sendAndRead("STATUS?", 1000);
}

bool HardwareBridge::sendAndExpect(const std::string& command, const std::string& expectedText, std::uint32_t waitMs) {
    return contains(sendAndRead(command, waitMs), expectedText);
}

std::string HardwareBridge::sendAndRead(const std::string& command, std::uint32_t waitMs) {
    if (!serial_.writeLine(command)) {
        return {};
    }
    return serial_.readAvailable(waitMs);
}
