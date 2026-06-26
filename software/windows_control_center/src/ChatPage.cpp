#include "ChatPage.h"

#include "ChatMessageWidget.h"
#include "DeepSeekChatClient.h"
#include "MarkdownLatexRenderer.h"
#include "QwenVisionQtClient.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPointer>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStyle>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QtConcurrent>

namespace {

class SmoothScrollArea final : public QScrollArea {
public:
    explicit SmoothScrollArea(QWidget* parent = nullptr)
        : QScrollArea(parent) {
        animation_ = new QPropertyAnimation(this);
        animation_->setTargetObject(verticalScrollBar());
        animation_->setPropertyName("value");
        animation_->setDuration(220);
        animation_->setEasingCurve(QEasingCurve::OutCubic);
    }

protected:
    void wheelEvent(QWheelEvent* event) override {
        if (verticalScrollBar() == nullptr) {
            QScrollArea::wheelEvent(event);
            return;
        }

        event->accept();
        const int delta = event->angleDelta().y();
        if (delta == 0) {
            return;
        }

        const int currentValue = verticalScrollBar()->value();
        const int step = qMax(36, verticalScrollBar()->singleStep() * 3);
        const int direction = delta > 0 ? -1 : 1;
        const int targetValue = qBound(verticalScrollBar()->minimum(),
                                       currentValue + direction * step,
                                       verticalScrollBar()->maximum());

        animation_->stop();
        animation_->setStartValue(currentValue);
        animation_->setEndValue(targetValue);
        animation_->start();
    }

private:
    QPropertyAnimation* animation_ = nullptr;
};

QString stageTitleText(const ConnectionState& state) {
    switch (state.assistantStatus) {
    case AssistantStatus::Ready:
        return QStringLiteral("Agent 对话已就绪");
    case AssistantStatus::Listening:
        return QStringLiteral("正在接收硬件侧语音指令");
    case AssistantStatus::Thinking:
        return QStringLiteral("AI 正在处理多模态输入");
    case AssistantStatus::Warning:
        return QStringLiteral("系统可运行，但有待检查项");
    case AssistantStatus::Error:
        return QStringLiteral("任务流程中断");
    case AssistantStatus::Connecting:
        return QStringLiteral("正在建立桥接链路");
    case AssistantStatus::Offline:
        return QStringLiteral("桥接链路离线");
    }
    return QStringLiteral("任务舞台等待中");
}

QString stageStatusText(const ConnectionState& state) {
    if (!state.assistantStatusText.isEmpty()) {
        return state.assistantStatusText;
    }
    return QStringLiteral("等待树莓派返回最新状态。");
}

QString stageMetaText(const ConnectionState& state) {
    QStringList parts;
    parts << QStringLiteral("文本主模型：DeepSeek V4 Flash");
    parts << QStringLiteral("视觉模型：Qwen VL");
    if (!state.activeHost.isEmpty()) {
        parts << QStringLiteral("主机：%1").arg(state.activeHost);
    }
    if (!state.localFramePath.isEmpty()) {
        parts << QStringLiteral("当前画面：已同步");
    } else {
        parts << QStringLiteral("当前画面：等待同步");
    }
    return parts.join(QStringLiteral("    ·    "));
}

QString summaryOrFallback(const QString& text, const QString& fallback) {
    const QString trimmed = text.trimmed();
    return trimmed.isEmpty() ? fallback : trimmed;
}

QPixmap loadHeroPixmap(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        return {};
    }
    QImageReader reader(imagePath);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull()) {
        return {};
    }
    return QPixmap::fromImage(image);
}

void animateWidgetRefresh(QWidget* widget, int duration = 240) {
    if (widget == nullptr) {
        return;
    }
    widget->setProperty("flash", true);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    QPointer<QWidget> guard(widget);
    QTimer::singleShot(duration, widget, [guard]() {
        if (guard == nullptr) {
            return;
        }
        guard->setProperty("flash", false);
        guard->style()->unpolish(guard);
        guard->style()->polish(guard);
        guard->update();
    });
}

