#include "SettingsPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

SettingsPage::SettingsPage(AppConfig& config, QWidget* parent)
    : BasePage("设置", "配置 SSH 连接和树莓派项目路径。", parent), config_(config) {
    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);

    userEdit_ = new QLineEdit(config_.username, this);
    fallbackIpEdit_ = new QLineEdit(config_.fallbackIp, this);
    manualCommandEdit_ = new QLineEdit(config_.manualSshCommand, this);
    projectPathEdit_ = new QLineEdit(config_.projectPath, this);

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

    auto* save = new QPushButton("保存设置", this);
    save->setObjectName("primaryButton");
    QObject::connect(save, &QPushButton::clicked, this, [this]() {
        saveToConfig();
    });

    bodyLayout()->addLayout(form);
    bodyLayout()->addWidget(save, 0, Qt::AlignRight);
    bodyLayout()->addStretch(1);
}

void SettingsPage::saveToConfig() {
    config_.username = userEdit_->text().trimmed();
    config_.fallbackIp = fallbackIpEdit_->text().trimmed();
    config_.manualSshCommand = manualCommandEdit_->text().trimmed();
    config_.projectPath = projectPathEdit_->text().trimmed();
    config_.authMode = authModeCombo_->currentData().toString();
    config_.save();
}
