#pragma once

#include "AppConfig.h"
#include "BasePage.h"

class QLineEdit;
class QComboBox;

class SettingsPage : public BasePage {
public:
    explicit SettingsPage(AppConfig& config, QWidget* parent = nullptr);
    void saveToConfig();

private:
    AppConfig& config_;
    QLineEdit* userEdit_ = nullptr;
    QLineEdit* fallbackIpEdit_ = nullptr;
    QLineEdit* manualCommandEdit_ = nullptr;
    QLineEdit* projectPathEdit_ = nullptr;
    QComboBox* authModeCombo_ = nullptr;
};
