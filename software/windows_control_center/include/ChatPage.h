#pragma once

#include "AppConfig.h"
#include "BasePage.h"
#include "ConnectionState.h"
#include "MarkdownLatexRenderer.h"

#include <QFutureWatcher>
#include <QList>

class QTextEdit;
class QLabel;
class QWidget;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class GlassCheckBox;

struct AgentUiMessage {
    QString role;
    QString title;
    QString rawText;
    QString htmlText;
    QString imagePath;
};

struct AgentTurnResult {
    bool success = false;
    QString userText;
    QString userImagePath;
    QString visionSummary;
    QString assistantMarkdown;
    QString assistantHtml;
    QString errorText;
};

class ChatPage : public BasePage {
public:
    explicit ChatPage(AppConfig& config, QWidget* parent = nullptr);
    void appendDemoConversation();
    void setLatestSession(const ConnectionState& state);

private:
    void clearMessages();
    void updateStagePanel(const ConnectionState& state);
    void updateOverviewPanels(const ConnectionState& state);
    void rebuildConversation();
    void appendUiMessage(const AgentUiMessage& message);
    void sendPrompt();
    AgentTurnResult runAgentTurn(const QString& userPrompt,
                                 bool includeScene,
                                 const ConnectionState& stateSnapshot,
                                 const QList<AgentUiMessage>& historySnapshot) const;
    void setChatBusy(bool busy, const QString& hint);

    AppConfig& config_;
    MarkdownLatexRenderer renderer_;
    ConnectionState latestState_;
    QList<AgentUiMessage> uiMessages_;
    QFutureWatcher<AgentTurnResult>* turnWatcher_ = nullptr;
    QLabel* sectionCaption_ = nullptr;
    QLabel* stageTitle_ = nullptr;
    QLabel* stageStatus_ = nullptr;
    QLabel* stageMeta_ = nullptr;
    QLabel* userSummary_ = nullptr;
    QLabel* visualStatus_ = nullptr;
    QLabel* visualFrame_ = nullptr;
    QLabel* answerSummary_ = nullptr;
    QWidget* conversationContainer_ = nullptr;
    QScrollArea* conversationScroll_ = nullptr;
    QWidget* conversationHost_ = nullptr;
    QVBoxLayout* conversationMessagesLayout_ = nullptr;
    QTextEdit* composerEdit_ = nullptr;
    QPushButton* sendButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;
    GlassCheckBox* includeSceneCheck_ = nullptr;
    QString lastSessionKey_;
};
