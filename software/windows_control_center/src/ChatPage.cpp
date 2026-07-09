#include "ChatPage.h"

#include "ChatMessageWidget.h"
#include "ChatInputPolicy.h"
#include "ChatSessionStore.h"
#include "AgentWorkflowPolicy.h"
#include "DeepSeekChatClient.h"
#include "GlassSurface.h"
#include "MarkdownLatexRenderer.h"
#include "QwenVisionQtClient.h"
#include "SmoothScrollArea.h"

#include <QFrame>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QLinearGradient>
#include <QPalette>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QPixmap>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QStyle>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QInputMethod>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <QWidget>
#include <QtConcurrent>

namespace {

BackgroundWidget* locateBackgroundWidget(const QWidget* origin) {
    QWidget* current = origin != nullptr ? origin->window() : nullptr;
    while (current != nullptr) {
        if (QWidget* candidate = current->findChild<QWidget*>(QStringLiteral("central"))) {
            if (auto* background = dynamic_cast<BackgroundWidget*>(candidate)) {
                return background;
            }
        }
        if (auto* background = dynamic_cast<BackgroundWidget*>(current)) {
            return background;
        }
        current = current->parentWidget();
    }
    return nullptr;
}

class ConversationHostWidget final : public QWidget {
public:
    explicit ConversationHostWidget(QWidget* parent = nullptr)
        : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        if (auto* background = locateBackgroundWidget(this)) {
            background->renderSceneInto(painter, rect(), mapTo(background, QPoint(0, 0)));
        } else {
            painter.fillRect(rect(), QColor(18, 40, 82, 36));
        }

        const QRectF overlay = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        QLinearGradient wash(overlay.topLeft(), overlay.bottomRight());
        wash.setColorAt(0.0, QColor(22, 48, 92, 52));
        wash.setColorAt(0.52, QColor(18, 42, 82, 34));
        wash.setColorAt(1.0, QColor(14, 34, 72, 58));
        painter.fillRect(rect(), wash);
    }
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

QString conversationSnapshotPath() {
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.trimmed().isEmpty()) {
        baseDir = QDir::currentPath();
    }
    QDir().mkpath(baseDir);
    return QDir(baseDir).filePath(QStringLiteral("chat-session.json"));
}

