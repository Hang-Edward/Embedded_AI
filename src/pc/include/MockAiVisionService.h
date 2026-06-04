#pragma once

#include "AiVisionService.h"

class MockAiVisionService final : public AiVisionService {
public:
    VisionAnalysisResult analyzeImage(const std::string& imagePath, TaskType intent) override;

private:
    VisionAnalysisResult buildSceneDescription(const std::string& imagePath) const;
    VisionAnalysisResult buildProblemSolvingHint(const std::string& imagePath) const;
    VisionAnalysisResult buildRiskAlert(const std::string& imagePath) const;
};
