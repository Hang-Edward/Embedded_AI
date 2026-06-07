#include "SettingsPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

SettingsPage::SettingsPage(AppConfig& config, QWidget* parent)
    : BasePage("Settings", "SSH connection and Raspberry Pi project paths.", parent), config_(config) {
    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    userEdit_ = new QLineEdit(config_.username, this);
    fallbackIpEdit_ = new QLineEdit(config_.fallbackIp, this);
    manualCommandEdit_ = new QLineEdit(config_.manualSshCommand, this);
    projectPathEdit_ = new QLineEdit(config_.projectPath, this);
    authModeCombo_ = new QComboBox(this);
    authModeCombo_->addItems({"password", "ssh-key"});
    authModeCombo_->setCurrentText(config_.authMode);

    form->addRow("SSH user", userEdit_);
    form->addRow("Fallback IP", fallbackIpEdit_);
    form->addRow("Manual SSH command", manualCommandEdit_);
    form->addRow("Project path", projectPathEdit_);
    form->addRow("Auth mode", authModeCombo_);

    auto* save = new QPushButton("Save settings", this);
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
    config_.authMode = authModeCombo_->currentText();
    config_.save();
}
