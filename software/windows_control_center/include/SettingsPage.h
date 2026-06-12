#pragma once

#include "AppConfig.h"
#include "BasePage.h"

#include <functional>

class QLineEdit;
class QComboBox;
class QPushButton;
class QScrollArea;
class QShowEvent;

class SettingsPage : public BasePage {
public:
    explicit SettingsPage(AppConfig& config, QWidget* parent = nullptr);
    void saveToConfig();
    void setServiceActions(std::function<void()> reconnect,
                           std::function<void()> restart,
                           std::function<void()> start,
                           std::function<void()> stop,
                           std::function<void()> toggleWatch);
    void setWatchLiveState(bool enabled);

protected:
    void showEvent(QShowEvent* event) override;

private:
    AppConfig& config_;
    QLineEdit* userEdit_ = nullptr;
    QLineEdit* fallbackIpEdit_ = nullptr;
    QLineEdit* manualCommandEdit_ = nullptr;
    QLineEdit* projectPathEdit_ = nullptr;
    QComboBox* authModeCombo_ = nullptr;
    QPushButton* watchButton_ = nullptr;
    QScrollArea* scroll_ = nullptr;
    std::function<void()> reconnectAction_;
    std::function<void()> restartAction_;
    std::function<void()> startAction_;
    std::function<void()> stopAction_;
    std::function<void()> toggleWatchAction_;
};
