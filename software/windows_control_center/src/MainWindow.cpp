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
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
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

QString inspectorCardHtml(const QString& label, const QString& value) {
    const QString safeLabel = label.toHtmlEscaped();
    const QString safeValue = value.toHtmlEscaped().replace("\n", "<br/>");
    return QStringLiteral(
               "<div style='font-size:11px;font-weight:700;color:rgba(214,233,255,0.76);letter-spacing:0.4px;'>%1</div>"
               "<div style='margin-top:6px;font-size:14px;font-weight:700;color:#eef6ff;line-height:1.45;'>%2</div>")
        .arg(safeLabel, safeValue);
}

QString inspectorHeadlineHtml(const QString& label, const QString& value) {
    const QString safeLabel = label.toHtmlEscaped();
    const QString safeValue = value.toHtmlEscaped().replace("\n", "<br/>");
    return QStringLiteral(
               "<div style='font-size:11px;font-weight:700;color:rgba(214,233,255,0.74);letter-spacing:0.4px;'>%1</div>"
               "<div style='margin-top:7px;font-size:15px;font-weight:800;color:#f4f8ff;line-height:1.48;'>%2</div>")
        .arg(safeLabel, safeValue);
}

void animateWidgetRefresh(QWidget* widget, int duration = 240) {
    if (widget == nullptr) {
        return;
    }
    widget->setProperty("flash", true);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    QPointer<QWidget> guard(widget);
    QTimer::singleShot(duration, widget, [guard]() {
        if (guard == nullptr) {
            return;
        }
        guard->setProperty("flash", false);
        guard->style()->unpolish(guard);
        guard->style()->polish(guard);
        guard->update();
    });
}

void refreshLayoutAround(QWidget* widget) {
    QWidget* current = widget;
    int depth = 0;
    while (current != nullptr && depth < 5) {
        current->updateGeometry();
        if (current->layout() != nullptr) {
            current->layout()->invalidate();
            current->layout()->activate();
        }
        current = current->parentWidget();
        ++depth;
    }
}

void setAnimatedLabelText(QLabel* label, const QString& text, bool richText = false, int duration = 240) {
    if (label == nullptr) {
        return;
    }
    const QString oldText = label->text();
    if (richText) {
        label->setTextFormat(Qt::RichText);
    } else {
        label->setTextFormat(Qt::PlainText);
    }
    if (oldText == text) {
        return;
    }
    label->setText(text);
    label->updateGeometry();
    refreshLayoutAround(label);
    animateWidgetRefresh(label, duration);
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), connection_(config_, this) {
    config_.load();
    setWindowTitle(QStringLiteral("Embedded AI Reality Bridge"));
    resize(1540, 980);
    setMinimumSize(1240, 820);
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
    sidebar_->setFixedWidth(312);
    auto* side = new QVBoxLayout(sidebar_);
    side->setContentsMargins(18, 18, 18, 18);
    side->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Embedded AI\nReality Bridge"), sidebar_);
    title->setObjectName(QStringLiteral("appTitle"));
    side->addWidget(title);
    auto* sectionTag = new QLabel(QStringLiteral("AI DESKTOP WORKBENCH"), sidebar_);
    sectionTag->setObjectName(QStringLiteral("sidebarTag"));
    side->addWidget(sectionTag);

    auto* summaryCard = new QWidget(sidebar_);
    summaryCard->setObjectName(QStringLiteral("sidebarSummaryCard"));
    auto* summaryLayout = new QVBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(14, 14, 14, 14);
    summaryLayout->setSpacing(6);
    sidebarSummaryTitle_ = new QLabel(QStringLiteral("等待桥接"), summaryCard);
    sidebarSummaryTitle_->setObjectName(QStringLiteral("sidebarSummaryTitle"));
    sidebarSummaryBody_ = new QLabel(QStringLiteral("Windows 控制端正在等待树莓派、硬件触发器和摄像头同步。"), summaryCard);
    sidebarSummaryBody_->setObjectName(QStringLiteral("sidebarSummaryBody"));
    sidebarSummaryBody_->setWordWrap(true);
    summaryLayout->addWidget(sidebarSummaryTitle_);
    summaryLayout->addWidget(sidebarSummaryBody_);
    side->addWidget(summaryCard);

    auto* navLabel = new QLabel(QStringLiteral("工作区"), sidebar_);
    navLabel->setObjectName(QStringLiteral("sidebarSectionLabel"));
    side->addWidget(navLabel);

    side->addWidget(makeNavButton(QStringLiteral("chat"), QStringLiteral("实时对话"), QStringLiteral("当前轮次、图像与 AI 回复"), QStringLiteral("◉")));
    side->addWidget(makeNavButton(QStringLiteral("history"), QStringLiteral("历史记录"), QStringLiteral("查看最近完成的分析记录"), QStringLiteral("↺")));
    side->addWidget(makeNavButton(QStringLiteral("hardware"), QStringLiteral("连接诊断"), QStringLiteral("硬件、串口与服务链路状态"), QStringLiteral("◎")));
    side->addWidget(makeNavButton(QStringLiteral("camera"), QStringLiteral("摄像头画面"), QStringLiteral("检查当前同步回来的现场画面"), QStringLiteral("◌")));
    side->addWidget(makeNavButton(QStringLiteral("logs"), QStringLiteral("原始日志"), QStringLiteral("读取树莓派侧完整执行日志"), QStringLiteral("⋯")));
    side->addWidget(makeNavButton(QStringLiteral("settings"), QStringLiteral("设置"), QStringLiteral("SSH、项目目录与服务控制"), QStringLiteral("⚙")));
    side->addStretch(1);

    auto* reconnectWrap = new QWidget(sidebar_);
    reconnectWrap->setObjectName(QStringLiteral("sidebarReconnectWrap"));
    auto* reconnectLayout = new QVBoxLayout(reconnectWrap);
    reconnectLayout->setContentsMargins(0, 0, 0, 0);
    reconnectLayout->setSpacing(8);
    auto* reconnectLabel = new QLabel(QStringLiteral("快速操作"), reconnectWrap);
    reconnectLabel->setObjectName(QStringLiteral("sidebarSectionLabel"));
    auto* reconnect = new QPushButton(QStringLiteral("重新连接"), reconnectWrap);
    reconnect->setObjectName(QStringLiteral("primaryButton"));
    QObject::connect(reconnect, &QPushButton::clicked, this, [this]() {
        settingsPage_->saveToConfig();
        connection_.reconnect();
    });
    reconnectLayout->addWidget(reconnectLabel);
    reconnectLayout->addWidget(reconnect);
    side->addWidget(reconnectWrap);

    root->addWidget(sidebar_);
}

