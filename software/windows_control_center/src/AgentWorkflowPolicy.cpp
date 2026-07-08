#include "AgentWorkflowPolicy.h"

VisionWorkflowDecision AgentWorkflowPolicy::decideVisionStep(bool includeScene,
                                                              bool imageAvailable,
                                                              bool visionSucceeded,
                                                              const QString& visionError) {
    if (!includeScene) {
        return {};
    }
    if (!imageAvailable) {
        return {false, QStringLiteral("当前画面尚未同步，已回退为纯文本回答。")};
    }
    if (!visionSucceeded) {
        const QString detail = visionError.trimmed().isEmpty()
            ? QStringLiteral("视觉服务未返回有效结果")
            : visionError.trimmed();
        return {false, QStringLiteral("Qwen 视觉识别失败，已回退为纯文本回答：%1").arg(detail)};
    }
    return {true, {}};
}

AgentWorkflowExecution AgentWorkflowPolicy::execute(bool includeScene,
                                                    bool imageAvailable,
                                                    const VisionStep& visionStep,
                                                    const TextStep& textStep) {
    AgentWorkflowExecution execution;
    AgentStageResult vision;
    if (includeScene && imageAvailable && visionStep) {
        vision = visionStep();
    }

    const VisionWorkflowDecision decision = decideVisionStep(
        includeScene, imageAvailable, vision.success, vision.errorText);
    execution.warningText = decision.warningText;
    if (decision.useVisualContext) {
        execution.visionSummary = vision.content;
    }

    if (!textStep) {
        execution.errorText = QStringLiteral("DeepSeek 文本阶段未配置。");
        return execution;
    }
    const AgentStageResult text = textStep(execution.visionSummary);
    if (!text.success) {
        execution.errorText = text.errorText.trimmed().isEmpty()
            ? QStringLiteral("DeepSeek 文本阶段失败。")
            : text.errorText;
        return execution;
    }
    execution.answer = text.content;
    execution.success = true;
    return execution;
}
