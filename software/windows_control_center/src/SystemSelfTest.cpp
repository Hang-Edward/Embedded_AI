#include "SystemSelfTest.h"

#include "DeepSeekChatClient.h"
#include "QwenVisionQtClient.h"

#include <QDateTime>
#include <QAbstractSocket>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QMetaObject>
#include <QNetworkInterface>
#include <QProcess>
#include <QStandardPaths>
#include <QtConcurrent>

#include <utility>

namespace {

struct ProcessResult {
    bool started = false;
    bool timedOut = false;
    int exitCode = -1;
    QString output;
    QString error;
    qint64 durationMs = 0;
};

ProcessResult runProcess(const QString& program,
                         const QStringList& arguments,
                         int timeoutMs) {
    QElapsedTimer timer;
    timer.start();
    QProcess process;
    process.start(program, arguments);
    ProcessResult result;
    result.started = process.waitForStarted(3000);
    if (!result.started) {
        result.error = process.errorString();
        result.durationMs = timer.elapsed();
        return result;
    }
    if (!process.waitForFinished(timeoutMs)) {
        result.timedOut = true;
        process.kill();
        process.waitForFinished(1000);
    }
    result.exitCode = process.exitCode();
    result.output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    result.error = QString::fromUtf8(process.readAllStandardError()).trimmed();
    result.durationMs = timer.elapsed();
    return result;
}

QString selfTestRemoteCommand() {
    return QString::fromUtf8(R"SH(
PROJECT="$HOME/Embedded_AI"
LOG="$PROJECT/logs/embedded-ai.log"
echo __SERVICE__
systemctl --user is-active embedded-ai.service 2>/dev/null || true
echo __SERIAL__
ls /dev/ttyACM* 2>/dev/null | tr '\n' ' ' || true
echo
echo __NUCLEO__
if test -r "$LOG" && tail -n 600 "$LOG" | grep -Eqi 'Hardware connection ok|NUCLEO-F446RE AI BRIDGE READY|STATUS LED=|Port: /dev/ttyACM'; then echo OK; else echo UNVERIFIED; fi
echo __VIDEO__
if lsusb 2>/dev/null | grep -Eqi 'Logitech|C270' && ls /dev/video* >/dev/null 2>&1; then echo OK; else echo MISSING; fi
echo __AUDIO__
if arecord -l 2>/dev/null | grep -Eqi 'C270|Webcam|USB Audio'; then echo OK; else echo MISSING; fi
echo __QWEN__
if test -s "$PROJECT/config/qwen-vision.ini" && test -s "$PROJECT/config/qwen-vision.key"; then echo OK; else echo MISSING; fi
echo __HUD__
printf 'SPI='; test -e /dev/spidev0.0 && echo -n OK || echo -n MISSING
printf ' SCRIPT='; test -s "$PROJECT/hardware/pi_bridge/scripts/pi_hud.py" && echo -n OK || echo -n MISSING
printf ' GPIO='; command -v pinctrl >/dev/null 2>&1 && echo -n OK || echo -n MISSING
printf ' STATE='; test -r /tmp/embedded-ai-hud-state.json && echo OK || echo MISSING
echo __KEYPAD__
if test -r "$LOG" && tail -n 800 "$LOG" | grep -Eqi 'Keypad K-[ABC] pressed|EVENT KEY [ABC]|conversation trigger received'; then echo OK; else echo UNVERIFIED; fi
echo __FILES__
printf 'FRAME='; test -r "$PROJECT/captures/latest-frame.jpg" && echo -n OK || echo -n MISSING
printf ' LOG='; test -r "$LOG" && test -w "$LOG" && echo -n OK || echo -n MISSING
printf ' CAPTURE_DIR='; test -d "$PROJECT/captures" && test -w "$PROJECT/captures" && echo -n OK || echo -n MISSING
printf ' AUDIT='; test -r "$PROJECT/audit-log.dat" && test -w "$PROJECT/audit-log.dat" && echo OK || echo MISSING
echo __LOG__
tail -n 120 "$LOG" 2>/dev/null || true
)SH");
}

QString targetHost(const AppConfig& config, const ConnectionState& state) {
    if (!state.activeHost.trimmed().isEmpty()) {
        return state.activeHost.trimmed();
    }
    if (!config.lastSuccessfulIp.trimmed().isEmpty()) {
        return config.lastSuccessfulIp.trimmed();
    }
    return config.fallbackIp.trimmed();
}

SelfTestCheck makeCheck(const QString& id,
                        const QString& name,
                        SelfTestOutcome outcome,
                        const QString& detail,
                        const QString& suggestion,
                        qint64 durationMs = 0) {
    return {id, name, outcome, detail, suggestion, durationMs};
}

QString reportsDirectory() {
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documents.trimmed().isEmpty()) {
        documents = QDir::homePath();
    }
    const QString path = QDir(documents).filePath(QStringLiteral("The Eye of AI/diagnostics"));
    QDir().mkpath(path);
    return path;
}

