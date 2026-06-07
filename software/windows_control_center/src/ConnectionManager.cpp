#include "ConnectionManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QtGlobal>

ConnectionManager::ConnectionManager(AppConfig& config, QObject* parent)
    : QObject(parent), config_(config) {
    QObject::connect(&process_,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        [this](int exitCode, QProcess::ExitStatus) {
            if (stage_ == ProbeStage::Ping) {
                handlePingFinished(exitCode);
                return;
            }
            if (stage_ == ProbeStage::Ssh) {
                handleSshFinished(exitCode);
            }
        });
}

void ConnectionManager::setStateCallback(StateCallback callback) {
    callback_ = std::move(callback);
}

void ConnectionManager::beginAutoConnect() {
    state_ = ConnectionState {};
    updateLocalAddresses();
    loadRecentFrames();
    candidates_ = config_.candidateHosts();
    candidateIndex_ = 0;
    tryNextHost();
}

void ConnectionManager::reconnect() {
    if (process_.state() != QProcess::NotRunning) {
        process_.kill();
    }
    stage_ = ProbeStage::None;
    beginAutoConnect();
}

void ConnectionManager::refreshNow() {
    if (!state_.sshOnline || state_.activeHost.isEmpty()) {
        reconnect();
        return;
    }

    runHealthChecks();
    publish();
}

void ConnectionManager::restartPiService() {
    const QString host = state_.activeHost.isEmpty() ? config_.fallbackIp : state_.activeHost;
    state_.activeHost = host;
    state_.hardwareItems = {
        {"embedded-ai.service", "Restarting over SSH...", HealthLevel::Checking},
        {"Raspberry Pi network", "Using " + buildSshTarget(host), HealthLevel::Checking}
    };
    publish();

    QProcess ssh;
    ssh.start("ssh", {"-o", "BatchMode=yes", "-o", "ConnectTimeout=3", buildSshTarget(host), "systemctl --user restart embedded-ai.service && echo restarted"});
    if (!ssh.waitForFinished(8000) || ssh.exitCode() != 0) {
        if (ssh.state() != QProcess::NotRunning) {
            ssh.kill();
        }
        state_.warning = "重启树莓派服务失败。请确认 SSH key 登录正常，且 embedded-ai.service 已安装。";
        state_.hardwareItems = {
            {"embedded-ai.service", "Restart failed: " + QString::fromUtf8(ssh.readAllStandardError()).trimmed(), HealthLevel::Error},
            {"SSH", buildSshTarget(host), HealthLevel::Warning}
        };
        publish();
        return;
    }

    refreshNow();
}

void ConnectionManager::setManualSshCommand(const QString& command) {
    config_.manualSshCommand = command;
    config_.save();
}

const ConnectionState& ConnectionManager::state() const {
    return state_;
}

void ConnectionManager::tryNextHost() {
    if (candidateIndex_ >= candidates_.size()) {
        stage_ = ProbeStage::None;
        state_.piReachable = false;
        state_.sshOnline = false;
        state_.activeHost.clear();
        state_.warning = "未连接到树莓派。Ping 和 SSH 探测都失败，请确认 PC 与树莓派在同一网络，或在 Settings 手动输入 ssh ch@ip 后重连。";
        state_.hardwareItems = {
            {"Raspberry Pi network", "No candidate IP responded to ping", HealthLevel::Error},
            {"SSH", "No SSH handshake attempted successfully", HealthLevel::Error},
            {"Manual input", "Waiting for ssh ch@ip", HealthLevel::Checking}
        };
        publish();
        return;
    }

    const QString host = candidates_.value(candidateIndex_);
    state_.activeHost = host;
    state_.piReachable = false;
    state_.sshOnline = false;
    state_.sameLanLikely = hostLooksSameLan(host);
    state_.warning = state_.sameLanLikely
        ? QString()
        : "疑似 PC 与树莓派不在同一局域网。常见热点/家庭网络通常前三段 IP 相同，但严格判断要看子网掩码。";
    state_.hardwareItems = {
        {"Raspberry Pi network", "Pinging " + host, HealthLevel::Checking},
        {"SSH", "Waiting for network reachability", HealthLevel::Unknown},
        {"LAN", state_.warning.isEmpty() ? "Local prefix looks compatible" : state_.warning, state_.warning.isEmpty() ? HealthLevel::Ok : HealthLevel::Warning}
    };
    publish();
    startPing(host);
}

