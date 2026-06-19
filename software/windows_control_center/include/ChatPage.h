#pragma once

#include "BasePage.h"
#include "ConnectionState.h"

class QPushButton;
class QVBoxLayout;
class QLabel;

class ChatPage : public BasePage {
public:
    explicit ChatPage(QWidget* parent = nullptr);
    void appendDemoConversation();
    void setLatestSession(const ConnectionState& state);

private:
    void clearMessages();
    void updateStagePanel(const ConnectionState& state);

    QVBoxLayout* messages_ = nullptr;
    QPushButton* triggerButton_ = nullptr;
    QLabel* stageTitle_ = nullptr;
    QLabel* stageStatus_ = nullptr;
    QLabel* stageMeta_ = nullptr;
    QString lastSessionKey_;
};
