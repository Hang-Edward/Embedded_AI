#include "PiRotaryInput.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace {

constexpr int PinA = 5;
constexpr int PinB = 6;
// 中文注释：常见 EC11 每个机械卡点可能只暴露两个稳定相位，使用 2 能避免漏转。
constexpr int RotationStepsPerDetent = 2;

bool isClockwiseTransition(int transition) {
    return transition == 0b1101
        || transition == 0b0100
        || transition == 0b0010
        || transition == 0b1011;
}

bool isCounterClockwiseTransition(int transition) {
    return transition == 0b1110
        || transition == 0b0111
        || transition == 0b0001
        || transition == 0b1000;
}

std::string readSmallFile(const std::string& path) {
    std::ifstream input(path);
    std::ostringstream content;
    content << input.rdbuf();
    return content.str();
}

bool writeSmallFile(const std::string& path, const std::string& value) {
    std::ofstream output(path);
    if (!output) {
        return false;
    }
    output << value;
    return static_cast<bool>(output);
}

} // namespace

PiRotaryInput::PiRotaryInput()
    : initialized_(false),
      linuxGpioInitialized_(false),
      linuxGpioBase_(-1),
      lastEncoded_(0),
      rotationAccumulator_(0) {
    initializeLinuxGpio();
}

RotaryEvent PiRotaryInput::poll() {
    const int a = readPin(PinA);
    const int b = readPin(PinB);
    if (a < 0 || b < 0) {
        return RotaryEvent::None;
    }

    if (!initialized_) {
        lastEncoded_ = (a << 1) | b;
        initialized_ = true;
        return RotaryEvent::None;
    }

    return pollRotation(a, b);
}

void PiRotaryInput::initializeLinuxGpio() {
#ifdef __linux__
    std::system("pinctrl set 5 ip pu >/dev/null 2>&1");
    std::system("pinctrl set 6 ip pu >/dev/null 2>&1");

    for (int chip = 0; chip < 700; ++chip) {
        const std::string basePath = "/sys/class/gpio/gpiochip" + std::to_string(chip);
        const std::string label = readSmallFile(basePath + "/label");
        if (label.find("pinctrl-rp1") == std::string::npos) {
            continue;
        }
        const std::string baseText = readSmallFile(basePath + "/base");
        try {
            linuxGpioBase_ = std::stoi(baseText);
        } catch (...) {
            linuxGpioBase_ = -1;
        }
        break;
    }

    if (linuxGpioBase_ < 0) {
        return;
    }

    for (int pin : {PinA, PinB}) {
        const int sysfsPin = linuxGpioBase_ + pin;
        const std::string gpioPath = "/sys/class/gpio/gpio" + std::to_string(sysfsPin);
        if (readSmallFile(gpioPath + "/value").empty()) {
            writeSmallFile("/sys/class/gpio/export", std::to_string(sysfsPin));
        }
        writeSmallFile(gpioPath + "/direction", "in");
    }
    linuxGpioInitialized_ = true;
#endif
}

int PiRotaryInput::readPin(int bcmPin) const {
#ifdef __linux__
    const int sysfsValue = readPinFromSysfs(bcmPin);
    if (sysfsValue >= 0) {
        return sysfsValue;
    }
    return readPinFromPinctrl(bcmPin);
#else
    (void)bcmPin;
    return -1;
#endif
}

int PiRotaryInput::readPinFromSysfs(int bcmPin) const {
#ifdef __linux__
    if (!linuxGpioInitialized_ || linuxGpioBase_ < 0) {
        return -1;
    }
    const int sysfsPin = linuxGpioBase_ + bcmPin;
    const std::string value = readSmallFile("/sys/class/gpio/gpio" + std::to_string(sysfsPin) + "/value");
    if (value.empty()) {
        return -1;
    }
    return value[0] == '0' ? 0 : 1;
#else
    (void)bcmPin;
    return -1;
#endif
}

int PiRotaryInput::readPinFromPinctrl(int bcmPin) const {
#ifdef __linux__
    const std::string command = "pinctrl get " + std::to_string(bcmPin) + " 2>/dev/null";
    std::array<char, 160> buffer{};
    std::string output;

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        return -1;
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output += buffer.data();
    }
    pclose(pipe);

    if (output.find(" hi") != std::string::npos || output.find("|hi") != std::string::npos) {
        return 1;
    }
    if (output.find(" lo") != std::string::npos || output.find("|lo") != std::string::npos) {
        return 0;
    }
#else
    (void)bcmPin;
#endif
    return -1;
}

RotaryEvent PiRotaryInput::pollRotation(int a, int b) {
    const int encoded = (a << 1) | b;
    if (encoded == lastEncoded_) {
        return RotaryEvent::None;
    }

    const int transition = (lastEncoded_ << 2) | encoded;
    lastEncoded_ = encoded;

    if (isClockwiseTransition(transition)) {
        ++rotationAccumulator_;
    } else if (isCounterClockwiseTransition(transition)) {
        --rotationAccumulator_;
    } else {
        rotationAccumulator_ = 0;
        return RotaryEvent::None;
    }

    if (rotationAccumulator_ >= RotationStepsPerDetent) {
        rotationAccumulator_ = 0;
        return RotaryEvent::Clockwise;
    }
    if (rotationAccumulator_ <= -RotationStepsPerDetent) {
        rotationAccumulator_ = 0;
        return RotaryEvent::CounterClockwise;
    }
    return RotaryEvent::None;
}