void MainWindow::buildHeader(QVBoxLayout* rightSide) {
    auto* header = new GlassSurface(GlassSurface::Tone::Elevated, central_);
    header->setObjectName(QStringLiteral("glassHeader"));
    auto* layout = new QHBoxLayout(header);
    layout->setContentsMargins(22, 14, 22, 14);
    layout->setSpacing(12);

    auto* titleWrap = new QWidget(header);
    auto* titleLayout = new QVBoxLayout(titleWrap);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(3);

    heroEyebrow_ = new QLabel(QStringLiteral("AI 现实桥接控制台"), titleWrap);
    heroEyebrow_->setObjectName(QStringLiteral("heroEyebrow"));

    connectionTitle_ = new QLabel(QStringLiteral("正在连接树莓派..."), titleWrap);
    connectionTitle_->setObjectName(QStringLiteral("connectionTitle"));
    connectionSubtitle_ = new QLabel(
        QStringLiteral("优先尝试最近一次成功 IP，然后尝试 ssh ch@172.20.10.6。"),
        titleWrap);
    connectionSubtitle_->setObjectName(QStringLiteral("connectionSubtitle"));
    connectionSubtitle_->setWordWrap(false);
    connectionSubtitle_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    titleLayout->addWidget(heroEyebrow_);
    titleLayout->addWidget(connectionTitle_);
    titleLayout->addWidget(connectionSubtitle_);

    auto* headerMetrics = new QWidget(header);
    headerMetrics->setObjectName(QStringLiteral("headerMetrics"));
    auto* metricsLayout = new QHBoxLayout(headerMetrics);
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setSpacing(10);
    headerHostPill_ = new QLabel(QStringLiteral("等待主机"), headerMetrics);
    headerHostPill_->setObjectName(QStringLiteral("headerMetaPill"));
    headerStatusPill_ = new QLabel(QStringLiteral("连接检测中"), headerMetrics);
    headerStatusPill_->setObjectName(QStringLiteral("headerStatusPill"));
    metricsLayout->addWidget(headerHostPill_);
    metricsLayout->addWidget(headerStatusPill_);

    layout->addWidget(titleWrap, 1);
    layout->addWidget(headerMetrics, 0, Qt::AlignVCenter | Qt::AlignRight);
    rightSide->addWidget(header);
}

