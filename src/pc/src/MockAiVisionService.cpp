#include "MockAiVisionService.h"

VisionAnalysisResult MockAiVisionService::analyzeImage(const std::string& imagePath, TaskType intent) {
    switch (intent) {
    case TaskType::SceneDescription:
        return buildSceneDescription(imagePath);
    case TaskType::ProblemSolving:
        return buildProblemSolvingHint(imagePath);
    case TaskType::RiskAlert:
        return buildRiskAlert(imagePath);
    }

    VisionAnalysisResult result;
    result.message = "Unsupported vision intent.";
    return result;
}

VisionAnalysisResult MockAiVisionService::buildSceneDescription(const std::string& imagePath) const {
    VisionAnalysisResult result;
    result.success = true;
    result.taskType = TaskType::SceneDescription;
    result.risk = TaskRisk::Low;
    result.title = "Vision scene description";
    result.prompt = "Describe the captured camera frame.";
    result.summary = "Mock vision answer: frame captured successfully. The prototype can now pass a real image path into a vision model.";
    result.sourceImagePath = imagePath;
    result.message = "Mock scene analysis completed.";
    return result;
}

VisionAnalysisResult MockAiVisionService::buildProblemSolvingHint(const std::string& imagePath) const {
    VisionAnalysisResult result;
    result.success = true;
    result.taskType = TaskType::ProblemSolving;
    result.risk = TaskRisk::Medium;
    result.title = "Vision problem-solving hint";
    result.prompt = "Read the captured problem and give a step-by-step hint.";
    result.summary = "Mock vision answer: identify the known conditions first, choose the matching formula, then calculate step by step.";
    result.sourceImagePath = imagePath;
    result.message = "Mock problem-solving analysis completed.";
    return result;
}

VisionAnalysisResult MockAiVisionService::buildRiskAlert(const std::string& imagePath) const {
    VisionAnalysisResult result;
    result.success = true;
    result.taskType = TaskType::RiskAlert;
    result.risk = TaskRisk::High;
    result.title = "Vision safety check";
    result.prompt = "Inspect the captured hardware scene for possible wiring or power risks.";
    result.summary = "Mock vision answer: possible wiring risk detected. Confirm 3.3V, 5V, and GND rails before touching the circuit.";
    result.sourceImagePath = imagePath;
    result.message = "Mock risk analysis completed.";
    return result;
}