void ConnectionManager::startPing(const QString& host) {
    stage_ = ProbeStage::Ping;
    QStringList args;
#ifdef Q_OS_WIN
    args << "-n" << "1" << "-w" << "1200" << host;
#else
    args << "-c" << "1" << "-W" << "1" << host;
#endif
    process_.start("ping", args);
}

void ConnectionManager::startSsh(const QString& host) {
    stage_ = ProbeStage::Ssh;
    QStringList args;
    args << "-o" << "BatchMode=yes"
         << "-o" << "ConnectTimeout=3"
         << buildSshTarget(host)
         << "echo embedded-ai-ok";
    process_.start("ssh", args);
}

void ConnectionManager::handlePingFinished(int exitCode) {
    const QString host = candidates_.value(candidateIndex_);
    if (exitCode != 0) {
        ++candidateIndex_;
        tryNextHost();
        return;
    }

    state_.piReachable = true;
    state_.hardwareItems = {
        {"Raspberry Pi network", "Ping OK: " + host, HealthLevel::Ok},
        {"SSH", "Trying key-based handshake: " + buildSshTarget(host), HealthLevel::Checking},
        {"LAN", state_.sameLanLikely ? "PC and Pi look like the same LAN" : state_.warning, state_.sameLanLikely ? HealthLevel::Ok : HealthLevel::Warning}
    };
    publish();
    startSsh(host);
}

void ConnectionManager::handleSshFinished(int exitCode) {
    const QString host = candidates_.value(candidateIndex_);
    state_.activeHost = host;
    state_.piReachable = true;

    if (exitCode == 0) {
        state_.sshOnline = true;
        state_.warning.clear();
        config_.lastSuccessfulIp = host;
        config_.save();
        runHealthChecks();
        publish();
        return;
    }

    state_.sshOnline = false;
    state_.warning = "树莓派可以 ping 通，但 SSH 免密握手失败。通常是还没有配置 SSH key，或树莓派 SSH 服务拒绝当前公钥。";
    state_.hardwareItems = {
        {"Raspberry Pi network", "Ping OK: " + host, HealthLevel::Ok},
        {"SSH", "Reachable, but key-based SSH failed for " + buildSshTarget(host), HealthLevel::Warning},
        {"Next step", "Run scripts/setup-pi-ssh-key.cmd or check ~/.ssh/authorized_keys", HealthLevel::Checking}
    };
    publish();
}