void refreshLayoutAround(QWidget* widget) {
    QWidget* current = widget;
    int depth = 0;
    while (current != nullptr && depth < 6) {
        current->updateGeometry();
        if (current->layout() != nullptr) {
            current->layout()->invalidate();
            current->layout()->activate();
        }
        current = current->parentWidget();
        ++depth;
    }
}

void setAnimatedLabelText(QLabel* label, const QString& text, bool richText = false, int duration = 240) {
    if (label == nullptr) {
        return;
    }
    if (richText) {
        label->setTextFormat(Qt::RichText);
    } else {
        label->setTextFormat(Qt::PlainText);
    }
    if (label->text() == text) {
        return;
    }
    label->setText(text);
    label->updateGeometry();
    refreshLayoutAround(label);
    animateWidgetRefresh(label, duration);
}

QString compactSummary(const QList<AgentUiMessage>& messages) {
    for (int index = messages.size() - 1; index >= 0; --index) {
        if (messages[index].role == QStringLiteral("assistant")) {
            return summaryOrFallback(messages[index].rawText.left(220), QStringLiteral("等待 AI 回复。"));
        }
    }
    return QStringLiteral("你现在可以在下方输入自然语言需求，让 DeepSeek 决策并回复。");
}

QString latestUserOverview(const QList<AgentUiMessage>& messages) {
    for (int index = messages.size() - 1; index >= 0; --index) {
        if (messages[index].role == QStringLiteral("user")) {
            return summaryOrFallback(messages[index].rawText, QStringLiteral("等待输入。"));
        }
    }
    return QStringLiteral("支持自由输入需求；勾选“结合当前画面”后，会先由 Qwen 识别图像，再交给 DeepSeek 回答。");
}

} // namespace

