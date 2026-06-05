#pragma once

#include "SerialPort.h"

#include <cstdint>
#include <string>

class HardwareBridge {
public:
    explicit HardwareBridge(SerialPort& serial);

    bool ping();
    bool setLed(bool enabled);
    bool setBuzzer(bool enabled);
    bool setVibration(bool enabled);
    bool showOledText(const std::string& text);
    std::string readStatus();

private:
    bool sendAndExpect(const std::string& command, const std::string& expectedText, std::uint32_t waitMs);
    std::string sendAndRead(const std::string& command, std::uint32_t waitMs);

    SerialPort& serial_;
};