void ConnectionManager::runHealthChecks() {
    const QString remoteCommand = R"SH(
echo __SERVICE__
systemctl --user is-active embedded-ai.service 2>/dev/null || true
echo __SERIAL__
ls /dev/ttyACM* 2>/dev/null | tr '\n' ' ' || true
echo
echo __VIDEO__
ls /dev/video* 2>/dev/null | tr '\n' ' ' || true
echo
echo __AUDIO__
arecord -l 2>/dev/null | grep -Ei 'c270|webcam|usb audio|card' | head -n 4 || true
echo __QWEN__
test -s ~/Embedded_AI/config/qwen-vision.ini && test -s ~/Embedded_AI/config/qwen-vision.key && echo OK || echo MISSING
echo __NETWORK__
ping -c 1 -W 2 dashscope.aliyuncs.com >/dev/null 2>&1 && echo OK || echo FAIL
echo __FRAME__
stat -c '%Y:%s' ~/Embedded_AI/captures/latest-frame.jpg 2>/dev/null || echo MISSING
echo __LOG__
tail -n 220 ~/Embedded_AI/logs/embedded-ai.log 2>/dev/null || echo 'No embedded-ai.log file yet.'
)SH";

    const QString output = runSshTextCommand(remoteCommand, 10000);
    auto section = [&output](const QString& name, const QString& nextName = QString()) {
        const QString marker = "__" + name + "__";
        const int start = output.indexOf(marker);
        if (start < 0) {
            return QString();
        }
        int valueStart = start + marker.size();
        if (valueStart < output.size() && output[valueStart] == '\n') {
            ++valueStart;
        }
        int end = output.size();
        if (!nextName.isEmpty()) {
            const int next = output.indexOf("__" + nextName + "__", valueStart);
            if (next >= 0) {
                end = next;
            }
        }
        return output.mid(valueStart, end - valueStart).trimmed();
    };

    const QString service = section("SERVICE", "SERIAL");
    const QString serial = section("SERIAL", "VIDEO");
    const QString video = section("VIDEO", "AUDIO");
    const QString audio = section("AUDIO", "QWEN");
    const QString qwen = section("QWEN", "NETWORK");
    const QString apiNetwork = section("NETWORK", "FRAME");
    const QString frameSignature = section("FRAME", "LOG");
    state_.logText = section("LOG");
    fetchLatestFrame(frameSignature);

    const bool serviceOk = service == "active";
    const bool serialOk = !serial.isEmpty();
    const bool videoOk = !video.isEmpty();
    const bool audioOk = !audio.isEmpty();
    const bool qwenOk = qwen == "OK";
    const bool networkOk = apiNetwork == "OK";

    state_.hardwareItems = {
        {"Raspberry Pi network", "Ping and SSH OK: " + state_.activeHost, HealthLevel::Ok},
        {"embedded-ai.service", serviceOk ? "active" : "not active: " + service, serviceOk ? HealthLevel::Ok : HealthLevel::Warning},
        {"NUCLEO serial", serialOk ? serial : "No /dev/ttyACM* found", serialOk ? HealthLevel::Ok : HealthLevel::Error},
        {"Logitech C270 camera", videoOk ? video : "No /dev/video* found", videoOk ? HealthLevel::Ok : HealthLevel::Error},
        {"Logitech C270 microphone", audioOk ? audio : "No USB audio device reported by arecord -l", audioOk ? HealthLevel::Ok : HealthLevel::Warning},
        {"Qwen config", qwenOk ? "qwen-vision.ini and key exist" : "Missing qwen config or key", qwenOk ? HealthLevel::Ok : HealthLevel::Error},
        {"DashScope network", networkOk ? "dashscope.aliyuncs.com reachable" : "API network check failed", networkOk ? HealthLevel::Ok : HealthLevel::Warning},
        {"Latest frame", state_.localFramePath.isEmpty() ? "No captures/latest-frame.jpg fetched" : state_.localFramePath, state_.localFramePath.isEmpty() ? HealthLevel::Warning : HealthLevel::Ok}
    };
    state_.latestSummary = QString("Service=%1, Serial=%2, Camera=%3, Audio=%4, Qwen=%5")
        .arg(serviceOk ? "OK" : "Check")
        .arg(serialOk ? "OK" : "Missing")
        .arg(videoOk ? "OK" : "Missing")
        .arg(audioOk ? "OK" : "Check")
        .arg(qwenOk ? "OK" : "Missing");
}

QString ConnectionManager::runSshTextCommand(const QString& remoteCommand, int timeoutMs) const {
    QProcess ssh;
    ssh.start("ssh", {"-o", "BatchMode=yes", "-o", "ConnectTimeout=3", buildSshTarget(state_.activeHost), remoteCommand});
    if (!ssh.waitForFinished(timeoutMs)) {
        ssh.kill();
        return "Command timed out: " + remoteCommand;
    }
    const QString stdoutText = QString::fromUtf8(ssh.readAllStandardOutput());
    const QString stderrText = QString::fromUtf8(ssh.readAllStandardError());
    if (ssh.exitCode() != 0 && stdoutText.trimmed().isEmpty()) {
        return stderrText.trimmed();
    }
    return stdoutText;
}

