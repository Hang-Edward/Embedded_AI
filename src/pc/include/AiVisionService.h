#pragma once

#include "SceneTask.h"
#include "VisionAnalysisResult.h"

#include <string>

class AiVisionService {
public:
    virtual ~AiVisionService() = default;

    // 后续接入千问视觉 API 时，只需要替换这个抽象接口的具体实现。
    virtual VisionAnalysisResult analyzeImage(const std::string& imagePath, TaskType intent) = 0;
};
