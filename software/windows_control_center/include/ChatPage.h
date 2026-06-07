#pragma once

#include "BasePage.h"
#include "ConnectionState.h"

class QPushButton;
class QVBoxLayout;

class ChatPage : public BasePage {
public:
    explicit ChatPage(QWidget* parent = nullptr);
    void appendDemoConversation();
    void setLatestSession(const ConnectionState& state);

private:
    void clearMessages();

    QVBoxLayout* messages_ = nullptr;
    QPushButton* triggerButton_ = nullptr;
};
