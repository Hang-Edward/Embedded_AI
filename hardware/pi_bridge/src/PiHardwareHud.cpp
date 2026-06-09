#include "PiHardwareHud.h"

#include <cstdlib>
#include <utility>

PiHardwareHud::PiHardwareHud(std::string projectRoot)
    : projectRoot_(std::move(projectRoot)) {
}

void PiHardwareHud::showStatus(HudStatus status, const std::string& message) const {
    switch (status) {
    case HudStatus::Ready:
        runScript("ready", message);
        break;
    case HudStatus::Busy:
        runScript("busy", message);
        break;
    case HudStatus::Error:
        runScript("error", message);
        break;
    }
}

void PiHardwareHud::showReply(const std::string& reply) const {
    runScript("reply", reply);
}

void PiHardwareHud::showError(const std::string& message) const {
    showStatus(HudStatus::Error, message);
}

void PiHardwareHud::showOlderReplyPage() const {
    runScript("page", "older");
}

void PiHardwareHud::showNewerReplyPage() const {
    runScript("page", "newer");
}

void PiHardwareHud::showRecordingCountdown(int seconds) const {
    runScript("recording", std::to_string(seconds), true);
}

void PiHardwareHud::runScript(const std::string& mode, const std::string& value, bool async) const {
#ifdef __linux__
    const std::string script = projectRoot_ + "/hardware/pi_bridge/scripts/pi_hud.py";
    const std::string command = "python3 " + shellQuote(script) + " "
        + shellQuote(mode) + " " + shellQuote(value)
        + " >>/tmp/embedded-ai-hud.log 2>&1"
        + (async ? " &" : "");
    std::system(command.c_str());
#else
    (void)mode;
    (void)value;
    (void)async;
#endif
}

std::string PiHardwareHud::shellQuote(const std::string& value) {
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted += ch;
        }
    }
    quoted += "'";
    return quoted;
}
