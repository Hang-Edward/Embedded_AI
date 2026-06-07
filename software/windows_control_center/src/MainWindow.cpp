#include "MainWindow.h"

#include "CameraPage.h"
#include "ChatPage.h"
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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), connection_(config_, this) {
    config_.load();
    setWindowTitle("嵌入式 AI 视觉助手控制台");
    resize(1280, 820);
    setMinimumSize(900, 620);
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
    QTimer::singleShot(200, this, [this]() {
        connection_.beginAutoConnect();
    });
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateResponsiveMode();
}

void MainWindow::buildUi() {
    central_ = new QWidget(this);
    central_->setObjectName("central");
    setCentralWidget(central_);

    auto* root = new QHBoxLayout(central_);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    buildSidebar(root);

    auto* rightSide = new QVBoxLayout();
    rightSide->setContentsMargins(0, 0, 0, 0);
    rightSide->setSpacing(0);
    buildHeader(rightSide);
    buildPages(rightSide);
    root->addLayout(rightSide, 1);
}

void MainWindow::buildSidebar(QHBoxLayout* root) {
    sidebar_ = new QWidget(this);
    sidebar_->setObjectName("sidebar");
    sidebar_->setFixedWidth(220);
    auto* side = new QVBoxLayout(sidebar_);
    side->setContentsMargins(16, 18, 16, 16);
    side->setSpacing(10);

    auto* title = new QLabel("嵌入式 AI\n控制台", sidebar_);
    title->setObjectName("appTitle");
    side->addWidget(title);
    side->addSpacing(18);

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

    auto* restartService = new QPushButton("重启服务", sidebar_);
    restartService->setObjectName("primaryButton");
    QObject::connect(restartService, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.restartPiService();
    });
    side->addWidget(restartService);

    auto* startService = new QPushButton("启动服务", sidebar_);
    startService->setObjectName("primaryButton");
    QObject::connect(startService, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.startPiService();
    });
    side->addWidget(startService);

    auto* stopService = new QPushButton("停止服务", sidebar_);
    stopService->setObjectName("primaryButton");
    QObject::connect(stopService, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.stopPiService();
    });
    side->addWidget(stopService);

    watchButton_ = new QPushButton("实时监听：开", sidebar_);
    watchButton_->setObjectName("primaryButton");
    QObject::connect(watchButton_, &QPushButton::clicked, this, [this]() {
        setWatchLive(!watchLive_);
    });
    side->addWidget(watchButton_);

    root->addWidget(sidebar_);
}

void MainWindow::buildHeader(QVBoxLayout* rightSide) {
    auto* header = new QWidget(this);
    auto* layout = new QVBoxLayout(header);
    layout->setContentsMargins(24, 18, 24, 14);
    layout->setSpacing(8);

    connectionTitle_ = new QLabel("正在连接树莓派...", header);
    connectionTitle_->setObjectName("connectionTitle");
    connectionSubtitle_ = new QLabel("优先尝试最近成功 IP，然后尝试 ssh ch@172.20.10.6。", header);
    connectionSubtitle_->setObjectName("connectionSubtitle");
    connectionSubtitle_->setWordWrap(true);

    auto* readyRow = new QHBoxLayout();
    readyDot_ = new QLabel(header);
    readyDot_->setObjectName("readyDot");
    readyDot_->setProperty("status", "connecting");
    readyDot_->setFixedSize(16, 16);
    readyText_ = new QLabel("连接中：等待树莓派状态", header);
    readyText_->setObjectName("readyText");
    readyRow->addWidget(readyDot_, 0, Qt::AlignLeft);
    readyRow->addWidget(readyText_, 1);

    actionBanner_ = new QLabel("⏳ 正在建立连接...", header);
    actionBanner_->setObjectName("actionBanner");
    actionBanner_->setWordWrap(true);

    lanWarning_ = new QLabel(header);
    lanWarning_->setObjectName("connectionSubtitle");
    lanWarning_->setWordWrap(true);
    lanWarning_->hide();

    layout->addWidget(connectionTitle_);
    layout->addWidget(connectionSubtitle_);
    readyDot_->hide();
    readyText_->hide();
    layout->addWidget(actionBanner_);
    layout->addWidget(lanWarning_);
    rightSide->addWidget(header);
}

void MainWindow::buildPages(QVBoxLayout* rightSide) {
    stack_ = new QStackedWidget(this);
    chatPage_ = new ChatPage(stack_);
    historyPage_ = new HistoryPage(stack_);
    hardwarePage_ = new HardwarePage(stack_);
    cameraPage_ = new CameraPage(stack_);
    logsPage_ = new LogsPage(stack_);
    settingsPage_ = new SettingsPage(config_, stack_);
    stack_->addWidget(chatPage_);
    stack_->addWidget(historyPage_);
    stack_->addWidget(hardwarePage_);
    stack_->addWidget(cameraPage_);
    stack_->addWidget(logsPage_);
    stack_->addWidget(settingsPage_);
    rightSide->addWidget(stack_, 1);
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
    stack_->setCurrentWidget(page);
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
        connectionSubtitle_->setText("Ping 正常，但 SSH 免密登录失败。请检查 SSH key 或 authorized_keys。");
    } else if (!state.activeHost.isEmpty()) {
        connectionTitle_->setText("正在检测树莓派：" + state.activeHost);
        connectionSubtitle_->setText("先检测网络可达性，再检测 SSH。");
    } else {
        connectionTitle_->setText("树莓派未连接");
        connectionSubtitle_->setText("请在设置页输入 ssh ch@ip，然后点击重新连接。");
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
    readyDot_->setProperty("status", statusClass);
    readyDot_->style()->unpolish(readyDot_);
    readyDot_->style()->polish(readyDot_);

    readyText_->setText(state.assistantStatusText.isEmpty() ? "等待树莓派状态" : state.assistantStatusText);
    actionBanner_->setProperty("status", statusClass);
    actionBanner_->style()->unpolish(actionBanner_);
    actionBanner_->style()->polish(actionBanner_);
    actionBanner_->setText(state.assistantStatusText.isEmpty() ? "⏳ 等待状态刷新..." : state.assistantStatusText);

    lanWarning_->setVisible(!state.warning.isEmpty());
    lanWarning_->setText(state.warning);
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
    sidebar_->setFixedWidth(compact ? 92 : 220);
    for (auto it = navButtons_.begin(); it != navButtons_.end(); ++it) {
        it.value()->setMinimumHeight(compact ? 46 : 42);
    }
}

void MainWindow::setWatchLive(bool enabled) {
    watchLive_ = enabled;
    watchButton_->setText(enabled ? "实时监听：开" : "实时监听：关");
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
    auto* button = new QPushButton(text, sidebar_);
    button->setObjectName("navButton");
    button->setProperty("active", false);
    QObject::connect(button, &QPushButton::clicked, this, [this, key]() {
        switchPage(key);
    });
    navButtons_.insert(key, button);
    return button;
}