QString missingOr(const QString& value) {
    return value.trimmed().isEmpty() ? QStringLiteral("无返回") : value.trimmed();
}

struct LocalStorageProbe {
    bool writable = false;
    bool conversationsReadable = false;
    QString detail;
};

LocalStorageProbe probeLocalConversationStorage() {
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.trimmed().isEmpty()) {
        baseDir = QDir::currentPath();
    }
    QDir().mkpath(baseDir);

    const QString probePath = QDir(baseDir).filePath(QStringLiteral(".self-test-write.tmp"));
    QFile probeFile(probePath);
    const QByteArray payload("SELF_TEST_WRITE_OK");
    bool writeOk = probeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)
        && probeFile.write(payload) == payload.size();
    probeFile.close();
    if (writeOk && probeFile.open(QIODevice::ReadOnly)) {
        writeOk = probeFile.readAll() == payload;
        probeFile.close();
    }
    QFile::remove(probePath);

    QStringList conversationStates;
    bool conversationsReadable = true;
    for (const QString& name : {QStringLiteral("chat-session.json"), QStringLiteral("chat-history.json")}) {
        const QString path = QDir(baseDir).filePath(name);
        const QFileInfo info(path);
        const bool readable = !info.exists() || info.isReadable();
        conversationsReadable = conversationsReadable && readable;
        conversationStates << QStringLiteral("%1=%2")
                                  .arg(name, !info.exists() ? QStringLiteral("尚未生成")
                                                           : (readable ? QStringLiteral("可读") : QStringLiteral("不可读")));
    }
    return {writeOk,
            conversationsReadable,
            QStringLiteral("Windows 会话目录=%1；写入探针=%2；%3")
                .arg(baseDir,
                     writeOk ? QStringLiteral("通过") : QStringLiteral("失败"),
                     conversationStates.join(QStringLiteral("；")))};
}

} // namespace

SystemSelfTestRunner::SystemSelfTestRunner(QObject* parent)
    : QObject(parent), watcher_(new QFutureWatcher<SelfTestReport>(this)) {
    QObject::connect(watcher_, &QFutureWatcher<SelfTestReport>::finished, this, [this]() {
        const SelfTestReport report = watcher_->result();
        if (finishedCallback_) {
            finishedCallback_(report);
        }
    });
}

bool SystemSelfTestRunner::isRunning() const {
    return watcher_->isRunning();
}

void SystemSelfTestRunner::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = std::move(callback);
}

void SystemSelfTestRunner::setFinishedCallback(FinishedCallback callback) {
    finishedCallback_ = std::move(callback);
}

void SystemSelfTestRunner::start(const AppConfig& config, const ConnectionState& state) {
    if (watcher_->isRunning()) {
        return;
    }
    const auto progress = [this](const SelfTestCheck& check, int completed, int total) {
        QMetaObject::invokeMethod(this, [this, check, completed, total]() {
            if (progressCallback_) {
                progressCallback_(check, completed, total);
            }
        });
    };
    watcher_->setFuture(QtConcurrent::run([config, state, progress]() {
        return runChecks(config, state, progress);
    }));
}

