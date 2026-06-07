#pragma once

#include "HardwareBridge.h"
#include "OutputDevice.h"

class NucleoLedDevice final : public BinaryOutputDevice {
public:
    explicit NucleoLedDevice(HardwareBridge& hardware);

    bool selfTest() override;
    bool setEnabled(bool enabled) override;

private:
    HardwareBridge& hardware_;
};

class NucleoBuzzerDevice final : public BinaryOutputDevice {
public:
    explicit NucleoBuzzerDevice(HardwareBridge& hardware);

    bool selfTest() override;
    bool setEnabled(bool enabled) override;

private:
    HardwareBridge& hardware_;
};

class NucleoVibrationDevice final : public BinaryOutputDevice {
public:
    explicit NucleoVibrationDevice(HardwareBridge& hardware);

    bool selfTest() override;
    bool setEnabled(bool enabled) override;

private:
    HardwareBridge& hardware_;
};

class NucleoOledDevice final : public TextOutputDevice {
public:
    explicit NucleoOledDevice(HardwareBridge& hardware);

    bool selfTest() override;
    bool turnOff() override;
    bool showText(const std::string& text) override;

private:
    HardwareBridge& hardware_;
};
