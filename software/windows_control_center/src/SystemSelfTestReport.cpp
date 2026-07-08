#include "SystemSelfTest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QString selfTestOutcomeText(SelfTestOutcome outcome) {
    switch (outcome) {
    case SelfTestOutcome::Passed:
        return QStringLiteral("通过");
    case SelfTestOutcome::Warning:
        return QStringLiteral("警告");
    case SelfTestOutcome::Failed:
        return QStringLiteral("失败");
    }
    return QStringLiteral("未知");
}

SelfTestOutcome SelfTestReport::overallOutcome() const {
    bool hasWarning = false;
    for (const SelfTestCheck& check : checks) {
        if (check.outcome == SelfTestOutcome::Failed) {
            return SelfTestOutcome::Failed;
        }
        hasWarning = hasWarning || check.outcome == SelfTestOutcome::Warning;
    }
    return hasWarning ? SelfTestOutcome::Warning : SelfTestOutcome::Passed;
}

QString SelfTestReport::summary() const {
    int passed = 0;
    int warnings = 0;
    int failed = 0;
    for (const SelfTestCheck& check : checks) {
        passed += check.outcome == SelfTestOutcome::Passed ? 1 : 0;
        warnings += check.outcome == SelfTestOutcome::Warning ? 1 : 0;
        failed += check.outcome == SelfTestOutcome::Failed ? 1 : 0;
    }
    return QStringLiteral("共 %1 项：%2 项通过，%3 项警告，%4 项失败。")
        .arg(checks.size())
        .arg(passed)
        .arg(warnings)
        .arg(failed);
}

QByteArray SelfTestReport::toJson() const {
    QJsonArray items;
    for (const SelfTestCheck& check : checks) {
        items.append(QJsonObject {
            {"id", check.id},
            {"name", check.name},
            {"outcome", selfTestOutcomeText(check.outcome)},
            {"detail", check.detail},
            {"suggestion", check.suggestion},
            {"durationMs", check.durationMs}
        });
    }
    const QJsonObject root {
        {"reportId", reportId},
        {"startedAt", startedAt},
        {"finishedAt", finishedAt},
        {"host", host},
        {"overall", selfTestOutcomeText(overallOutcome())},
        {"summary", summary()},
        {"checks", items}
    };
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

QString SelfTestReport::toText() const {
    QStringList lines;
    lines << QStringLiteral("The Eye of AI 一键完整检查报告")
          << QStringLiteral("报告编号：%1").arg(reportId)
          << QStringLiteral("目标主机：%1").arg(host)
          << QStringLiteral("开始时间：%1").arg(startedAt)
          << QStringLiteral("结束时间：%1").arg(finishedAt)
          << QStringLiteral("总体结果：%1").arg(selfTestOutcomeText(overallOutcome()))
          << summary()
          << QString();
    for (int index = 0; index < checks.size(); ++index) {
        const SelfTestCheck& check = checks[index];
        lines << QStringLiteral("[%1] %2 - %3")
                     .arg(index + 1)
                     .arg(check.name, selfTestOutcomeText(check.outcome))
              << QStringLiteral("详情：%1").arg(check.detail)
              << QStringLiteral("修复建议：%1").arg(check.suggestion)
              << QStringLiteral("耗时：%1 ms").arg(check.durationMs)
              << QString();
    }
    return lines.join('\n');
}

QString SelfTestProbeParser::sectionValue(const QString& output,
                                          const QString& name,
                                          const QString& nextName) {
    const QString marker = QStringLiteral("__") + name + QStringLiteral("__");
    const int start = output.indexOf(marker);
    if (start < 0) {
        return {};
    }
    int valueStart = start + marker.size();
    if (valueStart < output.size() && output[valueStart] == '\n') {
        ++valueStart;
    }
    int end = output.size();
    if (!nextName.isEmpty()) {
        const int next = output.indexOf(QStringLiteral("__") + nextName + QStringLiteral("__"), valueStart);
        if (next >= 0) {
            end = next;
        }
    }
    return output.mid(valueStart, end - valueStart).trimmed();
}
