#include "SceneTask.h"

std::string taskRiskToText(TaskRisk risk) {
    switch (risk) {
    case TaskRisk::Low:
        return "LOW";
    case TaskRisk::Medium:
        return "MEDIUM";
    case TaskRisk::High:
        return "HIGH";
    }
    return "UNKNOWN";
}
