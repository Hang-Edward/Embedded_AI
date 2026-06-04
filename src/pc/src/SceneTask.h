#pragma once

#include <string>

enum class TaskRisk {
    Low,
    Medium,
    High
};

struct SceneTask {
    std::string title;
    std::string prompt;
    std::string aiSummary;
    TaskRisk risk = TaskRisk::Low;
};

std::string taskRiskToText(TaskRisk risk);
