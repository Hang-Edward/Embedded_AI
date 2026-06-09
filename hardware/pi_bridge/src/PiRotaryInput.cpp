#include "PiRotaryInput.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

constexpr int PinA = 5;
constexpr int PinB = 6;
constexpr int PinButton = 16;
constexpr int RotationStepsPerDetent = 4;
constexpr auto PressDebounce = std::chrono::milliseconds(350);

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

} // namespace

PiRotaryInput::PiRotaryInput()
    : initialized_(false),
      lastEncoded_(0),
      rotationAccumulator_(0),
      lastButtonLevel_(1),
      lastPressTime_(std::chrono::steady_clock::now() - PressDebounce) {
#ifdef __linux__
    std::system("pinctrl set 5 ip pu >/dev/null 2>&1");
    std::system("pinctrl set 6 ip pu >/dev/null 2>&1");
    std::system("pinctrl set 16 ip pu >/dev/null 2>&1");
#endif
}

RotaryEvent PiRotaryInput::poll() {
    const int a = readPin(PinA);
    const int b = readPin(PinB);
    const int button = readPin(PinButton);
    if (a < 0 || b < 0 || button < 0) {
        return RotaryEvent::None;
    }

    if (!initialized_) {
        lastEncoded_ = (a << 1) | b;
        lastButtonLevel_ = button;
        initialized_ = true;
        return RotaryEvent::None;
    }

    const RotaryEvent buttonEvent = pollButton(button);
    if (buttonEvent != RotaryEvent::None) {
        return buttonEvent;
    }

    return pollRotation(a, b);
}

int PiRotaryInput::readPin(int bcmPin) const {
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

RotaryEvent PiRotaryInput::pollButton(int buttonLevel) {
    // 中文注释：常见旋钮按键为上拉输入，按下时 GPIO 变为低电平。
    const bool pressedNow = buttonLevel == 0 && lastButtonLevel_ == 1;
    lastButtonLevel_ = buttonLevel;
    if (!pressedNow) {
        return RotaryEvent::None;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - lastPressTime_ < PressDebounce) {
        return RotaryEvent::None;
    }
    lastPressTime_ = now;
    return RotaryEvent::Pressed;
}
