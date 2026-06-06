#pragma once

#include "SceneTask.h"
#include "VisionAnalysisResult.h"

#include <string>

class AiVisionService {
public:
    virtual ~AiVisionService() = default;

    // 中文注释：后续接入不同视觉 API 时，只需要替换这个抽象接口的具体实现。
    virtual VisionAnalysisResult analyzeImage(const std::string& imagePath, TaskType intent) = 0;

    virtual VisionAnalysisResult analyzeImageWithPrompt(const std::string& imagePath,
        TaskType intent,
        const std::string& userPrompt) {
        (void)userPrompt;
        return analyzeImage(imagePath, intent);
    }
};
