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

std::string taskTypeToText(TaskType type) {
    switch (type) {
    case TaskType::SceneDescription:
        return "SCENE_DESCRIPTION";
    case TaskType::ProblemSolving:
        return "PROBLEM_SOLVING";
    case TaskType::RiskAlert:
        return "RISK_ALERT";
    }
    return "UNKNOWN";
}
