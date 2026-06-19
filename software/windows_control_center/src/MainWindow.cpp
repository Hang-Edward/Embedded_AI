#include "MainWindow.h"

#include "CameraPage.h"
#include "ChatPage.h"
#include "GlassSurface.h"
#include "HardwarePage.h"
#include "HistoryPage.h"
#include "LogsPage.h"
#include "SettingsPage.h"
#include "Theme.h"

#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

namespace {

QString statusTextFor(const ConnectionState& state) {
    if (!state.sshOnline) {
        if (state.piReachable) {
            return QStringLiteral("红灯：树莓派可达，但 SSH 握手失败");
        }
        return QStringLiteral("红灯：未连接到树莓派，请检查网络或 IP");
    }

    switch (state.assistantStatus) {
    case AssistantStatus::Ready:
        return QStringLiteral("绿灯：系统就绪，现在可以按三键键盘 K-B 触发");
    case AssistantStatus::Listening:
        return state.voiceCountdownSeconds > 0
            ? QStringLiteral("黄灯：正在录音，还剩 %1 秒").arg(state.voiceCountdownSeconds)
            : QStringLiteral("黄灯：正在录音");
    case AssistantStatus::Thinking:
        return QStringLiteral("黄灯：AI 正在识别语音并分析画面");
    case AssistantStatus::Warning:
        return QStringLiteral("黄灯：系统可用，但有项目需要检查");
    case AssistantStatus::Error:
        return QStringLiteral("红灯：流程故障，请查看诊断页和原始日志");
    case AssistantStatus::Connecting:
        return QStringLiteral("黄灯：正在检测连接与服务状态");
    case AssistantStatus::Offline:
        return QStringLiteral("红灯：服务离线");
    }
    return QStringLiteral("正在刷新状态");
}

QString statusClassFor(const ConnectionState& state) {
    if (state.assistantStatus == AssistantStatus::Ready) {
        return QStringLiteral("ready");
    }
    if (state.assistantStatus == AssistantStatus::Listening
        || state.assistantStatus == AssistantStatus::Thinking) {
        return QStringLiteral("busy");
    }
    if (state.assistantStatus == AssistantStatus::Error
        || state.assistantStatus == AssistantStatus::Offline) {
        return QStringLiteral("error");
    }
    if (state.assistantStatus == AssistantStatus::Warning) {
        return QStringLiteral("warning");
    }
    return QStringLiteral("connecting");
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), connection_(config_, this) {
    config_.load();
    setWindowTitle(QStringLiteral("Embedded AI Reality Bridge"));
    resize(1280, 820);
    setMinimumSize(920, 620);
    qApp->setStyleSheet(Theme::styleSheet());
    buildUi();

    liveTimer_ = new QTimer(this);
    liveTimer_->setInterval(1000);
    QObject::connect(liveTimer_, &QTimer::timeout, this, [this]() {
        connection_.refreshNow();
    });
    connection_.setStateCallback([this](const ConnectionState& state) {
        applyConnectionState(state);
    });
    QTimer::singleShot(160, this, [this]() {
        connection_.beginAutoConnect();
    });
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateResponsiveMode();
}

void MainWindow::buildUi() {
    central_ = new BackgroundWidget(this);
    central_->setObjectName(QStringLiteral("central"));
    setCentralWidget(central_);

    auto* root = new QHBoxLayout(central_);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(14);
    buildSidebar(root);

    auto* rightSide = new QVBoxLayout();
    rightSide->setContentsMargins(0, 0, 0, 0);
    rightSide->setSpacing(14);
    buildHeader(rightSide);
    buildPages(rightSide);
    root->addLayout(rightSide, 1);
}

void MainWindow::buildSidebar(QHBoxLayout* root) {
    sidebar_ = new GlassSurface(GlassSurface::Tone::Sidebar, central_);
    sidebar_->setObjectName(QStringLiteral("sidebar"));
    sidebar_->setFixedWidth(232);
    auto* side = new QVBoxLayout(sidebar_);
    side->setContentsMargins(16, 18, 16, 16);
    side->setSpacing(10);

    auto* title = new QLabel(QStringLiteral("Embedded AI\nReality Bridge"), sidebar_);
    title->setObjectName(QStringLiteral("appTitle"));
    side->addWidget(title);
    side->addSpacing(16);

    side->addWidget(makeNavButton(QStringLiteral("chat"), QStringLiteral("实时对话")));
    side->addWidget(makeNavButton(QStringLiteral("history"), QStringLiteral("历史记录")));
    side->addWidget(makeNavButton(QStringLiteral("hardware"), QStringLiteral("连接诊断")));
    side->addWidget(makeNavButton(QStringLiteral("camera"), QStringLiteral("摄像头画面")));
    side->addWidget(makeNavButton(QStringLiteral("logs"), QStringLiteral("原始日志")));
    side->addWidget(makeNavButton(QStringLiteral("settings"), QStringLiteral("设置")));
    side->addStretch(1);

    auto* reconnect = new QPushButton(QStringLiteral("重新连接"), sidebar_);
    reconnect->setObjectName(QStringLiteral("primaryButton"));
    QObject::connect(reconnect, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.reconnect();
    });
    side->addWidget(reconnect);

