#pragma once

#include "BasePage.h"
#include "ConnectionState.h"

class QVBoxLayout;
class QLabel;
class QWidget;
class QScrollArea;

class ChatPage : public BasePage {
public:
    explicit ChatPage(QWidget* parent = nullptr);
    void appendDemoConversation();
    void setLatestSession(const ConnectionState& state);

private:
    void clearMessages();
    void updateStagePanel(const ConnectionState& state);
    void updateOverviewPanels(const ConnectionState& state);

    QVBoxLayout* messages_ = nullptr;
    QLabel* sectionCaption_ = nullptr;
    QLabel* stageTitle_ = nullptr;
    QLabel* stageStatus_ = nullptr;
    QLabel* stageMeta_ = nullptr;
    QLabel* userSummary_ = nullptr;
    QLabel* visualStatus_ = nullptr;
    QLabel* visualFrame_ = nullptr;
    QLabel* answerSummary_ = nullptr;
    QScrollArea* chatScroll_ = nullptr;
    QString lastSessionKey_;
};
