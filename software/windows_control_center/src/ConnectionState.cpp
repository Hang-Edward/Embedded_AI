#include "ConnectionState.h"

QString healthLevelText(HealthLevel level) {
    switch (level) {
    case HealthLevel::Unknown:
        return "Unknown";
    case HealthLevel::Checking:
        return "Checking";
    case HealthLevel::Ok:
        return "Ready";
    case HealthLevel::Warning:
        return "Warning";
    case HealthLevel::Error:
        return "Error";
    }
    return "Unknown";
}

QString healthLevelClass(HealthLevel level) {
    switch (level) {
    case HealthLevel::Ok:
        return "ok";
    case HealthLevel::Warning:
        return "warn";
    case HealthLevel::Error:
        return "error";
    case HealthLevel::Checking:
        return "checking";
    case HealthLevel::Unknown:
        return "unknown";
    }
    return "unknown";
}