ChatPage::ChatPage(AppConfig& config, QWidget* parent)
    : BasePage("实时对话", "现在这里是真正的 Agent 对话工作区：你输入需求，DeepSeek 负责推理与回复，Qwen 只做视觉观察。", parent)
    , config_(config) {
    turnWatcher_ = new QFutureWatcher<AgentTurnResult>(this);
    QObject::connect(turnWatcher_, &QFutureWatcher<AgentTurnResult>::finished, this, [this]() {
        const AgentTurnResult result = turnWatcher_->result();
        if (!result.success) {
            appendUiMessage({QStringLiteral("system"),
                             QStringLiteral("调用失败"),
                             result.errorText,
                             QString(),
                             QString()});
            setChatBusy(false, QStringLiteral("本轮调用失败，请检查 API key、网络或图片同步状态。"));
            return;
        }

        if (!result.visionSummary.trimmed().isEmpty()) {
            appendUiMessage({QStringLiteral("system"),
                             QStringLiteral("视觉观察（Qwen）"),
                             result.visionSummary,
                             result.visionSummary.toHtmlEscaped().replace("\n", "<br/>"),
                             result.userImagePath});
        }

        appendUiMessage({QStringLiteral("assistant"),
                         QStringLiteral("Agent 回复（DeepSeek）"),
                         result.assistantMarkdown,
                         result.assistantHtml,
                         QString()});
        setChatBusy(false, QStringLiteral("本轮回复已完成，可以继续追问或切换画面。"));
    });

    auto* workspaceRow = new QWidget(this);
    workspaceRow->setObjectName("chatWorkspaceRow");
    auto* workspaceLayout = new QHBoxLayout(workspaceRow);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(14);

    auto* leftWorkspace = new QWidget(this);
    leftWorkspace->setObjectName("chatWorkspaceLeft");
    leftWorkspace->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* leftLayout = new QVBoxLayout(leftWorkspace);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    auto* stagePanel = new QWidget(this);
    stagePanel->setObjectName("chatStagePanel");
    stagePanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto* stageLayout = new QVBoxLayout(stagePanel);
    stageLayout->setContentsMargins(18, 16, 18, 16);
    stageLayout->setSpacing(8);

    stageTitle_ = new QLabel(QStringLiteral("Agent 对话已就绪"), stagePanel);
    stageTitle_->setObjectName("chatStageTitle");
    stageStatus_ = new QLabel(QStringLiteral("输入自然语言需求，或等待硬件侧语音指令同步到这里。"), stagePanel);
    stageStatus_->setObjectName("chatStageStatus");
    stageStatus_->setWordWrap(true);
    stageStatus_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    stageMeta_ = new QLabel(QStringLiteral("文本主模型：DeepSeek V4 Flash    ·    视觉模型：Qwen VL"), stagePanel);
    stageMeta_->setObjectName("chatStageMeta");
    stageMeta_->setWordWrap(true);
    stageMeta_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    stageLayout->addWidget(stageTitle_);
    stageLayout->addWidget(stageStatus_);
    stageLayout->addWidget(stageMeta_);

    sectionCaption_ = new QLabel(QStringLiteral("Conversation"), this);
    sectionCaption_->setObjectName("chatSectionLabel");

    auto* conversationCard = new QWidget(this);
    conversationCard->setObjectName("chatConversationCard");
    conversationCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* conversationLayout = new QVBoxLayout(conversationCard);
    conversationLayout->setContentsMargins(18, 18, 18, 18);
    conversationLayout->setSpacing(12);

    chatScroll_ = new SmoothScrollArea(this);
    chatScroll_->setWidgetResizable(true);
    chatScroll_->setObjectName("chatScroll");
    chatScroll_->setFrameShape(QFrame::NoFrame);
    chatScroll_->setAttribute(Qt::WA_TranslucentBackground, true);
    chatScroll_->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    chatScroll_->viewport()->setAutoFillBackground(false);
    chatScroll_->verticalScrollBar()->setSingleStep(26);

    auto* inner = new QWidget(chatScroll_);
    inner->setAttribute(Qt::WA_TranslucentBackground, true);
    inner->setAutoFillBackground(false);
    messages_ = new QVBoxLayout(inner);
    messages_->setContentsMargins(8, 8, 8, 8);
    messages_->setSpacing(16);
    messages_->addStretch(1);
    chatScroll_->setWidget(inner);

    auto* composerCard = new QWidget(this);
    composerCard->setObjectName("chatComposerCard");
    auto* composerLayout = new QVBoxLayout(composerCard);
    composerLayout->setContentsMargins(14, 14, 14, 14);
    composerLayout->setSpacing(10);

    composerEdit_ = new QTextEdit(composerCard);
    composerEdit_->setObjectName("chatComposerEdit");
    composerEdit_->setPlaceholderText(QStringLiteral("输入你的需求，例如：\n- 帮我总结当前画面\n- 请结合画面解释这道题\n- 根据我刚才的实验结果给出下一步建议"));
    composerEdit_->setMinimumHeight(110);

    auto* composerActions = new QHBoxLayout();
    composerActions->setContentsMargins(0, 0, 0, 0);
    composerActions->setSpacing(10);

    includeSceneCheck_ = new QCheckBox(QStringLiteral("结合当前画面"), composerCard);
    includeSceneCheck_->setObjectName("chatSceneCheck");
    includeSceneCheck_->setChecked(true);

    clearButton_ = new QPushButton(QStringLiteral("清空会话"), composerCard);
    clearButton_->setObjectName("secondaryButton");
    QObject::connect(clearButton_, &QPushButton::clicked, this, [this]() {
        uiMessages_.clear();
        rebuildConversation();
        updateOverviewPanels(latestState_);
    });

    sendButton_ = new QPushButton(QStringLiteral("发送给 Agent"), composerCard);
    sendButton_->setObjectName("primaryButton");
    QObject::connect(sendButton_, &QPushButton::clicked, this, [this]() {
        sendPrompt();
    });

    composerActions->addWidget(includeSceneCheck_);
    composerActions->addStretch(1);
    composerActions->addWidget(clearButton_);
    composerActions->addWidget(sendButton_);

    composerLayout->addWidget(composerEdit_);
    composerLayout->addLayout(composerActions);

    conversationLayout->addWidget(sectionCaption_);
    conversationLayout->addWidget(chatScroll_, 1);
    conversationLayout->addWidget(composerCard, 0);

    auto* rightWorkspace = new QWidget(this);
    rightWorkspace->setObjectName("chatWorkspaceRight");
    rightWorkspace->setMinimumWidth(360);
    rightWorkspace->setMaximumWidth(430);
    rightWorkspace->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* rightLayout = new QVBoxLayout(rightWorkspace);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    auto* visualCard = new QWidget(this);
    visualCard->setObjectName("chatVisualCard");
    visualCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* visualLayout = new QVBoxLayout(visualCard);
    visualLayout->setContentsMargins(16, 16, 16, 16);
    visualLayout->setSpacing(10);

    auto* visualTitle = new QLabel(QStringLiteral("当前画面"), visualCard);
    visualTitle->setObjectName("chatPanelTitle");
    visualStatus_ = new QLabel(QStringLiteral("等待摄像头同步最新画面"), visualCard);
    visualStatus_->setObjectName("chatPanelSubtle");
    visualStatus_->setWordWrap(true);
    visualStatus_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    visualFrame_ = new QLabel(QStringLiteral("当树莓派抓取到最新照片后，这里会展示大图预览。"), visualCard);
    visualFrame_->setObjectName("chatImageHero");
    visualFrame_->setAlignment(Qt::AlignCenter);
    visualFrame_->setWordWrap(true);
    visualFrame_->setMinimumSize(360, 250);
    visualFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    visualFrame_->setMinimumHeight(250);

    visualLayout->addWidget(visualTitle);
    visualLayout->addWidget(visualStatus_);
    visualLayout->addWidget(visualFrame_, 1);

    auto* overviewCard = new QWidget(this);
    overviewCard->setObjectName("chatPanelCard");
    overviewCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto* overviewLayout = new QVBoxLayout(overviewCard);
    overviewLayout->setContentsMargins(16, 16, 16, 16);
    overviewLayout->setSpacing(10);
    auto* overviewTitle = new QLabel(QStringLiteral("当前输入摘要"), overviewCard);
    overviewTitle->setObjectName("chatPanelTitle");
    userSummary_ = new QLabel(QStringLiteral("你现在可以在左侧输入需求。"), overviewCard);
    userSummary_->setObjectName("chatPanelBody");
    userSummary_->setWordWrap(true);
    overviewLayout->addWidget(overviewTitle);
    overviewLayout->addWidget(userSummary_);

    auto* answerCard = new QWidget(this);
    answerCard->setObjectName("chatAnswerCard");
    answerCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    auto* answerLayout = new QVBoxLayout(answerCard);
    answerLayout->setContentsMargins(18, 18, 18, 18);
    answerLayout->setSpacing(8);
    auto* answerTitle = new QLabel(QStringLiteral("最新回复摘要"), answerCard);
    answerTitle->setObjectName("chatPanelTitle");
    answerSummary_ = new QLabel(QStringLiteral("等待第一条 Agent 回复。"), answerCard);
    answerSummary_->setObjectName("chatAnswerBody");
    answerSummary_->setWordWrap(true);
    answerLayout->addWidget(answerTitle);
    answerLayout->addWidget(answerSummary_, 1);

    leftLayout->addWidget(stagePanel, 0);
    leftLayout->addWidget(conversationCard, 1);

    rightLayout->addWidget(visualCard, 7);
    rightLayout->addWidget(overviewCard, 3);
    rightLayout->addWidget(answerCard, 4);

    workspaceLayout->addWidget(leftWorkspace, 9);
    workspaceLayout->addWidget(rightWorkspace, 5);

    bodyLayout()->addWidget(workspaceRow, 1);
    appendDemoConversation();
}

