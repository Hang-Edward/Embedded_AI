#pragma once

#include <chrono>
#include <string>

enum class RotaryEvent {
    None,
    Clockwise,
    CounterClockwise,
    Pressed
};

class PiRotaryInput {
public:
    PiRotaryInput();

    RotaryEvent poll();

private:
    void initializeLinuxGpio();
    int readPin(int bcmPin) const;
    int readPinFromSysfs(int bcmPin) const;
    int readPinFromPinctrl(int bcmPin) const;
    RotaryEvent pollRotation(int a, int b);

    bool initialized_;
    bool linuxGpioInitialized_;
    int linuxGpioBase_;
    int lastEncoded_;
    int rotationAccumulator_;
};
