#pragma once

#include "BasePage.h"
#include "ConnectionState.h"
#include "SystemSelfTest.h"

class QGridLayout;
class QLabel;
class QProgressBar;
class QPushButton;
class QTreeWidget;

class HardwarePage : public BasePage {
public:
    explicit HardwarePage(AppConfig& config, QWidget* parent = nullptr);
    void setState(const ConnectionState& state);

private:
    void startFullCheck();
    void updateCheckRow(const SelfTestCheck& check);
    void finishFullCheck(const SelfTestReport& report);
    void exportReport(bool jsonFormat);

    AppConfig& config_;
    ConnectionState currentState_;
    SystemSelfTestRunner runner_;
    SelfTestReport latestReport_;
    QGridLayout* grid_ = nullptr;
    QLabel* reportSummary_ = nullptr;
    QProgressBar* progress_ = nullptr;
    QPushButton* runButton_ = nullptr;
    QPushButton* exportJsonButton_ = nullptr;
    QPushButton* exportTextButton_ = nullptr;
    QTreeWidget* resultTree_ = nullptr;
    QString lastStateKey_;
};