QString conversationArchivePath() {
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.trimmed().isEmpty()) {
        baseDir = QDir::currentPath();
    }
    QDir().mkpath(baseDir);
    return QDir(baseDir).filePath(QStringLiteral("chat-history.json"));
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
    thinkingTimer_ = new QTimer(this);
    thinkingTimer_->setInterval(250);
    QObject::connect(thinkingTimer_, &QTimer::timeout, this, [this]() {
        updateThinkingIndicator();
    });
    QObject::connect(turnWatcher_, &QFutureWatcher<AgentTurnResult>::finished, this, [this]() {
        const AgentTurnResult result = turnWatcher_->result();
        const QString timing = QStringLiteral("总用时 %1 秒（视觉 %2 秒，DeepSeek %3 秒，排版 %4 秒）")
                                   .arg(result.totalMs / 1000.0, 0, 'f', 1)
                                   .arg(result.visionMs / 1000.0, 0, 'f', 1)
                                   .arg(result.deepSeekMs / 1000.0, 0, 'f', 1)
                                   .arg(result.renderMs / 1000.0, 0, 'f', 1);
        if (!result.success) {
            setChatBusy(false, QStringLiteral("本轮调用失败，%1。请检查 API key、网络或图片同步状态。").arg(timing));
            appendUiMessage({QStringLiteral("system"),
                             QStringLiteral("调用失败"),
                             result.errorText,
                             QString(),
                             QString()});
            return;
        }

        setChatBusy(false, QStringLiteral("本轮回复已完成，%1。").arg(timing));
        if (!result.warningText.trimmed().isEmpty()) {
            appendUiMessage({QStringLiteral("system"),
                             QStringLiteral("视觉回退"),
                             result.warningText,
                             QString(),
                             QString()});
        }

        appendUiMessage({QStringLiteral("assistant"),
                         QStringLiteral("Agent 回复（DeepSeek）"),
                         result.assistantMarkdown,
                         result.assistantHtml,
                         QString()});
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

    conversationHost_ = new ConversationHostWidget(conversationScroll_);
    conversationHost_->setObjectName("chatConversationHost");
    conversationHost_->setAttribute(Qt::WA_TranslucentBackground, true);
    conversationHost_->setAutoFillBackground(false);
    conversationHost_->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    conversationHost_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

    composerEdit_ = new QPlainTextEdit(composerCard);
    composerEdit_->setObjectName("chatComposerEdit");
    composerEdit_->setPlaceholderText(QStringLiteral("输入你的需求，例如：\n- 帮我总结当前画面\n- 请结合画面解释这道题\n- 根据我刚才的实验结果给出下一步建议"));
    composerEdit_->setMinimumHeight(104);
    composerEdit_->setAcceptDrops(false);
    composerEdit_->setFrameShape(QFrame::NoFrame);
    composerEdit_->installEventFilter(this);
    composerEdit_->viewport()->installEventFilter(this);
    composerEdit_->setStyleSheet(QStringLiteral(
        "QPlainTextEdit {"
        "background: rgba(7, 16, 36, 0.54);"
        "border: 1px solid rgba(170, 220, 255, 0.10);"
        "border-radius: 14px;"
        "padding: 10px 12px;"
        "selection-background-color: rgba(108, 164, 255, 0.34);"
        "}"
        "QPlainTextEdit:focus {"
        "border: 1px solid rgba(170, 220, 255, 0.20);"
        "background: rgba(7, 18, 38, 0.62);"
        "}"));
    composerEdit_->viewport()->setAutoFillBackground(true);
    composerEdit_->viewport()->setAttribute(Qt::WA_TranslucentBackground, false);
    composerEdit_->viewport()->setAttribute(Qt::WA_NoSystemBackground, false);
    QPalette composerPalette = composerEdit_->palette();
    composerPalette.setColor(QPalette::Text, QColor(QStringLiteral("#eef6ff")));
    composerPalette.setColor(QPalette::Base, QColor(10, 20, 44, 180));
    composerPalette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#8fa8c9")));
    composerEdit_->setPalette(composerPalette);

    auto* composerActions = new QHBoxLayout();
    composerActions->setContentsMargins(0, 0, 0, 0);
    composerActions->setSpacing(10);

    includeSceneCheck_ = new GlassCheckBox(QStringLiteral("结合当前画面"), composerCard);
    includeSceneCheck_->setObjectName("chatSceneCheck");
    includeSceneCheck_->setMinimumWidth(includeSceneCheck_->sizeHint().width() + 10);
    includeSceneCheck_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    includeSceneCheck_->setChecked(config_.chatIncludeCurrentScene);
    QObject::connect(includeSceneCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        config_.chatIncludeCurrentScene = checked;
        config_.save();
    });

    clearButton_ = new QPushButton(QStringLiteral("清空会话"), composerCard);
    clearButton_->setObjectName("secondaryButton");
    QObject::connect(clearButton_, &QPushButton::clicked, this, [this]() {
        startFreshConversation();
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
    loadConversationArchive();
    startFreshConversation();
}

bool ChatPage::eventFilter(QObject* watched, QEvent* event) {
    if ((watched == composerEdit_ || watched == composerEdit_->viewport())
        && event != nullptr
        && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (ChatInputPolicy::shouldSubmit(keyEvent->key(),
                                          keyEvent->modifiers(),
                                          keyEvent->isAutoRepeat())) {
            sendPrompt();
            return true;
        }
    }

    return BasePage::eventFilter(watched, event);
}

void ChatPage::appendDemoConversation() {
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

void ChatPage::startFreshConversation() {
    currentSessionId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
    appendDemoConversation();
}

QList<ArchivedChatSession> ChatPage::archivedSessions() const {
    return archivedSessions_;
}

bool ChatPage::restoreArchivedSession(const QString& sessionId) {
    for (const ArchivedChatSession& session : archivedSessions_) {
        if (session.sessionId != sessionId) {
            continue;
        }
        currentSessionId_ = session.sessionId;
        uiMessages_ = session.messages;
        rebuildConversation();
        updateStagePanel(latestState_);
        updateOverviewPanels(latestState_);
        return true;
    }
    return false;
}

void ChatPage::setHistoryChangedCallback(std::function<void()> callback) {
    historyChangedCallback_ = std::move(callback);
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
        QString htmlBody = message.htmlText;
        if (htmlBody.isEmpty()) {
            if (message.role == QStringLiteral("assistant")) {
                // 中文注释：恢复本地会话时，助手消息也要重新走 Markdown 渲染，
                // 否则标题、列表、代码块会退化成原始的 ## / ``` 纯文本。
                htmlBody = renderer_.renderToHtml(message.rawText);
            } else {
                htmlBody = message.rawText.toHtmlEscaped().replace("\n", "<br/>");
            }
        }

        widget->setRichMessage(message.title, htmlBody, message.imagePath);
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
    persistCurrentSessionToArchive();
    updateOverviewPanels(latestState_);
}

void ChatPage::saveConversationArchive() const {
    QString error;
    ChatSessionStore::save(conversationArchivePath(), archivedSessions_, &error);
}

void ChatPage::loadConversationArchive() {
    QString error;
    archivedSessions_ = ChatSessionStore::load(conversationArchivePath(), &error);
}

bool ChatPage::hasMeaningfulConversation() const {
    for (const AgentUiMessage& message : uiMessages_) {
        if (message.role == QStringLiteral("user") || message.role == QStringLiteral("assistant")) {
            return true;
        }
    }
    return false;
}

ArchivedChatSession ChatPage::buildCurrentSessionSnapshot() const {
    ArchivedChatSession session;
    session.sessionId = currentSessionId_;
    session.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    session.messages = uiMessages_;

    QString firstUserText;
    QString lastAssistantText;
    for (const AgentUiMessage& message : uiMessages_) {
        if (firstUserText.isEmpty() && message.role == QStringLiteral("user")) {
            firstUserText = normalizedPreviewText(message.rawText, 42);
        }
        if (message.role == QStringLiteral("assistant")) {
            lastAssistantText = normalizedPreviewText(message.rawText, 88);
        }
    }

    session.title = firstUserText.isEmpty() ? QStringLiteral("未命名对话") : firstUserText;
    session.summary = !lastAssistantText.isEmpty()
        ? lastAssistantText
        : QStringLiteral("暂无 AI 回复摘要。");
    return session;
}

void ChatPage::persistCurrentSessionToArchive() {
    if (!hasMeaningfulConversation()) {
        return;
    }

    const ArchivedChatSession snapshot = buildCurrentSessionSnapshot();
    for (int index = 0; index < archivedSessions_.size(); ++index) {
        if (archivedSessions_[index].sessionId == snapshot.sessionId) {
            archivedSessions_.removeAt(index);
            break;
        }
    }
    archivedSessions_.prepend(snapshot);
    saveConversationArchive();

    if (historyChangedCallback_) {
        historyChangedCallback_();
    }
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

    composerEdit_->clear();
    setChatBusy(true, QStringLiteral("深度思考中"));

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
    QElapsedTimer totalTimer;
    totalTimer.start();
    result.userText = userPrompt;
    result.userImagePath = includeScene ? stateSnapshot.localFramePath : QString();

    QList<ChatCompletionMessage> history;
    for (const AgentUiMessage& item : historySnapshot) {
        if (item.role == QStringLiteral("user")) {
            history.append({QStringLiteral("user"), item.rawText});
        } else if (item.role == QStringLiteral("assistant")) {
            history.append({QStringLiteral("assistant"), item.rawText});
        }
    }
    history.append({QStringLiteral("user"), userPrompt});

    const bool imageAvailable = includeScene
        && !result.userImagePath.trimmed().isEmpty()
        && QFileInfo::exists(result.userImagePath);
    const AgentWorkflowExecution execution = AgentWorkflowPolicy::execute(
        includeScene,
        imageAvailable,
        [this, &result, imagePath = result.userImagePath, userPrompt]() {
            QElapsedTimer timer;
            timer.start();
            QwenVisionQtClient client(config_);
            const VisionRecognitionResult vision = client.recognizeForPrompt(imagePath, userPrompt);
            result.visionMs = timer.elapsed();
            return AgentStageResult {vision.success, vision.summary, vision.message};
        },
        [this, &result, history](const QString& visualContext) {
            QElapsedTimer timer;
            timer.start();
            DeepSeekChatClient client(config_);
            const ChatCompletionResult completion = client.complete(history, visualContext);
            result.deepSeekMs = timer.elapsed();
            return AgentStageResult {completion.success, completion.content, completion.message};
        });
    if (!execution.success) {
        result.success = false;
        result.errorText = QStringLiteral("DeepSeek 回复失败：%1").arg(execution.errorText);
        result.totalMs = totalTimer.elapsed();
        return result;
    }

    result.visionSummary = execution.visionSummary;
    result.warningText = execution.warningText;
    result.assistantMarkdown = execution.answer;
    QElapsedTimer renderTimer;
    renderTimer.start();
    MarkdownLatexRenderer backgroundRenderer(config_);
    result.assistantHtml = backgroundRenderer.renderToHtml(execution.answer);
    result.renderMs = renderTimer.elapsed();
    result.totalMs = totalTimer.elapsed();
    result.success = true;
    return result;
}

void ChatPage::setChatBusy(bool busy, const QString& hint) {
    if (sendButton_ != nullptr) {
        sendButton_->setEnabled(!busy);
        if (!busy) {
            sendButton_->setText(QStringLiteral("发送给 Agent"));
        }
    }
    if (clearButton_ != nullptr) {
        clearButton_->setEnabled(!busy);
    }
    if (composerEdit_ != nullptr) {
        composerEdit_->setEnabled(!busy);
    }
    if (thinkingTimer_ != nullptr) {
        if (busy) {
            thinkingFrame_ = 0;
            thinkingElapsed_.restart();
            showThinkingMessage();
            thinkingTimer_->start();
            updateThinkingIndicator();
        } else {
            thinkingTimer_->stop();
            thinkingFrame_ = 0;
            removeThinkingMessage();
        }
    }
    if (!hint.trimmed().isEmpty()) {
        setAnimatedLabelText(stageStatus_, hint, false, 220);
    } else {
        setAnimatedLabelText(stageStatus_, stageStatusText(latestState_), false, 220);
    }
}

void ChatPage::updateThinkingIndicator() {
    if (sendButton_ == nullptr || thinkingTimer_ == nullptr || !thinkingTimer_->isActive()) {
        return;
    }
    static const QStringList frames {
        QStringLiteral("◜"),
        QStringLiteral("◠"),
        QStringLiteral("◝"),
        QStringLiteral("◞"),
        QStringLiteral("◡"),
        QStringLiteral("◟")
    };
    const qint64 seconds = thinkingElapsed_.isValid() ? thinkingElapsed_.elapsed() / 1000 : 0;
    const QString dots(3 + (thinkingFrame_ / 2) % 4, QLatin1Char('.'));
    sendButton_->setText(QStringLiteral("处理中 %1").arg(frames[thinkingFrame_ % frames.size()]));
    if (thinkingMessage_ != nullptr) {
        thinkingMessage_->setMessage(QString(),
                                     QStringLiteral("深度思考中%1 %2s").arg(dots).arg(seconds));
    }
    ++thinkingFrame_;
}

void ChatPage::showThinkingMessage() {
    if (thinkingMessage_ != nullptr || conversationMessagesLayout_ == nullptr) {
        return;
    }
    thinkingMessage_ = new ChatMessageWidget(ChatMessageWidget::Role::Assistant, conversationHost_);
    thinkingMessage_->setObjectName(QStringLiteral("thinkingMessage"));
    thinkingMessage_->setMessage(QString(), QStringLiteral("深度思考中... 0s"));
    const int insertIndex = qMax(0, conversationMessagesLayout_->count() - 1);
    conversationMessagesLayout_->insertWidget(insertIndex, thinkingMessage_, 0, Qt::AlignTop);
    if (conversationScroll_ != nullptr && conversationScroll_->verticalScrollBar() != nullptr) {
        conversationScroll_->verticalScrollBar()->setValue(
            conversationScroll_->verticalScrollBar()->maximum());
    }
}

void ChatPage::removeThinkingMessage() {
    if (thinkingMessage_ == nullptr) {
        return;
    }
    delete thinkingMessage_.data();
    thinkingMessage_.clear();
}
