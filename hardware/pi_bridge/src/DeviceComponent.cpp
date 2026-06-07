#include "DeviceComponent.h"

#include <utility>

DeviceComponent::DeviceComponent(std::string name)
    : name_(std::move(name)) {
}

const std::string& DeviceComponent::name() const {
    return name_;
}

DeviceHealth DeviceComponent::health() const {
    return health_;
}

void DeviceComponent::setHealth(DeviceHealth health) {
    health_ = health;
}
