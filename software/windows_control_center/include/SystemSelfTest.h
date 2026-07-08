#pragma once

#include "AppConfig.h"
#include "ConnectionState.h"

#include <QFutureWatcher>
#include <QList>
#include <QObject>
#include <QString>
#include <functional>

enum class SelfTestOutcome {
    Passed,
    Warning,
    Failed
};

struct SelfTestCheck {
    QString id;
    QString name;
    SelfTestOutcome outcome = SelfTestOutcome::Warning;
    QString detail;
    QString suggestion;
    qint64 durationMs = 0;
};

struct SelfTestReport {
    QString reportId;
    QString startedAt;
    QString finishedAt;
    QString host;
    QList<SelfTestCheck> checks;
    QString jsonPath;
    QString textPath;

    SelfTestOutcome overallOutcome() const;
    QString summary() const;
    QByteArray toJson() const;
    QString toText() const;
};

class SelfTestProbeParser {
public:
    static QString sectionValue(const QString& output,
                                const QString& name,
                                const QString& nextName = QString());
};

class SystemSelfTestRunner : public QObject {
public:
    using ProgressCallback = std::function<void(const SelfTestCheck&, int, int)>;
    using FinishedCallback = std::function<void(const SelfTestReport&)>;

    explicit SystemSelfTestRunner(QObject* parent = nullptr);

    bool isRunning() const;
    void setProgressCallback(ProgressCallback callback);
    void setFinishedCallback(FinishedCallback callback);
    void start(const AppConfig& config, const ConnectionState& state);

private:
    static SelfTestReport runChecks(const AppConfig& config,
                                    const ConnectionState& state,
                                    const std::function<void(const SelfTestCheck&, int, int)>& progress);
    static void writeAutomaticReports(SelfTestReport& report);

    QFutureWatcher<SelfTestReport>* watcher_ = nullptr;
    ProgressCallback progressCallback_;
    FinishedCallback finishedCallback_;
};

QString selfTestOutcomeText(SelfTestOutcome outcome);
