#pragma once

#include "NucleoOutputDevices.h"
#include "SceneTask.h"

#include <string>

class PrototypeDeviceSet {
public:
    explicit PrototypeDeviceSet(HardwareBridge& hardware);

    bool runSelfTest();
    bool setLed(bool enabled);
    bool displayMessage(const std::string& message);
    void applyFeedback(const SceneTask& task);

private:
    NucleoLedDevice led_;
    NucleoBuzzerDevice buzzer_;
    NucleoVibrationDevice vibration_;
    NucleoOledDevice oled_;
};
