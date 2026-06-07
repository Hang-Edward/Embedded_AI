#pragma once

#include "BasePage.h"

class QPushButton;
class QVBoxLayout;

class ChatPage : public BasePage {
public:
    explicit ChatPage(QWidget* parent = nullptr);
    void appendDemoConversation();
    void setLatestSession(const QString& logText, const QString& imagePath);

private:
    void clearMessages();

    QVBoxLayout* messages_ = nullptr;
    QPushButton* triggerButton_ = nullptr;
};
