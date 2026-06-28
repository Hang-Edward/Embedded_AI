#include "ChatPage.h"

#include "DeepSeekChatClient.h"
#include "GlassSurface.h"
#include "QwenVisionQtClient.h"
#include "WebView2Widget.h"

#include <QFrame>
#include <QFile>
#include <QHBoxLayout>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QPixmap>
#include <QSizePolicy>
#include <QStyle>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
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

QString jsonBase64(const QList<AgentUiMessage>& messages) {
    QJsonArray array;
    for (const AgentUiMessage& message : messages) {
        array.append(QJsonObject {
            {"role", message.role},
            {"title", message.title},
            {"body", message.rawText},
            {"imagePath", message.imagePath}
        });
    }
    return QString::fromLatin1(QJsonDocument(array).toJson(QJsonDocument::Compact).toBase64());
}

QString loadResourceText(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString htmlEscapedForStyle(const QString& text) {
    QString escaped = text;
    escaped.replace(QStringLiteral("</style>"), QStringLiteral("<\\/style>"), Qt::CaseInsensitive);
    return escaped;
}

QString htmlEscapedForScript(const QString& text) {
    QString escaped = text;
    escaped.replace(QStringLiteral("</script>"), QStringLiteral("<\\/script>"), Qt::CaseInsensitive);
    return escaped;
}

} // namespace

ChatPage::ChatPage(AppConfig& config, QWidget* parent)
    : BasePage("实时对话", "这里是主工作台：输入需求，DeepSeek 负责推理与回复，Qwen 只负责视觉观察。", parent)
    , config_(config) {
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
    conversationCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* conversationLayout = new QVBoxLayout(conversationCard);
    conversationLayout->setContentsMargins(20, 18, 20, 16);
    conversationLayout->setSpacing(14);

    conversationContainer_ = new QWidget(conversationCard);
    conversationContainer_->setObjectName("chatScroll");
    conversationContainer_->setFocusPolicy(Qt::StrongFocus);
    conversationContainer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    conversationContainer_->setMinimumHeight(650);
    auto* conversationPlaceholderLayout = new QVBoxLayout(conversationContainer_);
    conversationPlaceholderLayout->setContentsMargins(0, 0, 0, 0);
    conversationPlaceholderLayout->setSpacing(0);

    conversationWebView_ = new WebView2Widget(conversationContainer_);
    conversationWebView_->setObjectName("chatConversationWebView");
    conversationWebView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    conversationWebView_->setMinimumHeight(650);
    QObject::connect(conversationWebView_, &WebView2Widget::initializationFailed, this, [this](const QString& reason) {
        appendUiMessage({QStringLiteral("system"),
                         QStringLiteral("聊天渲染器初始化失败"),
                         reason,
                         QString(),
                         QString()});
        setAnimatedLabelText(stageStatus_, QStringLiteral("聊天渲染器初始化失败：%1").arg(reason), false, 220);
    });
    conversationPlaceholderLayout->addWidget(conversationWebView_, 1);

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
    Q_UNUSED(this)
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
    if (conversationWebView_ != nullptr) {
        conversationWebView_->setHtmlContent(buildConversationHtml());
    }
}

