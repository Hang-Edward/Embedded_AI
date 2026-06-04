#include "NucleoOutputDevices.h"

NucleoLedDevice::NucleoLedDevice(HardwareBridge& hardware)
    : BinaryOutputDevice("NUCLEO LED"), hardware_(hardware) {
}

bool NucleoLedDevice::selfTest() {
    const bool ok = setEnabled(false);
    setHealth(ok ? DeviceHealth::Ready : DeviceHealth::Failed);
    return ok;
}

bool NucleoLedDevice::setEnabled(bool enabled) {
    return hardware_.setLed(enabled);
}

NucleoBuzzerDevice::NucleoBuzzerDevice(HardwareBridge& hardware)
    : BinaryOutputDevice("NUCLEO Buzzer"), hardware_(hardware) {
}

bool NucleoBuzzerDevice::selfTest() {
    const bool ok = setEnabled(false);
    setHealth(ok ? DeviceHealth::Ready : DeviceHealth::Failed);
    return ok;
}

bool NucleoBuzzerDevice::setEnabled(bool enabled) {
    return hardware_.setBuzzer(enabled);
}

NucleoVibrationDevice::NucleoVibrationDevice(HardwareBridge& hardware)
    : BinaryOutputDevice("NUCLEO Vibration"), hardware_(hardware) {
}

bool NucleoVibrationDevice::selfTest() {
    const bool ok = setEnabled(false);
    setHealth(ok ? DeviceHealth::Ready : DeviceHealth::Failed);
    return ok;
}

bool NucleoVibrationDevice::setEnabled(bool enabled) {
    return hardware_.setVibration(enabled);
}

NucleoOledDevice::NucleoOledDevice(HardwareBridge& hardware)
    : TextOutputDevice("NUCLEO OLED"), hardware_(hardware) {
}

bool NucleoOledDevice::selfTest() {
    const bool ok = showText("AI Bridge Ready");
    setHealth(ok ? DeviceHealth::Ready : DeviceHealth::Failed);
    return ok;
}

bool NucleoOledDevice::turnOff() {
    return showText("");
}

bool NucleoOledDevice::showText(const std::string& text) {
    return hardware_.showOledText(text);
}
