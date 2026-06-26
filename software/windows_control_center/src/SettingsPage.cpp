#include "SettingsPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShowEvent>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <utility>

SettingsPage::SettingsPage(AppConfig& config, QWidget* parent)
    : BasePage("设置", "配置 SSH 连接和树莓派项目路径。", parent), config_(config) {
    scroll_ = new QScrollArea(this);
    scroll_->setObjectName("settingsScroll");
    scroll_->setWidgetResizable(true);
    scroll_->setFrameShape(QFrame::NoFrame);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_->setAttribute(Qt::WA_TranslucentBackground, true);
    scroll_->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);

    auto* content = new QWidget(scroll_);
    content->setObjectName("settingsContent");
    content->setAttribute(Qt::WA_TranslucentBackground, true);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(2, 2, 8, 8);
    contentLayout->setSpacing(18);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);
    form->setContentsMargins(18, 24, 18, 18);

    userEdit_ = new QLineEdit(config_.username, this);
    fallbackIpEdit_ = new QLineEdit(config_.fallbackIp, this);
    manualCommandEdit_ = new QLineEdit(config_.manualSshCommand, this);
    projectPathEdit_ = new QLineEdit(config_.projectPath, this);
    deepSeekBaseUrlEdit_ = new QLineEdit(config_.deepSeekBaseUrl, this);
    deepSeekModelEdit_ = new QLineEdit(config_.deepSeekModel, this);
    deepSeekApiKeyFileEdit_ = new QLineEdit(config_.deepSeekApiKeyFile, this);
    qwenBaseUrlEdit_ = new QLineEdit(config_.qwenVisionBaseUrl, this);
    qwenModelEdit_ = new QLineEdit(config_.qwenVisionModel, this);
    qwenApiKeyFileEdit_ = new QLineEdit(config_.qwenVisionApiKeyFile, this);

    authModeCombo_ = new QComboBox(this);
    authModeCombo_->addItem("密码登录", "password");
    authModeCombo_->addItem("密钥登录", "ssh-key");
    const int authIndex = authModeCombo_->findData(config_.authMode);
    authModeCombo_->setCurrentIndex(authIndex >= 0 ? authIndex : 0);

    form->addRow("SSH 用户", userEdit_);
    form->addRow("默认 IP", fallbackIpEdit_);
    form->addRow("手动 SSH 命令", manualCommandEdit_);
    form->addRow("项目路径", projectPathEdit_);
    form->addRow("认证方式", authModeCombo_);
    form->addRow("DeepSeek Base URL", deepSeekBaseUrlEdit_);
    form->addRow("DeepSeek Model", deepSeekModelEdit_);
    form->addRow("DeepSeek Key 文件", deepSeekApiKeyFileEdit_);
    form->addRow("Qwen Base URL", qwenBaseUrlEdit_);
    form->addRow("Qwen Vision Model", qwenModelEdit_);
    form->addRow("Qwen Key 文件", qwenApiKeyFileEdit_);

    auto* save = new QPushButton("保存设置", this);
    save->setObjectName("primaryButton");
    QObject::connect(save, &QPushButton::clicked, this, [this]() {
        saveToConfig();
    });

    auto* connectionGroup = new QGroupBox("连接配置", this);
    connectionGroup->setObjectName("settingsGroup");
    connectionGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connectionGroup->setLayout(form);

    auto* serviceGroup = new QGroupBox("服务与刷新", this);
    serviceGroup->setObjectName("settingsGroup");
    serviceGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto* actions = new QGridLayout(serviceGroup);
    actions->setContentsMargins(16, 20, 16, 16);
    actions->setHorizontalSpacing(12);
    actions->setVerticalSpacing(12);

    auto makeAction = [this, actions](const QString& text, int row, int column, auto callback) {
        auto* button = new QPushButton(text, this);
        button->setObjectName("secondaryButton");
        QObject::connect(button, &QPushButton::clicked, this, callback);
        actions->addWidget(button, row, column);
        return button;
    };
    makeAction("重新连接", 0, 0, [this]() { if (reconnectAction_) reconnectAction_(); });
    makeAction("重启树莓派服务", 0, 1, [this]() { if (restartAction_) restartAction_(); });
    makeAction("启动服务", 1, 0, [this]() { if (startAction_) startAction_(); });
    makeAction("停止服务", 1, 1, [this]() { if (stopAction_) stopAction_(); });
    watchButton_ = makeAction("实时监听：开", 2, 0, [this]() { if (toggleWatchAction_) toggleWatchAction_(); });
    actions->addWidget(save, 2, 1);

    contentLayout->addWidget(connectionGroup);
    contentLayout->addWidget(serviceGroup);
    contentLayout->addStretch(1);
    scroll_->setWidget(content);
    bodyLayout()->addWidget(scroll_, 1);
}

void SettingsPage::showEvent(QShowEvent* event) {
    BasePage::showEvent(event);
    // 中文注释：每次进入设置页都从第一项开始，避免保留旧滚动位置造成字段看似丢失。
    if (scroll_ && scroll_->verticalScrollBar()) {
        scroll_->verticalScrollBar()->setValue(0);
    }
}

void SettingsPage::setServiceActions(std::function<void()> reconnect,
                                     std::function<void()> restart,
                                     std::function<void()> start,
                                     std::function<void()> stop,
                                     std::function<void()> toggleWatch) {
    reconnectAction_ = std::move(reconnect);
    restartAction_ = std::move(restart);
    startAction_ = std::move(start);
    stopAction_ = std::move(stop);
    toggleWatchAction_ = std::move(toggleWatch);
}

void SettingsPage::setWatchLiveState(bool enabled) {
    if (watchButton_) {
        watchButton_->setText(enabled ? "实时监听：开" : "实时监听：关");
    }
}

void SettingsPage::saveToConfig() {
    config_.username = userEdit_->text().trimmed();
    config_.fallbackIp = fallbackIpEdit_->text().trimmed();
    config_.manualSshCommand = manualCommandEdit_->text().trimmed();
    config_.projectPath = projectPathEdit_->text().trimmed();
    config_.authMode = authModeCombo_->currentData().toString();
    config_.deepSeekBaseUrl = deepSeekBaseUrlEdit_->text().trimmed();
    config_.deepSeekModel = deepSeekModelEdit_->text().trimmed();
    config_.deepSeekApiKeyFile = deepSeekApiKeyFileEdit_->text().trimmed();
    config_.qwenVisionBaseUrl = qwenBaseUrlEdit_->text().trimmed();
    config_.qwenVisionModel = qwenModelEdit_->text().trimmed();
    config_.qwenVisionApiKeyFile = qwenApiKeyFileEdit_->text().trimmed();
    config_.save();
}
