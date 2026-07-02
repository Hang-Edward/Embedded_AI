#pragma once

#include "AppConfig.h"
#include "BasePage.h"
#include "ChatSessionModels.h"
#include "ConnectionState.h"
#include "MarkdownLatexRenderer.h"

#include <QFutureWatcher>
#include <QList>
#include <QEvent>
#include <functional>

class QTextEdit;
class QLabel;
class QWidget;
class QPushButton;
class QTimer;
class QVBoxLayout;
class GlassCheckBox;
class SmoothScrollArea;

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
    void startFreshConversation();
    QList<ArchivedChatSession> archivedSessions() const;
    bool restoreArchivedSession(const QString& sessionId);
    void setHistoryChangedCallback(std::function<void()> callback);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void clearMessages();
    void updateStagePanel(const ConnectionState& state);
    void updateOverviewPanels(const ConnectionState& state);
    void rebuildConversation();
    void appendUiMessage(const AgentUiMessage& message);
    void saveConversationArchive() const;
    void loadConversationArchive();
    void persistCurrentSessionToArchive();
    ArchivedChatSession buildCurrentSessionSnapshot() const;
    bool hasMeaningfulConversation() const;
    void sendPrompt();
    AgentTurnResult runAgentTurn(const QString& userPrompt,
                                 bool includeScene,
                                 const ConnectionState& stateSnapshot,
                                 const QList<AgentUiMessage>& historySnapshot) const;
    void setChatBusy(bool busy, const QString& hint);
    void updateThinkingIndicator();

    AppConfig& config_;
    MarkdownLatexRenderer renderer_;
    ConnectionState latestState_;
    QList<AgentUiMessage> uiMessages_;
    QList<ArchivedChatSession> archivedSessions_;
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
    SmoothScrollArea* conversationScroll_ = nullptr;
    QWidget* conversationHost_ = nullptr;
    QVBoxLayout* conversationMessagesLayout_ = nullptr;
    QTextEdit* composerEdit_ = nullptr;
    QPushButton* sendButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;
    GlassCheckBox* includeSceneCheck_ = nullptr;
    QTimer* thinkingTimer_ = nullptr;
    int thinkingFrame_ = 0;
    QString lastSessionKey_;
    QString currentSessionId_;
    std::function<void()> historyChangedCallback_;
};
