#pragma once

#include <QString>
#include <functional>

struct VisionWorkflowDecision {
    bool useVisualContext = false;
    QString warningText;
};

struct AgentStageResult {
    bool success = false;
    QString content;
    QString errorText;
};

struct AgentWorkflowExecution {
    bool success = false;
    QString visionSummary;
    QString warningText;
    QString answer;
    QString errorText;
};

class AgentWorkflowPolicy {
public:
    using VisionStep = std::function<AgentStageResult()>;
    using TextStep = std::function<AgentStageResult(const QString& visualContext)>;

    static VisionWorkflowDecision decideVisionStep(bool includeScene,
                                                   bool imageAvailable,
                                                   bool visionSucceeded,
                                                   const QString& visionError = QString());
    static AgentWorkflowExecution execute(bool includeScene,
                                          bool imageAvailable,
                                          const VisionStep& visionStep,
                                          const TextStep& textStep);
};