QByteArray ConnectionManager::runSshBinaryCommand(const QString& remoteCommand, int timeoutMs) const {
    QProcess ssh;
    ssh.start("ssh", {"-o", "BatchMode=yes", "-o", "ConnectTimeout=3", buildSshTarget(state_.activeHost), remoteCommand});
    if (!ssh.waitForFinished(timeoutMs)) {
        ssh.kill();
        return {};
    }
    if (ssh.exitCode() != 0) {
        return {};
    }
    return ssh.readAllStandardOutput();
}

void ConnectionManager::fetchLatestFrame(const QString& remoteSignature) {
    loadRecentFrames();
    if (remoteSignature.isEmpty() || remoteSignature == "MISSING") {
        state_.localFramePath = state_.recentFramePaths.isEmpty() ? QString() : state_.recentFramePaths.first();
        return;
    }

    if (remoteSignature == lastFrameSignature_ && !state_.recentFramePaths.isEmpty()) {
        state_.localFramePath = state_.recentFramePaths.first();
        return;
    }

    const QByteArray bytes = runSshBinaryCommand("test -s ~/Embedded_AI/captures/latest-frame.jpg && cat ~/Embedded_AI/captures/latest-frame.jpg", 9000);
    if (bytes.isEmpty()) {
        state_.localFramePath = state_.recentFramePaths.isEmpty() ? QString() : state_.recentFramePaths.first();
        return;
    }

    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString snapshotPath = cacheFilePath("frame-" + stamp + ".jpg");
    QFile snapshot(snapshotPath);
    if (snapshot.open(QIODevice::WriteOnly)) {
        snapshot.write(bytes);
        snapshot.close();
    }

    QFile latest(cacheFilePath("latest-frame.jpg"));
    if (latest.open(QIODevice::WriteOnly)) {
        latest.write(bytes);
        latest.close();
    }

    lastFrameSignature_ = remoteSignature;
    trimRecentFrames();
    loadRecentFrames();
    state_.localFramePath = snapshotPath;
}

void ConnectionManager::loadRecentFrames() {
    QDir dir(cacheDirPath());
    const QFileInfoList files = dir.entryInfoList({"frame-*.jpg"}, QDir::Files, QDir::Time);
    state_.recentFramePaths.clear();
    for (const QFileInfo& info : files) {
        state_.recentFramePaths << info.absoluteFilePath();
    }
}

void ConnectionManager::trimRecentFrames() {
    QDir dir(cacheDirPath());
    const QFileInfoList files = dir.entryInfoList({"frame-*.jpg"}, QDir::Files, QDir::Time);
    for (int i = 10; i < files.size(); ++i) {
        QFile::remove(files[i].absoluteFilePath());
    }
}

QString ConnectionManager::cacheFilePath(const QString& fileName) const {
    return QDir(cacheDirPath()).filePath(fileName);
}

QString ConnectionManager::cacheDirPath() const {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString cacheDir = QDir(base).filePath("cache");
    QDir().mkpath(cacheDir);
    return cacheDir;
}

void ConnectionManager::publish() {
    if (callback_) {
        callback_(state_);
    }
}

void ConnectionManager::updateLocalAddresses() {
    state_.localIpv4Addresses.clear();
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : interfaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) || !(iface.flags() & QNetworkInterface::IsRunning)) {
            continue;
        }

        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.ip().isLoopback()) {
                state_.localIpv4Addresses << entry.ip().toString();
            }
        }
    }
}

bool ConnectionManager::hostLooksSameLan(const QString& host) const {
    const QStringList parts = host.split('.');
    if (parts.size() != 4) {
        return true;
    }

    const QString prefix = parts.mid(0, 3).join('.');
    for (const QString& local : state_.localIpv4Addresses) {
        const QStringList localParts = local.split('.');
        if (localParts.size() == 4 && localParts.mid(0, 3).join('.') == prefix) {
            return true;
        }
    }
    return false;
}

QString ConnectionManager::buildSshTarget(const QString& host) const {
    return config_.username + "@" + host;
}