SelfTestReport SystemSelfTestRunner::runChecks(
    const AppConfig& config,
    const ConnectionState& state,
    const std::function<void(const SelfTestCheck&, int, int)>& progress) {
    constexpr int kTotalChecks = 13;
    SelfTestReport report;
    report.reportId = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    report.startedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    report.host = targetHost(config, state);

    auto append = [&](const SelfTestCheck& check) {
        report.checks.append(check);
        if (progress) {
            progress(check, report.checks.size(), kTotalChecks);
        }
    };

    QElapsedTimer localTimer;
    localTimer.start();
    QStringList addresses;
    for (const QHostAddress& address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
            addresses << address.toString();
        }
    }
    append(makeCheck(QStringLiteral("pc-network"),
                     QStringLiteral("PC 网络"),
                     addresses.isEmpty() ? SelfTestOutcome::Failed : SelfTestOutcome::Passed,
                     addresses.isEmpty() ? QStringLiteral("没有发现可用的非回环 IPv4 地址。")
                                         : QStringLiteral("本机 IPv4：%1").arg(addresses.join(QStringLiteral(", "))),
                     addresses.isEmpty() ? QStringLiteral("连接校园网、手机热点或有线网络后重新检查。")
                                         : QStringLiteral("网络接口工作正常。"),
                     localTimer.elapsed()));

    QStringList pingArgs;
#ifdef Q_OS_WIN
    pingArgs << QStringLiteral("-n") << QStringLiteral("1") << QStringLiteral("-w") << QStringLiteral("1800") << report.host;
#else
    pingArgs << QStringLiteral("-c") << QStringLiteral("1") << QStringLiteral("-W") << QStringLiteral("2") << report.host;
