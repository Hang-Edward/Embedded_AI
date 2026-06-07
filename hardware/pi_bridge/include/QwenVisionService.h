#pragma once

#include "AiVisionService.h"
#include "HttpClient.h"
#include "QwenResponseParser.h"
#include "QwenVisionConfig.h"

#include <string>

class QwenVisionService final : public AiVisionService {
public:
    QwenVisionService(QwenVisionConfig config, HttpClient& httpClient);

    VisionAnalysisResult analyzeImage(const std::string& imagePath, TaskType intent) override;
    VisionAnalysisResult analyzeImageWithPrompt(const std::string& imagePath,
        TaskType intent,
        const std::string& userPrompt) override;
    VisionAnalysisResult parseResponse(const std::string& responseJson, const std::string& imagePath, TaskType intent) const;
    std::string buildRequestBody(const std::string& imageUrlOrDataUrl, TaskType intent) const;
    std::string buildRequestBodyWithPrompt(const std::string& imageUrlOrDataUrl, TaskType intent, const std::string& prompt) const;

private:
    std::string promptFor(TaskType intent) const;
    TaskRisk defaultRiskFor(TaskType intent) const;
    std::string titleFor(TaskType intent) const;

    QwenVisionConfig config_;
    HttpClient& httpClient_;
    QwenResponseParser parser_;
};
