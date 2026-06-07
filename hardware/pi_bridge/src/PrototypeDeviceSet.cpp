#include "PrototypeDeviceSet.h"

PrototypeDeviceSet::PrototypeDeviceSet(HardwareBridge& hardware)
    : led_(hardware), buzzer_(hardware), vibration_(hardware), oled_(hardware) {
}

bool PrototypeDeviceSet::runSelfTest() {
    const bool ledOk = led_.selfTest();
    const bool buzzerOk = buzzer_.selfTest();
    const bool vibrationOk = vibration_.selfTest();
    const bool oledOk = oled_.selfTest();
    return ledOk && buzzerOk && vibrationOk && oledOk;
}

bool PrototypeDeviceSet::setLed(bool enabled) {
    return led_.setEnabled(enabled);
}

bool PrototypeDeviceSet::displayMessage(const std::string& message) {
    return oled_.showText(message);
}

void PrototypeDeviceSet::applyFeedback(const SceneTask& task) {
    switch (task.risk) {
    case TaskRisk::Low:
        led_.setEnabled(false);
        buzzer_.setEnabled(false);
        vibration_.setEnabled(false);
        oled_.showText("SCENE OK");
        break;
    case TaskRisk::Medium:
        led_.setEnabled(true);
        buzzer_.setEnabled(false);
        vibration_.setEnabled(false);
        oled_.showText("HINT READY");
        break;
    case TaskRisk::High:
        led_.setEnabled(true);
        buzzer_.setEnabled(true);
        vibration_.setEnabled(true);
        oled_.showText("RISK ALERT");
        break;
    }
}