    root->addWidget(sidebar_);
}

void MainWindow::buildHeader(QVBoxLayout* rightSide) {
    auto* header = new GlassSurface(GlassSurface::Tone::Elevated, central_);
    header->setObjectName(QStringLiteral("glassHeader"));
    auto* layout = new QVBoxLayout(header);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(8);

    heroEyebrow_ = new QLabel(QStringLiteral("AI 现实桥接控制台"), header);
    heroEyebrow_->setObjectName(QStringLiteral("heroEyebrow"));

    connectionTitle_ = new QLabel(QStringLiteral("正在连接树莓派..."), header);
    connectionTitle_->setObjectName(QStringLiteral("connectionTitle"));
    connectionSubtitle_ = new QLabel(
        QStringLiteral("优先尝试最近一次成功 IP，然后尝试 ssh ch@172.20.10.6。"),
        header);
    connectionSubtitle_->setObjectName(QStringLiteral("connectionSubtitle"));
    connectionSubtitle_->setWordWrap(true);

    layout->addWidget(heroEyebrow_);
    layout->addWidget(connectionTitle_);
    layout->addWidget(connectionSubtitle_);
    rightSide->addWidget(header);
}

void MainWindow::buildPages(QVBoxLayout* rightSide) {
    auto* contentRow = new QHBoxLayout();
    contentRow->setContentsMargins(0, 0, 0, 0);
    contentRow->setSpacing(14);

    pageSurface_ = new GlassSurface(GlassSurface::Tone::Regular, central_);
    pageSurface_->setObjectName(QStringLiteral("mainContentSurface"));
    auto* surfaceLayout = new QVBoxLayout(pageSurface_);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    stack_ = new TransparentStackedWidget(pageSurface_);
    stack_->setObjectName(QStringLiteral("pageStack"));
    chatPage_ = new ChatPage(stack_);
    historyPage_ = new HistoryPage(stack_);
    hardwarePage_ = new HardwarePage(stack_);
    cameraPage_ = new CameraPage(stack_);
    logsPage_ = new LogsPage(stack_);
    settingsPage_ = new SettingsPage(config_, stack_);
    settingsPage_->setServiceActions(
        [this]() { settingsPage_->saveToConfig(); connection_.reconnect(); },
        [this]() { settingsPage_->saveToConfig(); connection_.restartPiService(); },
        [this]() { settingsPage_->saveToConfig(); connection_.startPiService(); },
        [this]() { settingsPage_->saveToConfig(); connection_.stopPiService(); },
        [this]() { setWatchLive(!watchLive_); });
    stack_->addWidget(chatPage_);
    stack_->addWidget(historyPage_);
    stack_->addWidget(hardwarePage_);
    stack_->addWidget(cameraPage_);
    stack_->addWidget(logsPage_);
    stack_->addWidget(settingsPage_);
    surfaceLayout->addWidget(stack_);

    statusSurface_ = new GlassSurface(GlassSurface::Tone::Elevated, central_);
    statusSurface_->setObjectName(QStringLiteral("statusColumn"));
    statusSurface_->setFixedWidth(328);
    auto* statusLayout = new QVBoxLayout(statusSurface_);
    statusLayout->setContentsMargins(18, 18, 18, 18);
    statusLayout->setSpacing(12);

    auto* statusTitle = new QLabel(QStringLiteral("系统状态"), statusSurface_);
    statusTitle->setObjectName(QStringLiteral("statusColumnTitle"));
    auto* statusSubtitle = new QLabel(QStringLiteral("这里集中展示连接、阶段、触发与演示模式，不再占用主舞台。"), statusSurface_);
    statusSubtitle->setObjectName(QStringLiteral("statusColumnSubtitle"));
    statusSubtitle->setWordWrap(true);

    actionBanner_ = new QLabel(QStringLiteral("正在自动检测树莓派、服务、摄像头和日志..."), statusSurface_);
    actionBanner_->setObjectName(QStringLiteral("actionBanner"));
    actionBanner_->setWordWrap(true);

    networkChip_ = new QLabel(statusSurface_);
    phaseChip_ = new QLabel(statusSurface_);
    triggerChip_ = new QLabel(statusSurface_);
    modeChip_ = new QLabel(statusSurface_);
    const QList<QLabel*> chips {networkChip_, phaseChip_, triggerChip_, modeChip_};
    for (QLabel* chip : chips) {
        chip->setObjectName(QStringLiteral("metricChip"));
        chip->setWordWrap(true);
    }

    lanWarning_ = new QLabel(statusSurface_);
    lanWarning_->setObjectName(QStringLiteral("statusHint"));
    lanWarning_->setWordWrap(true);
    lanWarning_->hide();

    statusLayout->addWidget(statusTitle);
    statusLayout->addWidget(statusSubtitle);
    statusLayout->addWidget(actionBanner_);
    statusLayout->addWidget(networkChip_);
    statusLayout->addWidget(phaseChip_);
    statusLayout->addWidget(triggerChip_);
    statusLayout->addWidget(modeChip_);
    statusLayout->addStretch(1);
    statusLayout->addWidget(lanWarning_);

    contentRow->addWidget(pageSurface_, 1);
    contentRow->addWidget(statusSurface_);
    rightSide->addLayout(contentRow, 1);
    switchPage(QStringLiteral("chat"));
}

