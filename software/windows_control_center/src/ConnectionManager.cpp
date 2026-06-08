#include "ConnectionManager.h"

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QSet>
#include <QFileInfo>
#include <QStandardPaths>
#include <QtGlobal>

namespace {
constexpr int kVoiceSeconds = 5;

bool containsAny(const QString& text, const QStringList& needles) {
    for (const QString& needle : needles) {
        if (text.contains(needle, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}
}

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

void ConnectionManager::startPiService() {
    setPiServiceRunning(true);
}

void ConnectionManager::stopPiService() {
    setPiServiceRunning(false);
}

void ConnectionManager::restartPiService() {
    const QString host = state_.activeHost.isEmpty() ? config_.fallbackIp : state_.activeHost;
    state_.activeHost = host;
    state_.assistantStatus = AssistantStatus::Thinking;
    state_.assistantStatusText = "正在重启树莓派 embedded-ai.service...";
    state_.hardwareItems = {
        {"树莓派服务", "正在通过 SSH 重启 embedded-ai.service...", HealthLevel::Checking},
        {"网络连接", "使用 " + buildSshTarget(host), HealthLevel::Checking}
    };
    publish();

    QProcess ssh;
    ssh.start("ssh", {"-o", "BatchMode=yes", "-o", "ConnectTimeout=3", buildSshTarget(host), "systemctl --user restart embedded-ai.service && echo restarted"});
    if (!ssh.waitForFinished(8000) || ssh.exitCode() != 0) {
        if (ssh.state() != QProcess::NotRunning) {
            ssh.kill();
        }
        state_.warning = "重启树莓派服务失败。请确认 SSH key 登录正常，并且 embedded-ai.service 已安装。";
        state_.assistantStatus = AssistantStatus::Error;
        state_.assistantStatusText = "服务重启失败";
        state_.hardwareItems = {
            {"树莓派服务", "重启失败：" + QString::fromUtf8(ssh.readAllStandardError()).trimmed(), HealthLevel::Error},
            {"SSH", buildSshTarget(host), HealthLevel::Warning}
        };
        publish();
        return;
    }
    refreshNow();
}

void ConnectionManager::setPiServiceRunning(bool running) {
    const QString host = state_.activeHost.isEmpty() ? config_.fallbackIp : state_.activeHost;
    const QString action = running ? "start" : "stop";
    state_.activeHost = host;
    state_.assistantStatus = AssistantStatus::Thinking;
    state_.assistantStatusText = running ? "正在启动树莓派服务..." : "正在停止树莓派服务...";
    state_.hardwareItems = {
        {"树莓派服务", QString("正在%1 embedded-ai.service...").arg(running ? "启动" : "停止"), HealthLevel::Checking},
        {"网络连接", "使用 " + buildSshTarget(host), HealthLevel::Checking}
    };
    publish();

    QProcess ssh;
    ssh.start("ssh", {"-o", "BatchMode=yes", "-o", "ConnectTimeout=3", buildSshTarget(host), "systemctl --user " + action + " embedded-ai.service && echo ok"});
    if (!ssh.waitForFinished(8000) || ssh.exitCode() != 0) {
        if (ssh.state() != QProcess::NotRunning) {
            ssh.kill();
        }
        state_.warning = QString("%1树莓派服务失败。请确认 SSH key 登录正常，并且 embedded-ai.service 已安装。").arg(running ? "启动" : "停止");
        state_.assistantStatus = AssistantStatus::Error;
        state_.assistantStatusText = running ? "服务启动失败" : "服务停止失败";
        state_.hardwareItems = {
            {"树莓派服务", "命令失败：" + QString::fromUtf8(ssh.readAllStandardError()).trimmed(), HealthLevel::Error},
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
        state_.assistantStatus = AssistantStatus::Offline;
        state_.assistantStatusText = "未连接到树莓派";
        state_.warning = "未连接到树莓派。Ping 和 SSH 探测都失败，请确认 PC 与树莓派在同一网络，或在设置页手动输入 ssh ch@ip 后重连。";
        state_.hardwareItems = {
            {"树莓派网络", "候选 IP 都没有响应 ping", HealthLevel::Error},
            {"SSH", "未能完成 SSH 握手", HealthLevel::Error},
            {"手动输入", "请在设置页填写 ssh ch@ip", HealthLevel::Checking}
        };
        publish();
        return;
    }

    const QString host = candidates_.value(candidateIndex_);
    state_.activeHost = host;
    state_.piReachable = false;
    state_.sshOnline = false;
    state_.assistantStatus = AssistantStatus::Connecting;
    state_.assistantStatusText = "正在检测树莓派连接...";
    state_.sameLanLikely = hostLooksSameLan(host);
    state_.warning = state_.sameLanLikely
        ? QString()
        : "疑似 PC 与树莓派不在同一局域网。常见热点/家庭网络通常前三段 IP 相同，但严格判断还要看子网掩码。";
    state_.hardwareItems = {
        {"树莓派网络", "正在 ping " + host, HealthLevel::Checking},
        {"SSH", "等待网络可达后继续检测", HealthLevel::Unknown},
        {"局域网判断", state_.warning.isEmpty() ? "本机 IP 前缀看起来匹配" : state_.warning, state_.warning.isEmpty() ? HealthLevel::Ok : HealthLevel::Warning}
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
    process_.start("ssh", {"-o", "BatchMode=yes", "-o", "ConnectTimeout=3", buildSshTarget(host), "echo embedded-ai-ok"});
}

void ConnectionManager::handlePingFinished(int exitCode) {
    const QString host = candidates_.value(candidateIndex_);
    if (exitCode != 0) {
        ++candidateIndex_;
        tryNextHost();
        return;
    }

    state_.piReachable = true;
    state_.assistantStatus = AssistantStatus::Connecting;
    state_.assistantStatusText = "树莓派网络可达，正在进行 SSH 握手...";
    state_.hardwareItems = {
        {"树莓派网络", "✅ Ping 正常：" + host, HealthLevel::Ok},
        {"SSH", "正在尝试免密登录：" + buildSshTarget(host), HealthLevel::Checking},
        {"局域网判断", state_.sameLanLikely ? "✅ PC 和树莓派看起来在同一局域网" : state_.warning, state_.sameLanLikely ? HealthLevel::Ok : HealthLevel::Warning}
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
    state_.assistantStatus = AssistantStatus::Error;
    state_.assistantStatusText = "❌ SSH 连接失败：树莓派可达，但免密登录失败";
    state_.warning = "树莓派可以 ping 通，但 SSH 免密握手失败。通常是还没有配置 SSH key，或树莓派 SSH 服务拒绝当前公钥。";
    state_.hardwareItems = {
        {"树莓派网络", "✅ Ping 正常：" + host, HealthLevel::Ok},
        {"SSH", "⚠️ 免密登录失败：" + buildSshTarget(host), HealthLevel::Warning},
        {"下一步", "运行 scripts/setup-pi-ssh-key.cmd 或检查 ~/.ssh/authorized_keys", HealthLevel::Checking}
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
tail -n 320 ~/Embedded_AI/logs/embedded-ai.log 2>/dev/null || echo 'No embedded-ai.log file yet.'
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

    int buttonEventCount = 0;
    const QString latestSession = latestSessionText(&buttonEventCount);
    updateAssistantStatus(serviceOk, buttonEventCount, latestSession);
    state_.serviceActive = serviceOk;
    state_.buttonReady = state_.assistantStatus == AssistantStatus::Ready;
    parseConversationRecords();

    state_.hardwareItems = {
        {"树莓派网络", "✅ Ping 和 SSH 正常：" + state_.activeHost, HealthLevel::Ok},
        {"自启动服务", serviceOk ? "✅ embedded-ai.service 正在运行" : "⚠️ 服务状态：" + service, serviceOk ? HealthLevel::Ok : HealthLevel::Warning},
        {"蓝色按钮助手", state_.assistantStatusText, state_.assistantStatus == AssistantStatus::Ready ? HealthLevel::Ok : HealthLevel::Checking},
        {"NUCLEO 串口", serialOk ? "✅ " + serial : "❌ 未找到 /dev/ttyACM*", serialOk ? HealthLevel::Ok : HealthLevel::Error},
        {"Logitech C270 摄像头", videoOk ? "✅ " + video : "❌ 未找到 /dev/video*", videoOk ? HealthLevel::Ok : HealthLevel::Error},
        {"Logitech C270 麦克风", audioOk ? "✅ " + audio : "⚠️ arecord -l 未报告 USB 麦克风", audioOk ? HealthLevel::Ok : HealthLevel::Warning},
        {"Qwen 配置", qwenOk ? "✅ qwen-vision.ini 和 key 均存在" : "❌ 缺少 qwen 配置或 key", qwenOk ? HealthLevel::Ok : HealthLevel::Error},
        {"API 网络", networkOk ? "🌐 dashscope.aliyuncs.com 可达" : "⚠️ API 网络检测失败", networkOk ? HealthLevel::Ok : HealthLevel::Warning},
        {"最近画面", state_.localFramePath.isEmpty() ? "⚠️ 尚未拉取 latest-frame.jpg" : "🖼️ " + state_.localFramePath, state_.localFramePath.isEmpty() ? HealthLevel::Warning : HealthLevel::Ok}
    };
    state_.latestSummary = QString("服务=%1，串口=%2，摄像头=%3，麦克风=%4，Qwen=%5")
        .arg(serviceOk ? "正常" : "待检查")
        .arg(serialOk ? "正常" : "缺失")
        .arg(videoOk ? "正常" : "缺失")
        .arg(audioOk ? "正常" : "待检查")
        .arg(qwenOk ? "正常" : "缺失");
}

QString ConnectionManager::runSshTextCommand(const QString& remoteCommand, int timeoutMs) const {
    QProcess ssh;
    ssh.start("ssh", {"-o", "BatchMode=yes", "-o", "ConnectTimeout=3", buildSshTarget(state_.activeHost), remoteCommand});
    if (!ssh.waitForFinished(timeoutMs)) {
        ssh.kill();
        return "命令超时：" + remoteCommand;
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

    const QByteArray encoded = runSshBinaryCommand("test -s ~/Embedded_AI/captures/latest-frame.jpg && base64 -w 0 ~/Embedded_AI/captures/latest-frame.jpg", 9000);
    const QByteArray bytes = QByteArray::fromBase64(encoded.trimmed());
    if (bytes.isEmpty() || !bytes.startsWith(QByteArray::fromHex("FFD8FF"))) {
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
    state_.localFramePath = QFileInfo(snapshotPath).absoluteFilePath();
}

void ConnectionManager::loadRecentFrames() {
    QDir dir(cacheDirPath());
    const QFileInfoList files = dir.entryInfoList({"frame-*.jpg"}, QDir::Files, QDir::Time);
    state_.recentFramePaths.clear();
    for (const QFileInfo& info : files) {
        state_.recentFramePaths << info.absoluteFilePath();
    }
}

void ConnectionManager::parseConversationRecords() {
    state_.recentRecords.clear();
    if (state_.logText.trimmed().isEmpty()) {
        return;
    }

    const QString lower = state_.logText.toLower();
    const QString marker = "button event received";
    QList<int> starts;
    int pos = 0;
    while (true) {
        const int found = lower.indexOf(marker, pos);
        if (found < 0) {
            break;
        }
        starts << found;
        pos = found + marker.size();
    }
    if (starts.isEmpty()) {
        return;
    }

    QSet<QString> seenIds;
    for (int i = starts.size() - 1; i >= 0 && state_.recentRecords.size() < 10; --i) {
        const int begin = starts[i];
        const int end = (i + 1 < starts.size()) ? starts[i + 1] : state_.logText.size();
        const QString session = state_.logText.mid(begin, end - begin).trimmed();
        if (!containsAny(session, {"Voice vision analysis recorded", "Vision analysis recorded"})) {
            continue;
        }

        const QString recordId = extractRecordId(session);
        if (!recordId.isEmpty() && seenIds.contains(recordId)) {
            continue;
        }
        if (!recordId.isEmpty()) {
            seenIds.insert(recordId);
        }

        ConversationRecord record;
        record.title = recordId.isEmpty() ? "完成记录" : "记录 ID " + recordId;
        record.timestamp = recordId.isEmpty() ? "完成" : "ID " + recordId;
        record.userText = extractLastUserText(session);
        record.aiText = extractLastAnswer(session);
        record.flowText = formatSessionFlow(session);
        record.imagePath = state_.recentFramePaths.value(state_.recentRecords.size(), state_.localFramePath);
        if (!record.aiText.trimmed().isEmpty()) {
            state_.recentRecords << record;
        }
    }
}

QString ConnectionManager::extractRecordId(const QString& sessionText) const {
    const QRegularExpression re(R"(analysis recorded\. ID=(\d+))", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = re.match(sessionText);
    return match.hasMatch() ? match.captured(1) : QString();
}

QString ConnectionManager::extractLastAnswer(const QString& sessionText) const {
    QString cleaned = sessionText;
    const int source = cleaned.indexOf("Source image:", 0, Qt::CaseInsensitive);
    if (source >= 0) {
        cleaned = cleaned.left(source);
    }
    const int risk = cleaned.lastIndexOf("Risk=", -1, Qt::CaseInsensitive);
    if (risk >= 0) {
        const int nextLine = cleaned.indexOf('\n', risk);
        if (nextLine >= 0) {
            const QString answer = cleaned.mid(nextLine + 1).trimmed();
            if (!answer.isEmpty()) {
                return answer.left(2200);
            }
        }
    }
    return cleaned.right(1600).trimmed();
}

QString ConnectionManager::extractLastUserText(const QString& sessionText) const {
    const QStringList markers = {"Recognized command:", "ASR transcript:", "Transcript:", "Voice command:", "语音识别：", "语音识别:"};
    for (const QString& marker : markers) {
        const int index = sessionText.lastIndexOf(marker, -1, Qt::CaseInsensitive);
        if (index >= 0) {
            QString text = sessionText.mid(index + marker.size()).trimmed().section('\n', 0, 0).trimmed();
            if (text == "(empty)") {
                text = "未识别到有效语音，系统已自动改为描述当前画面。";
            }
            return text;
        }
    }
    if (sessionText.contains("Speech recognition failed", Qt::CaseInsensitive)) {
        return "⚠️ 语音识别失败，系统已自动改为描述当前画面。";
    }
    return "✅ 蓝色按钮触发，系统完成了一次画面分析。";
}

QString ConnectionManager::formatSessionFlow(const QString& sessionText) const {
    QStringList out;
    const QStringList lines = sessionText.split('\n');
    QString pendingType;
    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.contains("Button event received", Qt::CaseInsensitive)) {
            continue;
        } else if (line.contains("Recording voice command", Qt::CaseInsensitive)) {
            continue;
        } else if (line.startsWith("Audio recorded:", Qt::CaseInsensitive)) {
            out << "✅  录音完成    " + QFileInfo(line.section(':', 1).trimmed()).fileName();
        } else if (line.contains("Recognizing speech", Qt::CaseInsensitive)) {
            out << "🌐  语音识别    正在调用 Qwen ASR";
        } else if (line.contains("Speech recognition failed", Qt::CaseInsensitive)) {
            out << "⚠️  语音识别    未识别到有效文本";
        } else if (line.contains("Fallback:", Qt::CaseInsensitive)) {
            out << "✅  自动切换    改为描述当前画面";
        } else if (line.contains("Camera frame captured", Qt::CaseInsensitive)) {
            out << "📷  拍照完成    latest-frame.jpg";
        } else if (line.contains("Voice vision analysis recorded", Qt::CaseInsensitive) || line.contains("Vision analysis recorded", Qt::CaseInsensitive)) {
            out << "✅  AI 分析     已保存记录 " + extractRecordId(line);
        } else if (line.startsWith("Image=", Qt::CaseInsensitive)) {
            out << "🖼️  画面来源    " + QFileInfo(line.mid(QString("Image=").size()).trimmed()).fileName();
        } else if (line.startsWith("Type=", Qt::CaseInsensitive)) {
            pendingType = line.mid(QString("Type=").size()).trimmed();
        } else if (line.startsWith("Risk=", Qt::CaseInsensitive)) {
            out << "🛡️  风险等级    " + line.mid(QString("Risk=").size()).trimmed();
        } else if (line.contains("Ready. Press the NUCLEO blue button", Qt::CaseInsensitive)) {
            continue;
        }
    }
    if (!pendingType.isEmpty()) {
        out << "🧠  任务类型    " + pendingType;
    }
    return out.join("\n\n");
}

QString ConnectionManager::latestSessionText(int* buttonEventCount) const {
    const QString lower = state_.logText.toLower();
    const QString marker = "button event received";
    int count = 0;
    int pos = 0;
    int last = -1;
    while (true) {
        const int found = lower.indexOf(marker, pos);
        if (found < 0) {
            break;
        }
        ++count;
        last = found;
        pos = found + marker.size();
    }
    if (buttonEventCount) {
        *buttonEventCount = count;
    }
    return last >= 0 ? state_.logText.mid(last).trimmed() : QString();
}

void ConnectionManager::updateAssistantStatus(bool serviceOk, int buttonEventCount, const QString& latestSession) {
    if (!serviceOk) {
        state_.assistantStatus = AssistantStatus::Error;
        state_.assistantStatusText = "❌ 树莓派服务未运行，暂时不能按按钮";
        state_.voiceCountdownSeconds = 0;
        return;
    }

    const QString lower = latestSession.toLower();
    const int ready = lower.lastIndexOf("ready. press the nucleo blue button");
    const int received = lower.lastIndexOf("button event received");
    const int recording = lower.lastIndexOf("recording voice command");
    const int audioRecorded = lower.lastIndexOf("audio recorded:");
    const int recognizing = lower.lastIndexOf("recognizing speech");
    const int captured = lower.lastIndexOf("camera frame captured");
    const int recorded = qMax(lower.lastIndexOf("voice vision analysis recorded"), lower.lastIndexOf("vision analysis recorded"));

    if (buttonEventCount > lastButtonEventCount_ || !activeFlowSeenAt_.isValid()) {
        activeFlowSeenAt_ = QDateTime::currentDateTime();
    }
    lastButtonEventCount_ = buttonEventCount;

    if (latestSession.isEmpty() || (ready >= received && ready >= recorded && ready >= recording)) {
        state_.assistantStatus = AssistantStatus::Ready;
        state_.assistantStatusText = "✅ 已就绪：现在可以按 NUCLEO 蓝色按钮";
        state_.voiceCountdownSeconds = 0;
        return;
    }

    if (containsAny(latestSession, {"Camera capture failed", "Voice vision analysis failed", "AI analysis failed", "timed out", "timeout"})) {
        state_.assistantStatus = AssistantStatus::Error;
        state_.assistantStatusText = "❌ 故障：摄像头、AI 响应或连接流程出现失败，请查看原始日志";
        state_.voiceCountdownSeconds = 0;
        return;
    }

    if (recording >= 0 && audioRecorded < recording) {
        const int elapsed = activeFlowSeenAt_.secsTo(QDateTime::currentDateTime());
        state_.voiceCountdownSeconds = qMax(0, kVoiceSeconds - elapsed);
        state_.assistantStatus = AssistantStatus::Listening;
        state_.assistantStatusText = QString("🎙️ 已检测到按钮，正在录音：还剩 %1 秒").arg(state_.voiceCountdownSeconds);
        return;
    }

    if (received >= 0 && (recognizing >= 0 || audioRecorded >= 0 || captured >= 0 || recorded >= 0)) {
        state_.assistantStatus = AssistantStatus::Thinking;
        state_.assistantStatusText = "🌐 AI 正在识别语音并分析画面，请稍等...";
        state_.voiceCountdownSeconds = 0;
        return;
    }

    if (received >= 0) {
        state_.assistantStatus = AssistantStatus::Listening;
        state_.assistantStatusText = "🎙️ 已检测到按钮，正在准备录音...";
        state_.voiceCountdownSeconds = kVoiceSeconds;
        return;
    }

    state_.assistantStatus = AssistantStatus::Ready;
    state_.assistantStatusText = "✅ 已就绪：现在可以按 NUCLEO 蓝色按钮";
    state_.voiceCountdownSeconds = 0;
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
