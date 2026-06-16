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
            return "红灯：树莓派可达，但 SSH 握手失败";
        }
        return "红灯：未连接到树莓派，请检查网络或 IP";
    }
    switch (state.assistantStatus) {
    case AssistantStatus::Ready:
        return "绿灯：系统就绪，现在可以按三键键盘 K-B 触发";
    case AssistantStatus::Listening:
        return state.voiceCountdownSeconds > 0
            ? QString("黄灯：正在录音，还剩 %1 秒").arg(state.voiceCountdownSeconds)
            : "黄灯：正在录音";
    case AssistantStatus::Thinking:
        return "黄灯：AI 正在识别语音并分析画面";
    case AssistantStatus::Warning:
        return "黄灯：系统可用，但有项目需要检查";
    case AssistantStatus::Error:
        return "红灯：流程故障，请查看诊断页和原始日志";
    case AssistantStatus::Connecting:
        return "黄灯：正在检测连接与服务状态";
    case AssistantStatus::Offline:
        return "红灯：服务离线";
    }
    return "正在刷新状态";
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), connection_(config_, this) {
    config_.load();
    setWindowTitle("Embedded AI Reality Bridge");
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
    central_->setObjectName("central");
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
    sidebar_->setObjectName("sidebar");
    sidebar_->setFixedWidth(232);
    auto* side = new QVBoxLayout(sidebar_);
    side->setContentsMargins(16, 18, 16, 16);
    side->setSpacing(10);

    auto* title = new QLabel("Embedded AI\nReality Bridge", sidebar_);
    title->setObjectName("appTitle");
    side->addWidget(title);
    side->addSpacing(16);

    side->addWidget(makeNavButton("chat", "实时对话"));
    side->addWidget(makeNavButton("history", "历史记录"));
    side->addWidget(makeNavButton("hardware", "连接诊断"));
    side->addWidget(makeNavButton("camera", "摄像头画面"));
    side->addWidget(makeNavButton("logs", "原始日志"));
    side->addWidget(makeNavButton("settings", "设置"));
    side->addStretch(1);

    auto* reconnect = new QPushButton("重新连接", sidebar_);
    reconnect->setObjectName("primaryButton");
    QObject::connect(reconnect, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.reconnect();
    });
    side->addWidget(reconnect);

    root->addWidget(sidebar_);
}

void MainWindow::buildHeader(QVBoxLayout* rightSide) {
    auto* header = new GlassSurface(GlassSurface::Tone::Elevated, central_);
    header->setObjectName("glassHeader");
    auto* layout = new QVBoxLayout(header);
    layout->setContentsMargins(22, 18, 22, 18);
    layout->setSpacing(9);

    connectionTitle_ = new QLabel("正在连接树莓派...", header);
    connectionTitle_->setObjectName("connectionTitle");
    connectionSubtitle_ = new QLabel("优先尝试最近一次成功 IP，然后尝试 ssh ch@172.20.10.6。", header);
    connectionSubtitle_->setObjectName("connectionSubtitle");
    connectionSubtitle_->setWordWrap(true);

    actionBanner_ = new QLabel("正在自动检测树莓派、服务、摄像头和日志...", header);
    actionBanner_->setObjectName("actionBanner");
    actionBanner_->setWordWrap(true);

    lanWarning_ = new QLabel(header);
    lanWarning_->setObjectName("connectionSubtitle");
    lanWarning_->setWordWrap(true);
    lanWarning_->hide();

    layout->addWidget(connectionTitle_);
    layout->addWidget(connectionSubtitle_);
    layout->addWidget(actionBanner_);
    layout->addWidget(lanWarning_);
    rightSide->addWidget(header);
}

void MainWindow::buildPages(QVBoxLayout* rightSide) {
    pageSurface_ = new GlassSurface(GlassSurface::Tone::Regular, central_);
    auto* surfaceLayout = new QVBoxLayout(pageSurface_);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    stack_ = new TransparentStackedWidget(pageSurface_);
    stack_->setObjectName("pageStack");
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
    rightSide->addWidget(pageSurface_, 1);
    switchPage("chat");
}

void MainWindow::switchPage(const QString& key) {
    const QMap<QString, QWidget*> pages {
        {"chat", chatPage_},
        {"history", historyPage_},
        {"hardware", hardwarePage_},
        {"camera", cameraPage_},
        {"logs", logsPage_},
        {"settings", settingsPage_}
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
        connectionTitle_->setText("已连接：ssh ch@" + state.activeHost);
        connectionSubtitle_->setText("正在通过 SSH 自动刷新服务、硬件、日志和最新图片。");
    } else if (state.piReachable) {
        connectionTitle_->setText("树莓派网络可达：" + state.activeHost);
        connectionSubtitle_->setText("Ping 正常，但 SSH 握手失败。请检查 SSH key 或 authorized_keys。");
    } else if (!state.activeHost.isEmpty()) {
        connectionTitle_->setText("正在检测树莓派：" + state.activeHost);
        connectionSubtitle_->setText("先检测网络可达性，再检测 SSH。");
    } else {
        connectionTitle_->setText("树莓派未连接");
        connectionSubtitle_->setText("请确认 PC 与树莓派在同一网络，或在设置页手动输入 ssh ch@ip 后重连。");
    }

    QString statusClass = "connecting";
    if (state.assistantStatus == AssistantStatus::Ready) {
        statusClass = "ready";
    } else if (state.assistantStatus == AssistantStatus::Listening || state.assistantStatus == AssistantStatus::Thinking) {
        statusClass = "busy";
    } else if (state.assistantStatus == AssistantStatus::Error || state.assistantStatus == AssistantStatus::Offline) {
        statusClass = "error";
    } else if (state.assistantStatus == AssistantStatus::Warning) {
        statusClass = "warning";
    }
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

    lanWarning_->setVisible(!state.warning.isEmpty() && !state.sshOnline);
    lanWarning_->setText("网络提示：如果 PC 和树莓派没有处在同一局域网，SSH 可能无法连接。手机热点下通常前三段 IP 相同。");
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