void MainWindow::switchPage(const QString& key) {
    const QMap<QString, QWidget*> pages {
        {QStringLiteral("chat"), chatPage_},
        {QStringLiteral("history"), historyPage_},
        {QStringLiteral("hardware"), hardwarePage_},
        {QStringLiteral("camera"), cameraPage_},
        {QStringLiteral("logs"), logsPage_},
        {QStringLiteral("settings"), settingsPage_}
    };
    QWidget* page = pages.value(key, chatPage_);
    QWidget* previous = stack_->currentWidget();
    if (previous && previous != page) {
        previous->setVisible(false);
        previous->setUpdatesEnabled(false);
    }
    stack_->setCurrentWidget(page);
    page->setUpdatesEnabled(true);
    page->setVisible(true);
    page->raise();
    pageSurface_->update();
    stack_->update();
    for (auto it = navButtons_.begin(); it != navButtons_.end(); ++it) {
        it.value()->setProperty("active", it.key() == key);
        it.value()->style()->unpolish(it.value());
        it.value()->style()->polish(it.value());
    }
}

void MainWindow::applyConnectionState(const ConnectionState& state) {
    if (state.sshOnline) {
        connectionTitle_->setText(QStringLiteral("已连接：ssh ch@") + state.activeHost);
        connectionSubtitle_->setText(QStringLiteral("正在通过 SSH 自动刷新服务、硬件、日志和最新图片。"));
    } else if (state.piReachable) {
        connectionTitle_->setText(QStringLiteral("树莓派网络可达：") + state.activeHost);
        connectionSubtitle_->setText(QStringLiteral("Ping 正常，但 SSH 握手失败。请检查 SSH key 或 authorized_keys。"));
    } else if (!state.activeHost.isEmpty()) {
        connectionTitle_->setText(QStringLiteral("正在检测树莓派：") + state.activeHost);
        connectionSubtitle_->setText(QStringLiteral("先检测网络可达性，再检测 SSH。"));
    } else {
        connectionTitle_->setText(QStringLiteral("树莓派未连接"));
        connectionSubtitle_->setText(QStringLiteral("请确认 PC 与树莓派在同一网络，或在设置页手动输入 ssh ch@ip 后重连。"));
    }

    const QString statusClass = statusClassFor(state);
    const QString cleanStatus = statusTextFor(state);
    actionBanner_->setProperty("status", statusClass);
    if (statusClass != lastStatusClass_) {
        actionBanner_->style()->unpolish(actionBanner_);
        actionBanner_->style()->polish(actionBanner_);
        lastStatusClass_ = statusClass;
    }
    if (actionBanner_->text() != cleanStatus) {
        actionBanner_->setText(cleanStatus);
    }

    setMetricChipText(
        networkChip_,
        QStringLiteral("🌐 连接链路"),
        state.sshOnline ? QStringLiteral("树莓派在线 / SSH 已握手")
                        : state.piReachable ? QStringLiteral("已 Ping 通 / SSH 异常")
                                            : QStringLiteral("等待树莓派连接"),
        state.sshOnline ? QStringLiteral("ready")
                        : state.piReachable ? QStringLiteral("warning")
                                            : QStringLiteral("error"));
    setMetricChipText(
        phaseChip_,
        QStringLiteral("🧭 当前阶段"),
        phaseTextFor(state),
        state.assistantStatus == AssistantStatus::Ready
            ? QStringLiteral("ready")
            : (state.assistantStatus == AssistantStatus::Listening
               || state.assistantStatus == AssistantStatus::Thinking)
                ? QStringLiteral("busy")
                : (state.assistantStatus == AssistantStatus::Error
                   || state.assistantStatus == AssistantStatus::Offline)
                    ? QStringLiteral("error")
                    : QStringLiteral("warning"));
    setMetricChipText(
        triggerChip_,
        QStringLiteral("🎛 触发方式"),
        triggerTextFor(state),
        state.buttonReady ? QStringLiteral("ready")
                          : (state.assistantStatus == AssistantStatus::Listening
                             || state.assistantStatus == AssistantStatus::Thinking)
                                ? QStringLiteral("busy")
                                : QStringLiteral("warning"));
    setMetricChipText(
        modeChip_,
        QStringLiteral("🖥 演示模式"),
        displayModeTextFor(),
        QStringLiteral("neutral"));

    lanWarning_->setVisible(!state.warning.isEmpty() && !state.sshOnline);
    lanWarning_->setText(
        QStringLiteral("网络提示：如果 PC 和树莓派没有处在同一局域网，SSH 可能无法连接。手机热点下通常前三段 IP 相同。"));
    hardwarePage_->setState(state);
    logsPage_->setLogText(state.logText);
    cameraPage_->setImagePath(state.localFramePath);
    if (state.sshOnline) {
        chatPage_->setLatestSession(state);
        historyPage_->setRecords(state);
        if (watchLive_ && !liveTimer_->isActive()) {
            liveTimer_->start();
        }
    } else if (liveTimer_->isActive()) {
        liveTimer_->stop();
    }
}