QString ChatPage::buildConversationHtml() const {
    const QString payload = jsonBase64(uiMessages_);
    const QString markedJs = htmlEscapedForScript(loadResourceText(QStringLiteral(":/assets/web/marked.min.js")));
    const QString katexJs = htmlEscapedForScript(loadResourceText(QStringLiteral(":/assets/web/katex.min.js")));
    const QString katexAutoRenderJs = htmlEscapedForScript(loadResourceText(QStringLiteral(":/assets/web/auto-render.min.js")));
    const QString katexCss = htmlEscapedForStyle(loadResourceText(QStringLiteral(":/assets/web/katex.min.css")));
    return QStringLiteral(R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <style>
__KATEX_CSS__
    :root {
      color-scheme: dark;
      --bg: rgba(31, 61, 115, 0.34);
      --bubble-ai: rgba(8, 18, 38, 0.46);
      --bubble-user: rgba(18, 34, 70, 0.54);
      --bubble-system: rgba(10, 22, 42, 0.44);
      --border: rgba(170, 220, 255, 0.16);
      --text: #eaf4ff;
      --muted: #b9d4f2;
      --accent: #8ec5ff;
    }
    * { box-sizing: border-box; }
    html, body {
      margin: 0;
      padding: 0;
      background: linear-gradient(180deg, rgba(31, 61, 115, 0.34) 0%, rgba(20, 42, 84, 0.28) 100%);
      background-repeat: no-repeat;
      background-size: 100% 100%;
      background-attachment: fixed;
      color: var(--text);
      font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif;
      overflow-x: hidden;
      scroll-behavior: smooth;
      min-height: 100%;
    }
    body {
      padding: 18px 20px 26px 20px;
      border-radius: 22px;
      box-shadow: inset 0 1px 0 rgba(255,255,255,0.02);
      min-height: calc(100vh - 44px);
    }
    .conversation {
      display: flex;
      flex-direction: column;
      gap: 22px;
      width: 100%;
      max-width: 1180px;
      margin: 0 auto 0 0;
    }
    .message {
      display: flex;
      gap: 12px;
      align-items: flex-start;
      width: 100%;
    }
    .message.user {
      justify-content: flex-end;
    }
    .message.assistant,
    .message.system {
      justify-content: flex-start;
    }
    .message.user .avatar { order: 2; }
    .message.user .bubble { order: 1; max-width: 62%; background: var(--bubble-user); }
    .message.assistant .bubble { max-width: 80%; background: var(--bubble-ai); }
    .message.system .bubble { max-width: 76%; background: var(--bubble-system); }
    .avatar {
      flex: 0 0 36px;
      width: 36px;
      height: 36px;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: 800;
      color: #fff;
      background: rgba(103,126,255,0.35);
      border: 1px solid rgba(211,223,255,0.30);
      user-select: none;
    }
    .message.user .avatar { background: rgba(75,150,255,0.50); }
    .message.system .avatar { background: rgba(105,125,154,0.28); color: #d9e6f3; }
    .bubble {
      border: 1px solid var(--border);
      border-radius: 20px;
      padding: 18px 22px;
      min-width: 220px;
      backdrop-filter: blur(14px);
      -webkit-backdrop-filter: blur(14px);
      box-shadow: inset 0 1px 0 rgba(255,255,255,0.04), 0 16px 38px rgba(2, 8, 22, 0.12);
    }
    .title {
      font-size: 15px;
      font-weight: 800;
      margin-bottom: 10px;
      color: #ffffff;
    }
    .body {
      color: var(--text);
      font-size: 15px;
      line-height: 1.78;
      word-break: break-word;
      overflow-wrap: anywhere;
    }
    .body h1, .body h2, .body h3, .body h4 {
      margin: 0.55em 0 0.38em 0;
      line-height: 1.22;
    }
    .body h1 { font-size: 1.45em; }
    .body h2 { font-size: 1.28em; }
    .body h3 { font-size: 1.14em; }
    .body p { margin: 0.42em 0; }
    .body ul, .body ol { margin: 0.4em 0 0.55em 1.3em; }
    .body li { margin: 0.22em 0; }
    .body code {
      background: rgba(255,255,255,0.06);
      padding: 2px 6px;
      border-radius: 6px;
      font-family: Consolas, monospace;
    }
    .body pre {
      background: rgba(5,14,32,0.88);
      padding: 12px;
      border-radius: 12px;
      overflow-x: auto;
    }
    .body blockquote {
      border-left: 3px solid rgba(118,184,255,0.55);
      margin: 0.5em 0;
      padding-left: 12px;
      color: #c8ddf5;
    }
    .body table {
      width: 100%;
      border-collapse: collapse;
      margin: 0.7em 0;
      font-size: 13px;
    }
    .body th, .body td {
      border: 1px solid rgba(160,210,255,0.18);
      padding: 8px 10px;
      text-align: left;
      vertical-align: top;
    }
    .message-image {
      margin-top: 12px;
      max-width: min(560px, 100%);
      border-radius: 16px;
      border: 1px solid rgba(155, 210, 255, 0.22);
      display: block;
    }
    .body .katex {
      font-size: 0.98em;
      color: var(--text);
    }
    .body .katex-display {
      margin: 0.42em 0 0.58em 0;
      max-width: 100%;
      overflow-x: auto;
      overflow-y: hidden;
      padding: 0.08em 0 0.18em 0;
    }
    .body .katex-display > .katex {
      display: inline-block;
      text-align: left;
    }
    .body hr {
      border: none;
      border-top: 1px solid rgba(170,220,255,0.20);
      margin: 1em 0;
    }
    ::-webkit-scrollbar { width: 10px; height: 10px; }
    ::-webkit-scrollbar-thumb {
      background: rgba(126, 181, 255, 0.42);
      border-radius: 999px;
    }
    ::-webkit-scrollbar-track { background: transparent; }
  </style>
  <script>__MARKED_JS__</script>
  <script>__KATEX_JS__</script>
  <script>__KATEX_AUTORENDER_JS__</script>
</head>
<body>
  <div id="conversation" class="conversation"></div>
  <script>
    function decodePayload(base64) {
      const binary = atob(base64);
      const bytes = Uint8Array.from(binary, c => c.charCodeAt(0));
      return JSON.parse(new TextDecoder().decode(bytes));
    }

    function escapeHtml(text) {
      return text
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
    }

    function avatarText(role) {
      if (role === 'user') return '我';
      if (role === 'assistant') return 'AI';
      return '系统';
    }

    function protectMath(markdown) {
      const placeholders = [];
      let output = markdown || '';

      const store = (raw) => {
        const token = `EMBEDDED_AI_MATH_${placeholders.length}_TOKEN`;
        placeholders.push({ token, raw });
        return token;
      };

      output = output.replace(/\$\$([\s\S]+?)\$\$/g, (_, body) => store(`$$${body}$$`));
      output = output.replace(/\\\[([\s\S]+?)\\\]/g, (_, body) => store(`\\[${body}\\]`));
      output = output.replace(/\\\(([\s\S]+?)\\\)/g, (_, body) => store(`\\(${body}\\)`));
      output = output.replace(/(^|[^\\])\$([^\n$]+?)\$/g, (_, prefix, body) => `${prefix}${store(`$${body}$`)}`);

      return { text: output, placeholders };
    }

    function normalizeAssistantLatex(markdown) {
      if (!markdown) {
        return '';
      }

      // 中文注释：DeepSeek 有时会把 LaTeX 里的反斜杠再次转义成双反斜杠，
      // 比如 \\(、\\frac、\\sum。这里先把公式常见写法恢复成单反斜杠，
      // 再交给 Markdown 与 KaTeX 处理。
      return markdown
        .replace(/\\\\(?=[()[\]{}])/g, '\\')
        .replace(/\\\\(?=[A-Za-z])/g, '\\');
    }

    function restoreMath(html, placeholders) {
      let output = html;
      placeholders.forEach((item) => {
        output = output.split(item.token).join(item.raw);
      });
      return output;
    }

    marked.setOptions({
      gfm: true,
      breaks: true,
      headerIds: false,
      mangle: false
    });

    function renderBody(role, body) {
      if (role === 'assistant') {
        const normalizedBody = normalizeAssistantLatex(body || '');
        const protectedMath = protectMath(normalizedBody);
        const markdownHtml = marked.parse(protectedMath.text || '');
        return restoreMath(markdownHtml, protectedMath.placeholders);
      }
      return '<p>' + escapeHtml(body || '').replace(/\n/g, '<br/>') + '</p>';
    }

    function renderConversation() {
      const payload = decodePayload('__PAYLOAD_BASE64__');
      const root = document.getElementById('conversation');
      root.innerHTML = payload.map((message) => {
        const imageHtml = message.imagePath
          ? '<img class="message-image" src="' + encodeURI('file:///' + message.imagePath.replace(/\\/g, '/')) + '" />'
          : '';
        return `
          <section class="message ${message.role}">
            <div class="avatar">${avatarText(message.role)}</div>
            <div class="bubble">
              <div class="title">${escapeHtml(message.title || '')}</div>
              <div class="body">${renderBody(message.role, message.body || '')}</div>
              ${imageHtml}
            </div>
          </section>`;
      }).join('');

      if (window.renderMathInElement) {
        window.renderMathInElement(root, {
          throwOnError: false,
          strict: 'ignore',
          delimiters: [
            { left: '$$', right: '$$', display: true },
            { left: '\\[', right: '\\]', display: true },
            { left: '\\(', right: '\\)', display: false },
            { left: '$', right: '$', display: false }
          ]
        });
      }
      window.scrollTo({ top: document.body.scrollHeight, behavior: 'auto' });
    }

    if (document.readyState === 'loading') {
      document.addEventListener('DOMContentLoaded', renderConversation);
    } else {
      renderConversation();
    }
  </script>
</body>
</html>
)HTML")
        .replace(QStringLiteral("__PAYLOAD_BASE64__"), payload)
        .replace(QStringLiteral("__MARKED_JS__"), markedJs)
        .replace(QStringLiteral("__KATEX_JS__"), katexJs)
        .replace(QStringLiteral("__KATEX_AUTORENDER_JS__"), katexAutoRenderJs)
        .replace(QStringLiteral("__KATEX_CSS__"), katexCss);
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
