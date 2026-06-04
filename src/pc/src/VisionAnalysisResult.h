#pragma once

#include "SceneTask.h"

#include <string>

struct VisionAnalysisResult {
    bool success = false;
    TaskType taskType = TaskType::SceneDescription;
    TaskRisk risk = TaskRisk::Low;
    std::string title;
    std::string prompt;
    std::string summary;
    std::string sourceImagePath;
    std::string message;
};
