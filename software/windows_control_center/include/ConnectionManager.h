#pragma once

#include "AppConfig.h"
#include "ConnectionState.h"

#include <QFutureWatcher>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QDateTime>
#include <functional>

struct HealthCheckResult;

class ConnectionManager : public QObject {
public:
    using StateCallback = std::function<void(const ConnectionState&)>;

    explicit ConnectionManager(AppConfig& config, QObject* parent = nullptr);

    void setStateCallback(StateCallback callback);
    void beginAutoConnect();
    void reconnect();
    void refreshNow();
    void startPiService();
    void stopPiService();
    void restartPiService();
    void setManualSshCommand(const QString& command);
    const ConnectionState& state() const;

private:
    enum class ProbeStage {
        None,
        Ping,
        Ssh
    };

    void tryNextHost();
    void startPing(const QString& host);
    void startSsh(const QString& host);
    void handlePingFinished(int exitCode);
    void handleSshFinished(int exitCode);
    void runHealthChecks();
    void handleAsyncHealthRefreshFinished();
    void applyHealthOutput(const QString& output);
    void setPiServiceRunning(bool running);
    QString runSshTextCommand(const QString& remoteCommand, int timeoutMs = 5000) const;
    QByteArray runSshBinaryCommand(const QString& remoteCommand, int timeoutMs = 7000) const;
    void storeLatestFrameBytes(const QString& remoteSignature, const QByteArray& bytes);
    void loadRecentFrames();
    void parseConversationRecords();
    QString extractLastAnswer(const QString& sessionText) const;
    QString extractLastUserText(const QString& sessionText) const;
    QString formatSessionFlow(const QString& sessionText) const;
    void updateAssistantStatus(bool serviceOk, int buttonEventCount, const QString& latestSession);
    QString latestSessionText(int* buttonEventCount = nullptr) const;
    QString extractRecordId(const QString& sessionText) const;
    void trimRecentFrames();
    QString cacheFilePath(const QString& fileName) const;
    QString cacheDirPath() const;
    void publish();
    void updateLocalAddresses();
    bool hostLooksSameLan(const QString& host) const;
    QString buildSshTarget(const QString& host) const;

    AppConfig& config_;
    ConnectionState state_;
    QStringList candidates_;
    int candidateIndex_ = 0;
    QProcess process_;
    ProbeStage stage_ = ProbeStage::None;
    QString lastFrameSignature_;
    int lastButtonEventCount_ = 0;
    QDateTime activeFlowSeenAt_;
    QFutureWatcher<HealthCheckResult>* refreshWatcher_ = nullptr;
    bool refreshQueued_ = false;
    StateCallback callback_;
};