void MainWindow::buildPages(QVBoxLayout* rightSide) {
    auto* contentRow = new QHBoxLayout();
    contentRow->setContentsMargins(0, 0, 0, 0);
    contentRow->setSpacing(16);

    pageSurface_ = new GlassSurface(GlassSurface::Tone::Regular, central_);
    pageSurface_->setObjectName(QStringLiteral("mainContentSurface"));
    auto* surfaceLayout = new QVBoxLayout(pageSurface_);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    stack_ = new TransparentStackedWidget(pageSurface_);
    stack_->setObjectName(QStringLiteral("pageStack"));
    chatPage_ = new ChatPage(config_, stack_);
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
    statusSurface_->setFixedWidth(330);
    auto* statusSurfaceLayout = new QVBoxLayout(statusSurface_);
    statusSurfaceLayout->setContentsMargins(0, 0, 0, 0);

    auto* statusScroll = new QScrollArea(statusSurface_);
    statusScroll->setObjectName(QStringLiteral("statusScroll"));
    statusScroll->setWidgetResizable(true);
    statusScroll->setFrameShape(QFrame::NoFrame);
    statusScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    statusScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    statusScroll->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    statusScroll->viewport()->setAutoFillBackground(false);

    auto* statusHost = new QWidget(statusScroll);
    statusHost->setObjectName(QStringLiteral("statusHost"));
    statusHost->setAttribute(Qt::WA_TranslucentBackground, true);
    statusHost->setAutoFillBackground(false);
    auto* statusLayout = new QVBoxLayout(statusHost);
    statusLayout->setContentsMargins(18, 18, 18, 18);
    statusLayout->setSpacing(10);

    auto* statusHeader = new QWidget(statusSurface_);
    statusHeader->setObjectName(QStringLiteral("statusColumnHeader"));
    auto* statusHeaderLayout = new QHBoxLayout(statusHeader);
    statusHeaderLayout->setContentsMargins(0, 0, 0, 0);
    statusHeaderLayout->setSpacing(10);

    statusBeacon_ = new QLabel(statusHeader);
    statusBeacon_->setObjectName(QStringLiteral("statusBeacon"));
    statusBeacon_->setFixedSize(14, 14);

    auto* statusTextWrap = new QWidget(statusHeader);
    auto* statusTextLayout = new QVBoxLayout(statusTextWrap);
    statusTextLayout->setContentsMargins(0, 0, 0, 0);
    statusTextLayout->setSpacing(2);

    auto* statusTitle = new QLabel(QStringLiteral("系统状态总览"), statusTextWrap);
    statusTitle->setObjectName(QStringLiteral("statusColumnTitle"));
    auto* statusSubtitle = new QLabel(QStringLiteral("把链路、阶段与恢复建议收拢在这里，让主舞台只保留当前对话与结果。"), statusTextWrap);
    statusSubtitle->setObjectName(QStringLiteral("statusColumnSubtitle"));
    statusSubtitle->setWordWrap(true);
    statusSubtitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    statusTextLayout->addWidget(statusTitle);
    statusTextLayout->addWidget(statusSubtitle);
    statusHeaderLayout->addWidget(statusBeacon_, 0, Qt::AlignTop);
    statusHeaderLayout->addWidget(statusTextWrap, 1);

    actionBanner_ = new QLabel(QStringLiteral("正在自动检测树莓派、服务、摄像头和日志..."), statusSurface_);
    actionBanner_->setObjectName(QStringLiteral("actionBanner"));
    actionBanner_->setWordWrap(true);
    actionBanner_->setMargin(11);
    actionBanner_->setMinimumHeight(46);
    actionBanner_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    healthHeadline_ = new QLabel(QStringLiteral("等待第一轮系统诊断结果"), statusSurface_);
    healthHeadline_->setObjectName(QStringLiteral("healthHeadline"));
    healthHeadline_->setWordWrap(true);
    healthHeadline_->setMargin(11);
    healthHeadline_->setMinimumHeight(54);
    healthHeadline_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    networkChip_ = new QLabel(statusSurface_);
    phaseChip_ = new QLabel(statusSurface_);
    triggerChip_ = new QLabel(statusSurface_);
    modeChip_ = new QLabel(statusSurface_);
    nextActionCard_ = new QLabel(statusSurface_);
    const QList<QLabel*> chips {networkChip_, phaseChip_, triggerChip_, modeChip_};
    for (QLabel* chip : chips) {
        chip->setObjectName(QStringLiteral("metricChip"));
        chip->setWordWrap(true);
        chip->setMargin(10);
        chip->setMinimumHeight(54);
        chip->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    }
    nextActionCard_->setObjectName(QStringLiteral("statusActionCard"));
    nextActionCard_->setWordWrap(true);
    nextActionCard_->setMargin(12);
    nextActionCard_->setMinimumHeight(68);
    nextActionCard_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    lanWarning_ = new QLabel(statusHost);
    lanWarning_->setObjectName(QStringLiteral("statusHint"));
    lanWarning_->setWordWrap(true);
    lanWarning_->setMargin(6);
    lanWarning_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    lanWarning_->hide();

    statusLayout->addWidget(statusHeader);
    statusLayout->addWidget(actionBanner_);
    statusLayout->addWidget(healthHeadline_);
    sectionPrimaryToggle_ = new QToolButton(statusHost);
    sectionPrimaryToggle_->setObjectName(QStringLiteral("inspectorToggle"));
    sectionPrimaryToggle_->setText(QStringLiteral("链路与阶段"));
    sectionPrimaryToggle_->setCheckable(true);
    sectionPrimaryToggle_->setChecked(true);
    sectionPrimaryToggle_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sectionPrimaryToggle_->setArrowType(Qt::DownArrow);

    sectionPrimaryContainer_ = new QWidget(statusHost);
    sectionPrimaryContainer_->setObjectName(QStringLiteral("inspectorSectionBody"));
    auto* primaryLayout = new QVBoxLayout(sectionPrimaryContainer_);
    primaryLayout->setContentsMargins(0, 0, 0, 0);
    primaryLayout->setSpacing(10);
    primaryLayout->addWidget(networkChip_);
    primaryLayout->addWidget(phaseChip_);
    primaryLayout->addWidget(triggerChip_);
    primaryLayout->addWidget(modeChip_);

    sectionRecoveryToggle_ = new QToolButton(statusHost);
    sectionRecoveryToggle_->setObjectName(QStringLiteral("inspectorToggle"));
    sectionRecoveryToggle_->setText(QStringLiteral("建议与恢复"));
    sectionRecoveryToggle_->setCheckable(true);
    sectionRecoveryToggle_->setChecked(true);
    sectionRecoveryToggle_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sectionRecoveryToggle_->setArrowType(Qt::DownArrow);

    sectionRecoveryContainer_ = new QWidget(statusHost);
    sectionRecoveryContainer_->setObjectName(QStringLiteral("inspectorSectionBody"));
    auto* recoveryLayout = new QVBoxLayout(sectionRecoveryContainer_);
    recoveryLayout->setContentsMargins(0, 0, 0, 0);
    recoveryLayout->setSpacing(10);
    recoveryLayout->addWidget(nextActionCard_);

    QObject::connect(sectionPrimaryToggle_, &QToolButton::toggled, this, [this](bool checked) {
        setInspectorSectionExpanded(sectionPrimaryToggle_, sectionPrimaryContainer_, checked);
    });
    QObject::connect(sectionRecoveryToggle_, &QToolButton::toggled, this, [this](bool checked) {
        setInspectorSectionExpanded(sectionRecoveryToggle_, sectionRecoveryContainer_, checked);
    });

    statusLayout->addWidget(sectionPrimaryToggle_);
    statusLayout->addWidget(sectionPrimaryContainer_);
    statusLayout->addWidget(sectionRecoveryToggle_);
    statusLayout->addWidget(sectionRecoveryContainer_);
    statusLayout->addStretch(1);

    statusScroll->setWidget(statusHost);
    statusSurfaceLayout->addWidget(statusScroll);

    contentRow->addWidget(pageSurface_, 14);
    contentRow->addWidget(statusSurface_, 4);
    rightSide->addLayout(contentRow, 1);
    switchPage(QStringLiteral("chat"));
    startBeaconPulse();
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
        setAnimatedLabelText(connectionTitle_, QStringLiteral("已连接：ssh ch@") + state.activeHost, false, 220);
        setAnimatedLabelText(connectionSubtitle_, QStringLiteral("正在通过 SSH 自动刷新服务、硬件、日志和最新图片。"), false, 220);
    } else if (state.piReachable) {
        setAnimatedLabelText(connectionTitle_, QStringLiteral("树莓派网络可达：") + state.activeHost, false, 220);
        setAnimatedLabelText(connectionSubtitle_, QStringLiteral("Ping 正常，但 SSH 握手失败。请检查 SSH key 或 authorized_keys。"), false, 220);
    } else if (!state.activeHost.isEmpty()) {
        setAnimatedLabelText(connectionTitle_, QStringLiteral("正在检测树莓派：") + state.activeHost, false, 220);
        setAnimatedLabelText(connectionSubtitle_, QStringLiteral("先检测网络可达性，再检测 SSH。"), false, 220);
    } else {
        setAnimatedLabelText(connectionTitle_, QStringLiteral("树莓派未连接"), false, 220);
        setAnimatedLabelText(connectionSubtitle_, QStringLiteral("请确认 PC 与树莓派在同一网络，或在设置页手动输入 ssh ch@ip 后重连。"), false, 220);
    }

    if (headerHostPill_ != nullptr) {
        const QString hostText = state.activeHost.isEmpty()
            ? QStringLiteral("主机：等待分配")
            : QStringLiteral("主机：%1").arg(state.activeHost);
        setAnimatedLabelText(headerHostPill_, hostText, false, 200);
    }

    const QString statusClass = statusClassFor(state);
    const QString cleanStatus = statusTextFor(state);
    actionBanner_->setProperty("status", statusClass);
    statusBeacon_->setProperty("status", statusClass);
    if (statusClass != lastStatusClass_) {
        actionBanner_->style()->unpolish(actionBanner_);
        actionBanner_->style()->polish(actionBanner_);
        statusBeacon_->style()->unpolish(statusBeacon_);
        statusBeacon_->style()->polish(statusBeacon_);
        lastStatusClass_ = statusClass;
        animateWidgetRefresh(actionBanner_, 260);
    }
    setAnimatedLabelText(actionBanner_, inspectorHeadlineHtml(QStringLiteral("实时状态"), cleanStatus), true, 260);
    if (headerStatusPill_ != nullptr) {
        setAnimatedLabelText(headerStatusPill_, cleanStatus, false, 220);
        headerStatusPill_->setProperty("status", statusClass);
        headerStatusPill_->style()->unpolish(headerStatusPill_);
        headerStatusPill_->style()->polish(headerStatusPill_);
    }
    setAnimatedLabelText(healthHeadline_, inspectorHeadlineHtml(QStringLiteral("系统判断"), healthHeadlineTextFor(state)), true, 250);
    if (sidebarSummaryTitle_ != nullptr && sidebarSummaryBody_ != nullptr) {
        setAnimatedLabelText(sidebarSummaryTitle_, phaseTextFor(state), false, 220);
        setAnimatedLabelText(sidebarSummaryBody_, nextActionTextFor(state), false, 220);
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
    const QString nextActionText = inspectorCardHtml(QStringLiteral("下一步建议"), nextActionTextFor(state));
    setAnimatedLabelText(nextActionCard_, nextActionText, true, 240);
    nextActionCard_->setProperty(
        "tone",
        state.sshOnline
            ? (state.assistantStatus == AssistantStatus::Listening
                   || state.assistantStatus == AssistantStatus::Thinking)
                ? QStringLiteral("busy")
                : QStringLiteral("ready")
            : QStringLiteral("warning"));
    nextActionCard_->style()->unpolish(nextActionCard_);
    nextActionCard_->style()->polish(nextActionCard_);
    updateInspectorFocus(state);

    lanWarning_->hide();
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
    sidebar_->setFixedWidth(compact ? 124 : 312);
    if (statusSurface_ != nullptr) {
        statusSurface_->setFixedWidth(width() < 1380 ? 304 : 330);
    }
    for (auto it = navButtons_.begin(); it != navButtons_.end(); ++it) {
        it.value()->setMinimumHeight(compact ? 74 : 92);
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

QPushButton* MainWindow::makeNavButton(const QString& key, const QString& text, const QString& subtitle, const QString& glyph) {
    auto* button = new LiquidNavButton(text, sidebar_);
    button->setProperty("active", false);
    button->setProperty("subtitle", subtitle);
    button->setProperty("glyph", glyph);
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
    const QString newText = inspectorCardHtml(label, value);
    const QString previousText = chip->text();
    const QString previousTone = chip->property("tone").toString();
    chip->setTextFormat(Qt::RichText);
    chip->setText(newText);
    chip->setProperty("tone", tone);
    chip->style()->unpolish(chip);
    chip->style()->polish(chip);
    if (previousText != newText || previousTone != tone) {
        animateWidgetRefresh(chip, 220);
    }
}

void MainWindow::setInspectorSectionExpanded(QToolButton* toggle, QWidget* container, bool expanded) {
    if (toggle == nullptr || container == nullptr) {
        return;
    }
    toggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    container->setVisible(expanded);
    container->updateGeometry();
    if (container->parentWidget() != nullptr && container->parentWidget()->layout() != nullptr) {
        container->parentWidget()->layout()->invalidate();
        container->parentWidget()->layout()->activate();
    }
}

void MainWindow::updateInspectorFocus(const ConnectionState& state) {
    const bool focusRecovery = !state.sshOnline
        || state.assistantStatus == AssistantStatus::Error
        || state.assistantStatus == AssistantStatus::Offline;

    if (sectionPrimaryToggle_ != nullptr) {
        sectionPrimaryToggle_->setProperty("focused", !focusRecovery);
        sectionPrimaryToggle_->style()->unpolish(sectionPrimaryToggle_);
        sectionPrimaryToggle_->style()->polish(sectionPrimaryToggle_);
    }
    if (sectionRecoveryToggle_ != nullptr) {
        sectionRecoveryToggle_->setProperty("focused", focusRecovery);
        sectionRecoveryToggle_->style()->unpolish(sectionRecoveryToggle_);
        sectionRecoveryToggle_->style()->polish(sectionRecoveryToggle_);
    }

    if (focusRecovery && sectionRecoveryToggle_ != nullptr && !sectionRecoveryToggle_->isChecked()) {
        sectionRecoveryToggle_->setChecked(true);
    }
    if (!focusRecovery && sectionPrimaryToggle_ != nullptr && !sectionPrimaryToggle_->isChecked()) {
        sectionPrimaryToggle_->setChecked(true);
    }
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

QString MainWindow::nextActionTextFor(const ConnectionState& state) const {
    if (!state.sshOnline) {
        return QStringLiteral("检查树莓派 IP 与 SSH 服务，然后点击重新连接。");
    }
    if (state.assistantStatus == AssistantStatus::Listening) {
        return QStringLiteral("继续说话，等待 5 秒录音完成后自动转入识别。");
    }
    if (state.assistantStatus == AssistantStatus::Thinking) {
        return QStringLiteral("保持画面稳定，等待语音识别与视觉分析结束。");
    }
    if (state.assistantStatus == AssistantStatus::Error) {
        return QStringLiteral("切到连接诊断或原始日志页，查看最近一次失败原因。");
    }
    return QStringLiteral("现在可以按三键键盘 K-B 发起下一次分析。");
}

QString MainWindow::healthHeadlineTextFor(const ConnectionState& state) const {
    if (!state.sshOnline) {
        return QStringLiteral("主链路尚未打通，系统仍处于桥接前状态。");
    }
    if (state.assistantStatus == AssistantStatus::Listening) {
        return QStringLiteral("硬件已接管输入，正在把现场语音转成可分析指令。");
    }
    if (state.assistantStatus == AssistantStatus::Thinking) {
        return QStringLiteral("多模态链路工作中：语音识别、图像理解与回答生成正在推进。");
    }
    if (state.assistantStatus == AssistantStatus::Ready) {
        return QStringLiteral("系统处于可演示状态，输入、拍照、分析与回传链路均已待命。");
    }
    if (state.assistantStatus == AssistantStatus::Error) {
        return QStringLiteral("系统捕获到中断或异常，请先恢复链路再继续演示。");
    }
    return QStringLiteral("系统正在刷新最新硬件与服务状态。");
}

void MainWindow::startBeaconPulse() {
    if (beaconTimer_ != nullptr) {
        return;
    }
    beaconTimer_ = new QTimer(this);
    beaconTimer_->setInterval(720);
    QObject::connect(beaconTimer_, &QTimer::timeout, this, [this]() {
        if (statusBeacon_ == nullptr) {
            return;
        }
        const bool dim = statusBeacon_->property("pulse").toString() == QStringLiteral("dim");
        statusBeacon_->setProperty("pulse", dim ? QStringLiteral("bright") : QStringLiteral("dim"));
        statusBeacon_->style()->unpolish(statusBeacon_);
        statusBeacon_->style()->polish(statusBeacon_);
    });
    statusBeacon_->setProperty("pulse", QStringLiteral("bright"));
    beaconTimer_->start();
}
