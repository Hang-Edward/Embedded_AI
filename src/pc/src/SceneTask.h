#pragma once

#include <string>

enum class TaskRisk {
    Low,
    Medium,
    High
};

enum class TaskType {
    SceneDescription,
    ProblemSolving,
    RiskAlert
};

struct SceneTask {
    TaskType type = TaskType::SceneDescription;
    std::string title;
    std::string prompt;
    std::string aiSummary;
    TaskRisk risk = TaskRisk::Low;
};

std::string taskRiskToText(TaskRisk risk);
std::string taskTypeToText(TaskType type);
