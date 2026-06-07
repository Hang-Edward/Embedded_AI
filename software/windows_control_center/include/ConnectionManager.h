#pragma once

#include "AppConfig.h"
#include "ConnectionState.h"

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <functional>

class ConnectionManager : public QObject {
public:
    using StateCallback = std::function<void(const ConnectionState&)>;

    explicit ConnectionManager(AppConfig& config, QObject* parent = nullptr);

    void setStateCallback(StateCallback callback);
    void beginAutoConnect();
    void reconnect();
    void refreshNow();
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
    QString runSshTextCommand(const QString& remoteCommand, int timeoutMs = 5000) const;
    QByteArray runSshBinaryCommand(const QString& remoteCommand, int timeoutMs = 7000) const;
    void fetchLatestFrame(const QString& remoteSignature);
    void loadRecentFrames();
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
    StateCallback callback_;
};
