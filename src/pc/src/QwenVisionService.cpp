#include "QwenVisionService.h"

#include "ImageDataUrlBuilder.h"

#include <map>
#include <sstream>
#include <utility>

namespace {

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() + 16U);

    for (const char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }

    return escaped;
}

} // namespace

QwenVisionService::QwenVisionService(QwenVisionConfig config, HttpClient& httpClient)
    : config_(std::move(config)), httpClient_(httpClient) {
}

VisionAnalysisResult QwenVisionService::analyzeImage(const std::string& imagePath, TaskType intent) {
    VisionAnalysisResult result;
    result.taskType = intent;
    result.risk = defaultRiskFor(intent);
    result.title = titleFor(intent);
    result.prompt = promptFor(intent);
    result.sourceImagePath = imagePath;

    if (!config_.enabled) {
        result.message = "QwenVisionService is disabled. Set enabled=true in config/qwen-vision.ini.";
        return result;
    }

    if (!config_.hasApiKey()) {
        result.message = "Missing API key. Put it in " + config_.apiKeyFilePath
            + " or set environment variable " + config_.apiKeyEnvName + ".";
        return result;
    }

    const ImageDataUrlBuilder imageBuilder;
    const std::string imageDataUrl = imageBuilder.buildJpegDataUrl(imagePath);
    if (imageDataUrl.empty()) {
        result.message = "Failed to read image file for Qwen request: " + imagePath;
        return result;
    }

    const std::string requestBody = buildRequestBody(imageDataUrl, intent);
    const std::map<std::string, std::string> headers {
        {"Authorization", "Bearer " + config_.apiKey},
    };

    const HttpResponse response = httpClient_.postJson(config_.endpoint(), headers, requestBody);
    if (!response.ok()) {
        result.message = "Qwen HTTP request failed. status=" + std::to_string(response.statusCode);
        const std::string apiError = parser_.extractErrorMessage(response.body);
        if (!apiError.empty()) {
            result.message += " api_error=" + apiError;
        }
        if (!response.errorMessage.empty()) {
            result.message += " transport_error=" + response.errorMessage;
        }
        return result;
    }

    return parseResponse(response.body, imagePath, intent);
}

VisionAnalysisResult QwenVisionService::parseResponse(const std::string& responseJson,
    const std::string& imagePath,
    TaskType intent) const {
    VisionAnalysisResult result;
    result.taskType = intent;
    result.risk = defaultRiskFor(intent);
    result.title = titleFor(intent);
    result.prompt = promptFor(intent);
    result.sourceImagePath = imagePath;

    const std::string assistantText = parser_.extractAssistantText(responseJson);
    if (!assistantText.empty()) {
        result.success = true;
        result.summary = assistantText;
        result.message = "Qwen response parsed.";
        return result;
    }

    result.message = parser_.extractErrorMessage(responseJson);
    if (result.message.empty()) {
        result.message = "Qwen response did not contain choices[0].message.content.";
    }
    return result;
}

std::string QwenVisionService::buildRequestBody(const std::string& imageUrlOrDataUrl, TaskType intent) const {
    std::ostringstream body;
    body << "{";
    body << "\"model\":\"" << jsonEscape(config_.model) << "\",";
    body << "\"messages\":[{";
    body << "\"role\":\"user\",";
    body << "\"content\":[";
    body << "{\"type\":\"image_url\",\"image_url\":{\"url\":\"" << jsonEscape(imageUrlOrDataUrl) << "\"}},";
    body << "{\"type\":\"text\",\"text\":\"" << jsonEscape(promptFor(intent)) << "\"}";
    body << "]";
    body << "}]";
    body << "}";
    return body.str();
}

std::string QwenVisionService::promptFor(TaskType intent) const {
    switch (intent) {
    case TaskType::SceneDescription:
        return "Please describe the current image in concise Chinese. Focus on details useful for an AI hardware prototype demo.";
    case TaskType::ProblemSolving:
        return "Please read the problem in the image and give a step-by-step hint in Chinese. Do not only output the final answer.";
    case TaskType::RiskAlert:
        return "Please inspect the hardware scene in Chinese. Focus on power rails, ground, short circuits, loose wires, and safety risks.";
    }
    return "Please analyze the current image in Chinese.";
}

TaskRisk QwenVisionService::defaultRiskFor(TaskType intent) const {
    switch (intent) {
    case TaskType::SceneDescription:
        return TaskRisk::Low;
    case TaskType::ProblemSolving:
        return TaskRisk::Medium;
    case TaskType::RiskAlert:
        return TaskRisk::High;
    }
    return TaskRisk::Low;
}

std::string QwenVisionService::titleFor(TaskType intent) const {
    switch (intent) {
    case TaskType::SceneDescription:
        return "Qwen scene description";
    case TaskType::ProblemSolving:
        return "Qwen problem-solving hint";
    case TaskType::RiskAlert:
        return "Qwen hardware safety check";
    }
    return "Qwen vision analysis";
}