void ChatPage::appendDemoConversation() {
    lastSessionKey_.clear();
    uiMessages_.clear();
    uiMessages_.append({QStringLiteral("system"),
                        QStringLiteral("Agent 已待命"),
                        QStringLiteral("左侧输入自然语言需求，点击“发送给 Agent”后，将由 DeepSeek 负责最终回答；如果勾选“结合当前画面”，会先由 Qwen 对当前图片做客观识别，再把结果交给 DeepSeek 推理。"),
                        QString(),
                        QString()});
    rebuildConversation();
    updateStagePanel(latestState_);
    updateOverviewPanels(latestState_);
}

void ChatPage::setLatestSession(const ConnectionState& state) {
    latestState_ = state;
    updateStagePanel(state);
    updateOverviewPanels(state);
}

void ChatPage::clearMessages() {
    while (messages_->count() > 1) {
        QLayoutItem* item = messages_->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void ChatPage::updateStagePanel(const ConnectionState& state) {
    setAnimatedLabelText(stageTitle_, stageTitleText(state), false, 220);
    setAnimatedLabelText(stageStatus_, stageStatusText(state), false, 220);
    setAnimatedLabelText(stageMeta_, stageMetaText(state), false, 220);
}

void ChatPage::updateOverviewPanels(const ConnectionState& state) {
    setAnimatedLabelText(userSummary_, latestUserOverview(uiMessages_), false, 230);
    setAnimatedLabelText(answerSummary_, compactSummary(uiMessages_), false, 230);

    const QString imagePath = state.localFramePath;
    const QPixmap pixmap = loadHeroPixmap(imagePath);
    const QString oldKey = visualFrame_->property("contentKey").toString();
    if (!pixmap.isNull()) {
        visualFrame_->setPixmap(pixmap.scaled(620, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        visualFrame_->setProperty("contentKey", imagePath);
        setAnimatedLabelText(visualStatus_, QStringLiteral("最新画面已同步。勾选“结合当前画面”后，Qwen 会先读取这张图。"), false, 220);
    } else if (!imagePath.isEmpty()) {
        visualFrame_->setPixmap(QPixmap());
        visualFrame_->setText(QStringLiteral("图片已缓存，但当前无法显示。\n%1").arg(imagePath));
        visualFrame_->setProperty("contentKey", QStringLiteral("error:%1").arg(imagePath));
        setAnimatedLabelText(visualStatus_, QStringLiteral("本地存在图片路径，但 Qt 暂未成功解码。"), false, 220);
    } else {
        visualFrame_->setPixmap(QPixmap());
        visualFrame_->setText(QStringLiteral("等待树莓派同步首张摄像头画面。"));
        visualFrame_->setProperty("contentKey", QStringLiteral("empty"));
        setAnimatedLabelText(visualStatus_, QStringLiteral("当前没有可供视觉模型读取的现场图片。"), false, 220);
    }
    if (oldKey != visualFrame_->property("contentKey").toString()) {
        animateWidgetRefresh(visualFrame_, 260);
    }
}

void ChatPage::rebuildConversation() {
    clearMessages();
    int insertAt = qMax(0, messages_->count() - 1);
    for (const AgentUiMessage& message : uiMessages_) {
        auto role = ChatMessageWidget::Role::System;
        if (message.role == QStringLiteral("user")) {
            role = ChatMessageWidget::Role::User;
        } else if (message.role == QStringLiteral("assistant")) {
            role = ChatMessageWidget::Role::Assistant;
        }
        auto* widget = new ChatMessageWidget(role, this);
        if (!message.htmlText.isEmpty()) {
            widget->setRichMessage(message.title, message.htmlText, message.imagePath);
        } else {
            widget->setMessage(message.title, message.rawText, message.imagePath);
        }
        messages_->insertWidget(insertAt, widget);
        ++insertAt;
    }

    if (chatScroll_ != nullptr && chatScroll_->verticalScrollBar() != nullptr) {
        QTimer::singleShot(0, this, [this]() {
            if (chatScroll_ != nullptr && chatScroll_->verticalScrollBar() != nullptr) {
                chatScroll_->verticalScrollBar()->setValue(chatScroll_->verticalScrollBar()->maximum());
            }
        });
    }
}

void ChatPage::appendUiMessage(const AgentUiMessage& message) {
    uiMessages_.append(message);
    rebuildConversation();
    updateOverviewPanels(latestState_);
}

void ChatPage::sendPrompt() {
    const QString userPrompt = composerEdit_->toPlainText().trimmed();
    if (userPrompt.isEmpty()) {
        setChatBusy(false, QStringLiteral("请输入一个具体需求，再发送给 Agent。"));
        return;
    }
    if (turnWatcher_->isRunning()) {
        setChatBusy(true, QStringLiteral("上一轮还在处理中，请稍等。"));
        return;
    }

    const bool includeScene = includeSceneCheck_->isChecked();
    const QString imagePath = includeScene ? latestState_.localFramePath : QString();

    appendUiMessage({QStringLiteral("user"),
                     QStringLiteral("我的需求"),
                     userPrompt,
                     QString(),
                     imagePath});

    composerEdit_->clear();
    setChatBusy(true, includeScene
        ? QStringLiteral("正在先调用 Qwen 识别当前画面，再交给 DeepSeek 生成最终回答...")
        : QStringLiteral("正在调用 DeepSeek 生成最终回答..."));

    const ConnectionState stateSnapshot = latestState_;
    const QList<AgentUiMessage> historySnapshot = uiMessages_;
    turnWatcher_->setFuture(QtConcurrent::run([this, userPrompt, includeScene, stateSnapshot, historySnapshot]() {
        return runAgentTurn(userPrompt, includeScene, stateSnapshot, historySnapshot);
    }));
}

AgentTurnResult ChatPage::runAgentTurn(const QString& userPrompt,
                                       bool includeScene,
                                       const ConnectionState& stateSnapshot,
                                       const QList<AgentUiMessage>& historySnapshot) const {
    AgentTurnResult result;
    result.userText = userPrompt;
    result.userImagePath = includeScene ? stateSnapshot.localFramePath : QString();

    QString visualContext;
    if (includeScene) {
        if (result.userImagePath.trimmed().isEmpty()) {
            result.success = false;
            result.errorText = QStringLiteral("你勾选了“结合当前画面”，但当前还没有同步到最新图片。");
            return result;
        }

        QwenVisionQtClient qwenClient(config_);
        const VisionRecognitionResult vision = qwenClient.recognizeForPrompt(result.userImagePath, userPrompt);
        if (!vision.success) {
            result.success = false;
            result.errorText = vision.message;
            return result;
        }
        result.visionSummary = vision.summary;
        visualContext = vision.summary;
    }

    QList<ChatCompletionMessage> history;
    for (const AgentUiMessage& item : historySnapshot) {
        if (item.role == QStringLiteral("user")) {
            history.append({QStringLiteral("user"), item.rawText});
        } else if (item.role == QStringLiteral("assistant")) {
            history.append({QStringLiteral("assistant"), item.rawText});
        }
    }
    history.append({QStringLiteral("user"), userPrompt});

    DeepSeekChatClient deepSeekClient(config_);
    const ChatCompletionResult completion = deepSeekClient.complete(history, visualContext);
    if (!completion.success) {
        result.success = false;
        result.errorText = completion.message;
        return result;
    }

    MarkdownLatexRenderer renderer(config_);
    result.assistantMarkdown = completion.content;
    result.assistantHtml = renderer.renderToHtml(completion.content);
    result.success = true;
    return result;
}

void ChatPage::setChatBusy(bool busy, const QString& hint) {
    if (sendButton_ != nullptr) {
        sendButton_->setEnabled(!busy);
        sendButton_->setText(busy ? QStringLiteral("Agent 思考中...") : QStringLiteral("发送给 Agent"));
    }
    if (clearButton_ != nullptr) {
        clearButton_->setEnabled(!busy);
    }
    if (composerEdit_ != nullptr) {
        composerEdit_->setEnabled(!busy);
    }
    if (!hint.trimmed().isEmpty()) {
        setAnimatedLabelText(stageStatus_, hint, false, 220);
    } else {
        setAnimatedLabelText(stageStatus_, stageStatusText(latestState_), false, 220);
    }
}
