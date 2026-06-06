#include "HardwareBridge.h"

#include <algorithm>
#include <cctype>

namespace {

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

std::string toUpperAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

bool statusShowsLed(const std::string& status, bool enabled) {
    const std::string normalized = toUpperAscii(status);
    return contains(normalized, enabled ? "LED=ON" : "LED=OFF");
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
    if (sendAndExpect(enabled ? "LED:ON" : "LED:OFF", enabled ? "OK LED ON" : "OK LED OFF", 800)) {
        return true;
    }

    // 中文注释：如果串口回复格式不同或混入启动信息，用状态回读确认真实 LED 状态。
    return statusShowsLed(readStatus(), enabled);
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

std::string HardwareBridge::readEvents(std::uint32_t waitMs) {
    return serial_.readAvailable(waitMs);
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
