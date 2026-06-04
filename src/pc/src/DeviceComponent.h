#pragma once

#include <string>

enum class DeviceHealth {
    Unknown,
    Ready,
    Failed
};

class DeviceComponent {
public:
    explicit DeviceComponent(std::string name);
    virtual ~DeviceComponent() = default;

    const std::string& name() const;
    DeviceHealth health() const;
    virtual bool selfTest() = 0;

protected:
    void setHealth(DeviceHealth health);

private:
    std::string name_;
    DeviceHealth health_ = DeviceHealth::Unknown;
};
