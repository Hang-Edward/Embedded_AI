#include "ChatPage.h"

#include "ChatMessageWidget.h"
#include "DeepSeekChatClient.h"
#include "GlassSurface.h"
#include "MarkdownLatexRenderer.h"
#include "QwenVisionQtClient.h"
#include "SmoothScrollArea.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QPixmap>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStyle>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtConcurrent>

namespace {

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
    parts << QStringLiteral("文本：DeepSeek V4 Flash");
    parts << QStringLiteral("视觉：Qwen VL");
    if (!state.activeHost.isEmpty()) {
        parts << QStringLiteral("主机 %1").arg(state.activeHost);
    }
    if (!state.localFramePath.isEmpty()) {
        parts << QStringLiteral("画面已同步");
    } else {
        parts << QStringLiteral("等待画面");
    }
    return parts.join(QStringLiteral("    ·    "));
}

QString summaryOrFallback(const QString& text, const QString& fallback) {
    const QString trimmed = text.trimmed();
    return trimmed.isEmpty() ? fallback : trimmed;
}

QString normalizedPreviewText(QString text, int maxLength) {
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QRegularExpression(QStringLiteral("```[\\s\\S]*?```")), QStringLiteral(" 代码片段 "));
    text.replace(QRegularExpression(QStringLiteral("`([^`]+)`")), QStringLiteral("\\1"));
    text.replace(QRegularExpression(QStringLiteral("^[#>]+\\s*"), QRegularExpression::MultilineOption), QString());
    text.replace(QRegularExpression(QStringLiteral("^\\s*[-*+]\\s+"), QRegularExpression::MultilineOption), QString());
    text.replace(QRegularExpression(QStringLiteral("^\\s*\\d+[.)]\\s+"), QRegularExpression::MultilineOption), QString());
    text.replace(QStringLiteral("$$"), QString());
    text.replace(QStringLiteral("\\["), QString());
    text.replace(QStringLiteral("\\]"), QString());
    text.replace(QStringLiteral("\\("), QString());
    text.replace(QStringLiteral("\\)"), QString());
    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    text = text.trimmed();
    if (text.size() > maxLength) {
        text = text.left(maxLength).trimmed() + QStringLiteral("...");
    }
    return text;
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
            return summaryOrFallback(
                normalizedPreviewText(messages[index].rawText, 132),
                QStringLiteral("等待 AI 回复。"));
        }
    }
    return QStringLiteral("你现在可以继续追问，让 DeepSeek 保持同一轮上下文。");
}

QString latestUserOverview(const QList<AgentUiMessage>& messages) {
    for (int index = messages.size() - 1; index >= 0; --index) {
        if (messages[index].role == QStringLiteral("user")) {
            return summaryOrFallback(
                normalizedPreviewText(messages[index].rawText, 88),
                QStringLiteral("等待输入。"));
        }
    }
    return QStringLiteral("支持自由输入；勾选“结合当前画面”后，会先由 Qwen 识别图像，再交给 DeepSeek 回答。");
}

} // namespace

