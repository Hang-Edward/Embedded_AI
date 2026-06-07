#include "MainWindow.h"

#include "CameraPage.h"
#include "ChatPage.h"
#include "HardwarePage.h"
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
    setWindowTitle("Embedded AI Reality Bridge");
    resize(1280, 820);
    setMinimumSize(900, 620);
    qApp->setStyleSheet(Theme::styleSheet());
    buildUi();
    liveTimer_ = new QTimer(this);
    liveTimer_->setInterval(5000);
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

    auto* title = new QLabel("Embedded AI\nControl Center", sidebar_);
    title->setObjectName("appTitle");
    side->addWidget(title);
    side->addSpacing(18);

    side->addWidget(makeNavButton("chat", "Chat"));
    side->addWidget(makeNavButton("hardware", "Hardware"));
    side->addWidget(makeNavButton("camera", "Camera"));
    side->addWidget(makeNavButton("logs", "Logs"));
    side->addWidget(makeNavButton("settings", "Settings"));
    side->addStretch(1);

    auto* reconnect = new QPushButton("Reconnect", sidebar_);
    reconnect->setObjectName("primaryButton");
    QObject::connect(reconnect, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.reconnect();
    });
    side->addWidget(reconnect);

    auto* restartService = new QPushButton("Restart Service", sidebar_);
    restartService->setObjectName("primaryButton");
    QObject::connect(restartService, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.restartPiService();
    });
    side->addWidget(restartService);

    watchButton_ = new QPushButton("Watch Live: On", sidebar_);
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
    layout->setSpacing(4);
    connectionTitle_ = new QLabel("Connecting to Raspberry Pi...", header);
    connectionTitle_->setObjectName("connectionTitle");
    connectionSubtitle_ = new QLabel("Trying last successful IP, then ssh ch@172.20.10.6.", header);
    connectionSubtitle_->setObjectName("connectionSubtitle");
    connectionSubtitle_->setWordWrap(true);
    lanWarning_ = new QLabel(header);
    lanWarning_->setObjectName("connectionSubtitle");
    lanWarning_->setWordWrap(true);
    lanWarning_->hide();
    layout->addWidget(connectionTitle_);
    layout->addWidget(connectionSubtitle_);
    layout->addWidget(lanWarning_);
    rightSide->addWidget(header);
}

void MainWindow::buildPages(QVBoxLayout* rightSide) {
    stack_ = new QStackedWidget(this);
    chatPage_ = new ChatPage(stack_);
    hardwarePage_ = new HardwarePage(stack_);
    cameraPage_ = new CameraPage(stack_);
    logsPage_ = new LogsPage(stack_);
    settingsPage_ = new SettingsPage(config_, stack_);
    stack_->addWidget(chatPage_);
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
        connectionTitle_->setText("Connected: ssh ch@" + state.activeHost);
        connectionSubtitle_->setText("Service and hardware status are being checked over SSH.");
    } else if (state.piReachable) {
        connectionTitle_->setText("Raspberry Pi reachable: " + state.activeHost);
        connectionSubtitle_->setText("Ping is OK, but SSH key-based login failed. Configure SSH key or add password login support.");
    } else if (!state.activeHost.isEmpty()) {
        connectionTitle_->setText("Checking Raspberry Pi: " + state.activeHost);
        connectionSubtitle_->setText("Testing network reachability first, then SSH.");
    } else {
        connectionTitle_->setText("Raspberry Pi not connected");
        connectionSubtitle_->setText("Enter a command such as ssh ch@172.20.10.6 in Settings, then press Reconnect.");
    }
    lanWarning_->setVisible(!state.warning.isEmpty());
    lanWarning_->setText(state.warning);
    hardwarePage_->setState(state);
    logsPage_->setLogText(state.logText);
    cameraPage_->setImagePath(state.localFramePath);
    if (state.sshOnline) {
        chatPage_->setLatestSession(state.logText, state.localFramePath);
        if (watchLive_ && !liveTimer_->isActive()) {
            liveTimer_->start();
        }
    } else if (liveTimer_->isActive()) {
        liveTimer_->stop();
    }
}

void MainWindow::updateResponsiveMode() {
    const bool compact = width() < 1040;
    sidebar_->setFixedWidth(compact ? 84 : 220);
    for (auto it = navButtons_.begin(); it != navButtons_.end(); ++it) {
        it.value()->setMinimumHeight(compact ? 46 : 42);
    }
}

void MainWindow::setWatchLive(bool enabled) {
    watchLive_ = enabled;
    watchButton_->setText(enabled ? "Watch Live: On" : "Watch Live: Off");
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
