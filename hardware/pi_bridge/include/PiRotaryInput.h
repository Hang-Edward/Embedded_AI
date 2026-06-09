#pragma once

#include <chrono>

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
    int readPin(int bcmPin) const;
    RotaryEvent pollRotation(int a, int b);
    RotaryEvent pollButton(int pressedLevel);

    bool initialized_;
    int lastEncoded_;
    int rotationAccumulator_;
    int lastButtonLevel_;
    std::chrono::steady_clock::time_point lastPressTime_;
};