ChatPage::ChatPage(AppConfig& config, QWidget* parent)
    : BasePage("实时对话", "这里是主工作台：输入需求，DeepSeek 负责推理与回复，Qwen 只负责视觉观察。", parent)
    , config_(config)
    , renderer_(config) {
    titleLabel()->setProperty("chatPage", true);
    titleLabel()->style()->unpolish(titleLabel());
    titleLabel()->style()->polish(titleLabel());
    if (QLabel* subtitleLabel = findChild<QLabel*>(QStringLiteral("pageSubtitle"), Qt::FindDirectChildrenOnly)) {
        subtitleLabel->hide();
    }

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
    leftLayout->setSpacing(10);

    auto* stagePanel = new QWidget(this);
    stagePanel->setObjectName("chatStagePanel");
    stagePanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto* stageLayout = new QVBoxLayout(stagePanel);
    stageLayout->setContentsMargins(18, 12, 18, 12);
    stageLayout->setSpacing(5);

    stageTitle_ = new QLabel(QStringLiteral("Agent 对话已就绪"), stagePanel);
    stageTitle_->setObjectName("chatStageTitle");
    stageStatus_ = new QLabel(QStringLiteral("输入自然语言需求，或等待硬件侧语音指令同步到这里。"), stagePanel);
    stageStatus_->setObjectName("chatStageStatus");
    stageStatus_->setWordWrap(true);
    stageStatus_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    stageMeta_ = new QLabel(QStringLiteral("文本：DeepSeek V4 Flash    ·    视觉：Qwen VL"), stagePanel);
    stageMeta_->setObjectName("chatStageMeta");
    stageMeta_->setWordWrap(true);
    stageMeta_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    stageLayout->addWidget(stageTitle_);
    stageLayout->addWidget(stageStatus_);
    stageLayout->addWidget(stageMeta_);

    sectionCaption_ = new QLabel(QStringLiteral("CONVERSATION STAGE"), this);
    sectionCaption_->setObjectName("chatSectionLabel");

    auto* conversationCard = new QWidget(this);
    conversationCard->setObjectName("chatConversationCard");
    conversationCard->setAttribute(Qt::WA_TranslucentBackground, true);
    conversationCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* conversationLayout = new QVBoxLayout(conversationCard);
    conversationLayout->setContentsMargins(20, 18, 20, 16);
    conversationLayout->setSpacing(14);

    conversationContainer_ = new QWidget(conversationCard);
    conversationContainer_->setObjectName("chatConversationViewport");
    conversationContainer_->setAttribute(Qt::WA_TranslucentBackground, true);
    conversationContainer_->setAutoFillBackground(false);
    conversationContainer_->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    conversationContainer_->setFocusPolicy(Qt::StrongFocus);
    conversationContainer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    conversationContainer_->setMinimumHeight(650);
    auto* conversationPlaceholderLayout = new QVBoxLayout(conversationContainer_);
    conversationPlaceholderLayout->setContentsMargins(0, 0, 0, 0);
    conversationPlaceholderLayout->setSpacing(0);

    conversationScroll_ = new SmoothScrollArea(conversationContainer_);
    conversationScroll_->setObjectName("chatScroll");
    conversationScroll_->setFrameShape(QFrame::NoFrame);
    conversationScroll_->setWidgetResizable(true);
    conversationScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    conversationScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    conversationScroll_->setAttribute(Qt::WA_TranslucentBackground, true);
    conversationScroll_->setAutoFillBackground(false);
    conversationScroll_->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    conversationScroll_->viewport()->setAutoFillBackground(false);
    conversationScroll_->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    conversationScroll_->viewport()->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    conversationScroll_->setMinimumHeight(650);

    conversationHost_ = new QWidget(conversationScroll_);
    conversationHost_->setObjectName("chatConversationHost");
    conversationHost_->setAttribute(Qt::WA_TranslucentBackground, true);
    conversationHost_->setAutoFillBackground(false);
    conversationHost_->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    conversationHost_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    conversationMessagesLayout_ = new QVBoxLayout(conversationHost_);
    conversationMessagesLayout_->setContentsMargins(0, 0, 0, 0);
    conversationMessagesLayout_->setSpacing(18);
    conversationMessagesLayout_->addStretch(1);

    conversationScroll_->setWidget(conversationHost_);
    conversationPlaceholderLayout->addWidget(conversationScroll_, 1);

    auto* composerCard = new QWidget(this);
    composerCard->setObjectName("chatComposerCard");
    auto* composerLayout = new QVBoxLayout(composerCard);
    composerLayout->setContentsMargins(14, 14, 14, 14);
    composerLayout->setSpacing(8);

    composerEdit_ = new QTextEdit(composerCard);
    composerEdit_->setObjectName("chatComposerEdit");
    composerEdit_->setPlaceholderText(QStringLiteral("输入你的需求，例如：\n- 帮我总结当前画面\n- 请结合画面解释这道题\n- 根据我刚才的实验结果给出下一步建议"));
    composerEdit_->setMinimumHeight(104);

    auto* composerActions = new QHBoxLayout();
    composerActions->setContentsMargins(0, 0, 0, 0);
    composerActions->setSpacing(10);

    includeSceneCheck_ = new GlassCheckBox(QStringLiteral("结合当前画面"), composerCard);
    includeSceneCheck_->setObjectName("chatSceneCheck");
    includeSceneCheck_->setChecked(config_.chatIncludeCurrentScene);
    QObject::connect(includeSceneCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        config_.chatIncludeCurrentScene = checked;
        config_.save();
    });

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
    conversationLayout->addWidget(conversationContainer_, 1);
    conversationLayout->addWidget(composerCard, 0);

    leftLayout->addWidget(stagePanel, 0);
    leftLayout->addWidget(conversationCard, 1);
    workspaceLayout->addWidget(leftWorkspace, 1);

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
    if (conversationMessagesLayout_ == nullptr) {
        return;
    }

    while (QLayoutItem* item = conversationMessagesLayout_->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            delete widget;
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
    if (userSummary_ != nullptr) {
        setAnimatedLabelText(userSummary_, latestUserOverview(uiMessages_), false, 230);
    }
    if (answerSummary_ != nullptr) {
        setAnimatedLabelText(answerSummary_, compactSummary(uiMessages_), false, 230);
    }

    if (visualFrame_ == nullptr || visualStatus_ == nullptr) {
        return;
    }
    const QString imagePath = state.localFramePath;
    const QPixmap pixmap = loadHeroPixmap(imagePath);
    const QString oldKey = visualFrame_->property("contentKey").toString();
    if (!pixmap.isNull()) {
        visualFrame_->setPixmap(pixmap.scaled(560, 340, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
    if (conversationMessagesLayout_ == nullptr) {
        return;
    }

    clearMessages();

    for (const AgentUiMessage& message : uiMessages_) {
        ChatMessageWidget::Role role = ChatMessageWidget::Role::System;
        if (message.role == QStringLiteral("user")) {
            role = ChatMessageWidget::Role::User;
        } else if (message.role == QStringLiteral("assistant")) {
            role = ChatMessageWidget::Role::Assistant;
        }

        auto* widget = new ChatMessageWidget(role, conversationHost_);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        widget->setRichMessage(message.title,
                               message.htmlText.isEmpty()
                                   ? message.rawText.toHtmlEscaped().replace("\n", "<br/>")
                                   : message.htmlText,
                               message.imagePath);
        conversationMessagesLayout_->addWidget(widget, 0, Qt::AlignTop);
    }

    conversationMessagesLayout_->addStretch(1);

    if (conversationScroll_ != nullptr && conversationScroll_->verticalScrollBar() != nullptr) {
        QPointer<SmoothScrollArea> guard(conversationScroll_);
        QTimer::singleShot(0, this, [guard]() {
            if (guard == nullptr || guard->verticalScrollBar() == nullptr) {
                return;
            }
            guard->verticalScrollBar()->setValue(guard->verticalScrollBar()->maximum());
        });
    }
}

void ChatPage::appendUiMessage(const AgentUiMessage& message) {
    AgentUiMessage normalized = message;
    if (normalized.role == QStringLiteral("assistant")) {
        normalized.htmlText = normalized.htmlText.isEmpty()
            ? renderer_.renderToHtml(normalized.rawText)
            : normalized.htmlText;
    } else if (normalized.htmlText.isEmpty()) {
        normalized.htmlText = normalized.rawText.toHtmlEscaped().replace("\n", "<br/>");
    }

    uiMessages_.append(normalized);
    rebuildConversation();
    updateOverviewPanels(latestState_);
}

void ChatPage::sendPrompt() {
    const QString userPrompt = composerEdit_->toPlainText().trimmed();
    if (userPrompt.isEmpty()) {
        appendUiMessage({QStringLiteral("system"),
                         QStringLiteral("输入为空"),
                         QStringLiteral("请输入一个具体需求，再发送给 Agent。"),
                         QString(),
                         QString()});
        setChatBusy(false, QStringLiteral("请输入一个具体需求，再发送给 Agent。"));
        return;
    }
    if (turnWatcher_->isRunning()) {
        appendUiMessage({QStringLiteral("system"),
                         QStringLiteral("任务仍在进行"),
                         QStringLiteral("上一轮还在处理中，请等待当前回复完成后再发送新需求。"),
                         QString(),
                         QString()});
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

    if (includeScene) {
        appendUiMessage({QStringLiteral("system"),
                         QStringLiteral("任务已接收"),
                         QStringLiteral("已进入多模态流程：先读取当前画面，再交给 DeepSeek 做最终推理。"),
                         QString(),
                         QString()});
    } else {
        appendUiMessage({QStringLiteral("system"),
                         QStringLiteral("任务已接收"),
                         QStringLiteral("已进入纯文本流程：直接调用 DeepSeek 生成最终回答。"),
                         QString(),
                         QString()});
    }

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
            result.errorText = QStringLiteral("你勾选了“结合当前画面”，但当前还没有同步到最新图片，无法启动视觉识别流程。");
            return result;
        }

        QwenVisionQtClient qwenClient(config_);
        const VisionRecognitionResult vision = qwenClient.recognizeForPrompt(result.userImagePath, userPrompt);
        if (!vision.success) {
            result.success = false;
            result.errorText = QStringLiteral("Qwen 视觉识别失败：%1").arg(vision.message);
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
        result.errorText = QStringLiteral("DeepSeek 回复失败：%1").arg(completion.message);
        return result;
    }

    result.assistantMarkdown = completion.content;
    result.assistantHtml = QString();
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
