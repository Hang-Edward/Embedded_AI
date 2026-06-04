#pragma once

#include "DeviceComponent.h"

class OutputDevice : public DeviceComponent {
public:
    using DeviceComponent::DeviceComponent;

    virtual bool turnOff() = 0;
};

class BinaryOutputDevice : public OutputDevice {
public:
    using OutputDevice::OutputDevice;

    virtual bool setEnabled(bool enabled) = 0;
    bool turnOff() override;
};

class TextOutputDevice : public OutputDevice {
public:
    using OutputDevice::OutputDevice;

    virtual bool showText(const std::string& text) = 0;
};