#endif
    const ProcessResult ping = runProcess(QStringLiteral("ping"), pingArgs, 4000);
    const bool pingOk = ping.started && !ping.timedOut && ping.exitCode == 0;
    append(makeCheck(QStringLiteral("pi-network"),
                     QStringLiteral("树莓派网络"),
                     pingOk ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     pingOk ? QStringLiteral("Ping %1 成功。").arg(report.host)
                            : QStringLiteral("Ping %1 失败：%2").arg(report.host, missingOr(ping.error)),
                     pingOk ? QStringLiteral("树莓派与 PC 网络可达。")
                            : QStringLiteral("确认两台设备连接同一网络，并检查设置页中的树莓派 IP。"),
                     ping.durationMs));

    const QString sshTarget = config.username + QStringLiteral("@") + report.host;
    const ProcessResult ssh = runProcess(
        QStringLiteral("ssh"),
        {QStringLiteral("-o"), QStringLiteral("BatchMode=yes"),
         QStringLiteral("-o"), QStringLiteral("ConnectTimeout=4"),
         sshTarget, QStringLiteral("echo SELF_TEST_SSH_OK")},
        7000);
    const bool sshOk = ssh.started && !ssh.timedOut && ssh.exitCode == 0
        && ssh.output.contains(QStringLiteral("SELF_TEST_SSH_OK"));
    append(makeCheck(QStringLiteral("ssh"),
                     QStringLiteral("SSH 握手"),
                     sshOk ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     sshOk ? QStringLiteral("免密 SSH 握手成功：%1").arg(sshTarget)
                           : QStringLiteral("SSH 失败：%1").arg(missingOr(ssh.error)),
                     sshOk ? QStringLiteral("SSH 链路可用于状态读取与服务控制。")
                           : QStringLiteral("检查 SSH key、树莓派 ssh 服务和 authorized_keys。"),
                     ssh.durationMs));

    ProcessResult probe;
    QString probeOutput;
    if (sshOk) {
        probe = runProcess(
            QStringLiteral("ssh"),
            {QStringLiteral("-o"), QStringLiteral("BatchMode=yes"),
             QStringLiteral("-o"), QStringLiteral("ConnectTimeout=4"),
             sshTarget, selfTestRemoteCommand()},
            14000);
        probeOutput = probe.output;
    }

    const QString service = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("SERVICE"), QStringLiteral("SERIAL"));
    append(makeCheck(QStringLiteral("service"),
                     QStringLiteral("embedded-ai.service"),
                     service == QStringLiteral("active") ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     sshOk ? QStringLiteral("systemd 状态：%1").arg(missingOr(service))
                           : QStringLiteral("SSH 未连接，无法检查服务。"),
                     service == QStringLiteral("active") ? QStringLiteral("自启动服务运行正常。")
                                                          : QStringLiteral("在设置页重启服务，或执行 systemctl --user status embedded-ai.service。"),
                     probe.durationMs));

    const QString serial = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("SERIAL"), QStringLiteral("NUCLEO"));
    const QString nucleo = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("NUCLEO"), QStringLiteral("VIDEO"));
    const bool serialOk = serial.contains(QStringLiteral("/dev/ttyACM"));
    const bool nucleoOk = nucleo == QStringLiteral("OK");
    append(makeCheck(QStringLiteral("nucleo"),
                     QStringLiteral("NUCLEO 串口与握手"),
                     serialOk && nucleoOk ? SelfTestOutcome::Passed
                                          : (serialOk ? SelfTestOutcome::Warning : SelfTestOutcome::Failed),
                     QStringLiteral("串口：%1；握手日志：%2").arg(missingOr(serial), missingOr(nucleo)),
                     !serialOk ? QStringLiteral("重新插拔 NUCLEO，并检查 /dev/ttyACM*。")
                               : (!nucleoOk ? QStringLiteral("服务运行时不抢占串口；按一次 K-B 后重新检查握手日志。")
                                            : QStringLiteral("串口设备与服务握手证据正常。"))));

    const QString video = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("VIDEO"), QStringLiteral("AUDIO"));
    append(makeCheck(QStringLiteral("camera"),
                     QStringLiteral("Logitech C270 摄像头"),
                     video == QStringLiteral("OK") ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     video == QStringLiteral("OK") ? QStringLiteral("USB 设备与 /dev/video* 均存在。")
                                                   : QStringLiteral("未同时发现 Logitech C270 与视频节点。"),
                     video == QStringLiteral("OK") ? QStringLiteral("摄像头采集条件正常。")
                                                   : QStringLiteral("优先插入树莓派 USB 口，运行 lsusb 和 ls /dev/video* 检查。")));

    const QString audio = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("AUDIO"), QStringLiteral("QWEN"));
    append(makeCheck(QStringLiteral("microphone"),
                     QStringLiteral("Logitech C270 麦克风"),
                     audio == QStringLiteral("OK") ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     audio == QStringLiteral("OK") ? QStringLiteral("arecord 已发现 C270 USB Audio。")
                                                   : QStringLiteral("arecord 未发现 C270 麦克风。"),
                     audio == QStringLiteral("OK") ? QStringLiteral("语音输入设备正常。")
                                                   : QStringLiteral("运行 arecord -l，并确认 ALSA 卡号没有变化。")));

    const QString qwenConfig = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("QWEN"), QStringLiteral("HUD"));
    append(makeCheck(QStringLiteral("qwen-config"),
                     QStringLiteral("Qwen 配置"),
                     qwenConfig == QStringLiteral("OK") ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     qwenConfig == QStringLiteral("OK") ? QStringLiteral("树莓派侧 qwen-vision.ini 与 key 均存在。")
                                                        : QStringLiteral("树莓派侧 Qwen 配置不完整。"),
                     qwenConfig == QStringLiteral("OK") ? QStringLiteral("配置文件存在且非空。")
                                                        : QStringLiteral("补齐 ~/Embedded_AI/config/qwen-vision.ini 与 qwen-vision.key。")));

    QElapsedTimer qwenTimer;
    qwenTimer.start();
    VisionRecognitionResult vision;
    const bool frameReadable = !state.localFramePath.isEmpty() && QFileInfo::exists(state.localFramePath);
    if (frameReadable) {
        QwenVisionQtClient client(config);
        vision = client.recognizeForPrompt(
            state.localFramePath,
            QStringLiteral("这是系统自检。请确认图片可读取，并用一句话客观描述画面。"));
    } else {
        vision.message = QStringLiteral("本地尚无可读的 latest-frame.jpg，未执行真实视觉调用。");
    }
    append(makeCheck(QStringLiteral("qwen-api"),
                     QStringLiteral("Qwen 视觉调用"),
                     vision.success ? SelfTestOutcome::Passed
                                    : (frameReadable ? SelfTestOutcome::Failed : SelfTestOutcome::Warning),
                     vision.success ? QStringLiteral("视觉模型返回有效文本：%1").arg(vision.summary.left(120))
                                    : vision.message,
                     vision.success ? QStringLiteral("Qwen 视觉 API 工作正常。")
                                    : QStringLiteral("先刷新最新画面，再检查本机 Qwen key、网络与模型名称。"),
                     qwenTimer.elapsed()));

    QElapsedTimer deepSeekTimer;
    deepSeekTimer.start();
    DeepSeekChatClient deepSeek(config);
    const ChatCompletionResult completion = deepSeek.complete({
        {QStringLiteral("user"), QStringLiteral("这是系统自检。请只回复 SELF_TEST_OK。")}
    });
    append(makeCheck(QStringLiteral("deepseek-api"),
                     QStringLiteral("DeepSeek 文本调用"),
                     completion.success ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     completion.success ? QStringLiteral("模型返回：%1").arg(completion.content.left(120))
                                        : completion.message,
                     completion.success ? QStringLiteral("DeepSeek 文本 API 工作正常。")
                                        : QStringLiteral("检查 deepseek.key、模型名、余额、网络和 API 地址。"),
                     deepSeekTimer.elapsed()));

    const QString hud = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("HUD"), QStringLiteral("KEYPAD"));
    const bool hudCoreOk = hud.contains(QStringLiteral("SPI=OK"))
        && hud.contains(QStringLiteral("SCRIPT=OK"))
        && hud.contains(QStringLiteral("GPIO=OK"));
    const bool hudStateOk = hud.contains(QStringLiteral("STATE=OK"));
    append(makeCheck(QStringLiteral("hud"),
                     QStringLiteral("LCD 与三色状态灯"),
                     hudCoreOk && hudStateOk ? SelfTestOutcome::Passed
                                             : (hudCoreOk ? SelfTestOutcome::Warning : SelfTestOutcome::Failed),
                     missingOr(hud),
                     !hudCoreOk ? QStringLiteral("检查 SPI 是否启用、pi_hud.py 是否存在以及 GPIO 工具是否安装。")
                                : (!hudStateOk ? QStringLiteral("执行一次正常流程以生成 HUD 状态文件，然后重新检查。")
                                               : QStringLiteral("LCD、GPIO 和 HUD 状态文件均可访问。"))));

    const QString keypad = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("KEYPAD"), QStringLiteral("FILES"));
    append(makeCheck(QStringLiteral("keypad"),
                     QStringLiteral("三键键盘"),
                     keypad == QStringLiteral("OK") ? SelfTestOutcome::Passed : SelfTestOutcome::Warning,
                     keypad == QStringLiteral("OK") ? QStringLiteral("日志中存在 K-A/K-B/K-C 或触发事件。")
                                                    : QStringLiteral("未在最近日志中发现三键键盘事件。"),
                     keypad == QStringLiteral("OK") ? QStringLiteral("键盘事件链路已有运行证据。")
                                                    : QStringLiteral("依次按下 K-A、K-B、K-C，再运行一次完整检查。")));

    const QString files = SelfTestProbeParser::sectionValue(probeOutput, QStringLiteral("FILES"), QStringLiteral("LOG"));
    const LocalStorageProbe localStorage = probeLocalConversationStorage();
    const bool remoteFilesOk = files.contains(QStringLiteral("FRAME=OK"))
        && files.contains(QStringLiteral("LOG=OK"))
        && files.contains(QStringLiteral("CAPTURE_DIR=OK"));
    const bool filesOk = remoteFilesOk && localStorage.writable && localStorage.conversationsReadable;
    append(makeCheck(QStringLiteral("files"),
                     QStringLiteral("照片、日志与会话文件"),
                     filesOk ? SelfTestOutcome::Passed : SelfTestOutcome::Failed,
                     QStringLiteral("树莓派：%1\n本机：%2").arg(missingOr(files), localStorage.detail),
                     filesOk ? QStringLiteral("最新照片、日志和采集目录可读写；Windows 会话由本地 JSON 管理。")
                             : QStringLiteral("检查树莓派 captures/logs/audit-log.dat 权限，以及 Windows 应用数据目录权限。")));

    report.finishedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    writeAutomaticReports(report);
    return report;
}

void SystemSelfTestRunner::writeAutomaticReports(SelfTestReport& report) {
    const QString baseName = QStringLiteral("self-test-%1").arg(report.reportId);
    report.jsonPath = QDir(reportsDirectory()).filePath(baseName + QStringLiteral(".json"));
    report.textPath = QDir(reportsDirectory()).filePath(baseName + QStringLiteral(".txt"));

    QFile jsonFile(report.jsonPath);
    if (jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        jsonFile.write(report.toJson());
    }
    QFile textFile(report.textPath);
    if (textFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        textFile.write(report.toText().toUtf8());
    }
}