void MainWindow::updateResponsiveMode() {
    const bool compact = width() < 1040;
    sidebar_->setFixedWidth(compact ? 98 : 232);
    if (statusSurface_ != nullptr) {
        statusSurface_->setFixedWidth(width() < 1240 ? 286 : 328);
    }
    for (auto it = navButtons_.begin(); it != navButtons_.end(); ++it) {
        it.value()->setMinimumHeight(compact ? 48 : 54);
    }
}

void MainWindow::setWatchLive(bool enabled) {
    watchLive_ = enabled;
    settingsPage_->setWatchLiveState(enabled);
    if (enabled) {
        connection_.refreshNow();
        if (!liveTimer_->isActive()) {
            liveTimer_->start();
        }
    } else {
        liveTimer_->stop();
    }
}

QPushButton* MainWindow::makeNavButton(const QString& key, const QString& text) {
    auto* button = new LiquidNavButton(text, sidebar_);
    button->setProperty("active", false);
    QObject::connect(button, &QPushButton::clicked, this, [this, key]() {
        switchPage(key);
    });
    navButtons_.insert(key, button);
    return button;
}

void MainWindow::setMetricChipText(QLabel* chip, const QString& label, const QString& value, const QString& tone) {
    if (chip == nullptr) {
        return;
    }
    chip->setText(QString(
        "<span style='color:#b8cbe3;font-size:11px;font-weight:700;'>%1</span><br>"
        "<span style='color:#f7fbff;font-size:15px;font-weight:800;'>%2</span>")
            .arg(label, value));
    chip->setProperty("tone", tone);
    chip->style()->unpolish(chip);
    chip->style()->polish(chip);
}

QString MainWindow::phaseTextFor(const ConnectionState& state) const {
    switch (state.assistantStatus) {
    case AssistantStatus::Ready:
        return QStringLiteral("待命完成，可立即触发");
    case AssistantStatus::Listening:
        return state.voiceCountdownSeconds > 0
            ? QStringLiteral("正在录音，剩余 %1 秒").arg(state.voiceCountdownSeconds)
            : QStringLiteral("正在等待语音结束");
    case AssistantStatus::Thinking:
        return QStringLiteral("AI 正在识别与分析画面");
    case AssistantStatus::Warning:
        return QStringLiteral("系统可运行，但存在待检查项");
    case AssistantStatus::Error:
        return QStringLiteral("流程中断，等待恢复");
    case AssistantStatus::Connecting:
        return QStringLiteral("正在扫描硬件与服务");
    case AssistantStatus::Offline:
        return QStringLiteral("树莓派离线");
    }
    return QStringLiteral("状态未知");
}

QString MainWindow::triggerTextFor(const ConnectionState& state) const {
    if (state.assistantStatus == AssistantStatus::Listening || state.assistantStatus == AssistantStatus::Thinking) {
        return QStringLiteral("三键键盘 K-B 触发后自动执行");
    }
    if (state.buttonReady) {
        return QStringLiteral("三键键盘 K-B 可立即触发一次分析");
    }
    return QStringLiteral("等待硬件就绪后开放触发");
}

QString MainWindow::displayModeTextFor() const {
    return QStringLiteral("Windows 控制中心 + 树莓派实时桥接");
}
