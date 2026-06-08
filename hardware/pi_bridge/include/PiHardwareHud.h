#pragma once

#include <string>

enum class HudStatus {
    Ready,
    Busy,
    Error
};

class PiHardwareHud {
public:
    explicit PiHardwareHud(std::string projectRoot = ".");

    void showStatus(HudStatus status, const std::string& message) const;
    void showReply(const std::string& reply) const;
    void showError(const std::string& message) const;

private:
    void runScript(const std::string& mode, const std::string& value) const;
    static std::string shellQuote(const std::string& value);

    std::string projectRoot_;
};
