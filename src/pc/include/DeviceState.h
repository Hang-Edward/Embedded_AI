#pragma once

#include <string>

struct DeviceState {
    bool ledOn = false;
    bool buzzerOn = false;
    bool vibrationOn = false;
    std::string oledText = "READY";
};
