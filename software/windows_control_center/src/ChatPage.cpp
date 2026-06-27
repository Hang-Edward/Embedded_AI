#include "ChatPage.h"

#include "DeepSeekChatClient.h"
#include "QwenVisionQtClient.h"
#include "WebView2Widget.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
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

    conversationContainer_ = new QWidget(conversationCard);
    conversationContainer_->setObjectName("chatScroll");
    conversationContainer_->setFocusPolicy(Qt::StrongFocus);
    conversationContainer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    conversationContainer_->setMinimumHeight(420);
    auto* conversationPlaceholderLayout = new QVBoxLayout(conversationContainer_);
    conversationPlaceholderLayout->setContentsMargins(0, 0, 0, 0);
    conversationPlaceholderLayout->setSpacing(0);

    conversationWebView_ = new WebView2Widget(conversationContainer_);
    conversationWebView_->setObjectName("chatConversationWebView");
    conversationWebView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    conversationWebView_->setMinimumHeight(420);
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
    conversationLayout->addWidget(conversationContainer_, 1);
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
    Q_UNUSED(this)
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
    if (conversationWebView_ != nullptr) {
        conversationWebView_->setHtmlContent(buildConversationHtml());
    }
}

QString ChatPage::buildConversationHtml() const {
    const QString payload = jsonBase64(uiMessages_);
    return QStringLiteral(R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <style>
    :root {
      color-scheme: dark;
      --bg: rgba(0,0,0,0);
      --bubble-ai: rgba(8, 18, 38, 0.78);
      --bubble-user: rgba(18, 34, 70, 0.82);
      --bubble-system: rgba(10, 22, 42, 0.72);
      --border: rgba(170, 220, 255, 0.16);
      --text: #eaf4ff;
      --muted: #b9d4f2;
      --accent: #8ec5ff;
    }
    * { box-sizing: border-box; }
    html, body {
      margin: 0;
      padding: 0;
      background: transparent;
      color: var(--text);
      font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif;
      overflow-x: hidden;
      scroll-behavior: smooth;
    }
    body {
      padding: 16px 18px 22px 18px;
    }
    .conversation {
      display: flex;
      flex-direction: column;
      gap: 18px;
      width: 100%;
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
    .message.user .avatar { order: 2; }
    .message.user .bubble { order: 1; max-width: 72%; background: var(--bubble-user); }
    .message.assistant .bubble { max-width: 94%; background: var(--bubble-ai); }
    .message.system .bubble { max-width: 90%; background: var(--bubble-system); }
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
      border-radius: 18px;
      padding: 18px 20px;
      min-width: 260px;
      backdrop-filter: blur(14px);
      -webkit-backdrop-filter: blur(14px);
      box-shadow: inset 0 1px 0 rgba(255,255,255,0.04);
    }
    .title {
      font-size: 15px;
      font-weight: 800;
      margin-bottom: 10px;
      color: #ffffff;
    }
    .body {
      color: var(--text);
      font-size: 14px;
      line-height: 1.72;
      word-break: break-word;
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
    .message-image {
      margin-top: 12px;
      max-width: min(560px, 100%);
      border-radius: 16px;
      border: 1px solid rgba(155, 210, 255, 0.22);
      display: block;
    }
    .MathJax, mjx-container {
      font-size: 0.94em !important;
    }
    mjx-container[display="true"] {
      margin: 0.55em auto !important;
      max-width: 100%;
      overflow-x: auto;
      overflow-y: hidden;
    }
    ::-webkit-scrollbar { width: 10px; height: 10px; }
    ::-webkit-scrollbar-thumb {
      background: rgba(126, 181, 255, 0.42);
      border-radius: 999px;
    }
    ::-webkit-scrollbar-track { background: transparent; }
  </style>
  <script>
    window.MathJax = {
      tex: {
        inlineMath: [['$', '$'], ['\\(', '\\)']],
        displayMath: [['$$', '$$'], ['\\[', '\\]']]
      },
      chtml: { scale: 0.92 },
      svg: { fontCache: 'global' }
    };
  </script>
  <script src="https://cdn.jsdelivr.net/npm/marked/marked.min.js"></script>
  <script async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
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

    function renderBody(role, body) {
      if (role === 'assistant') {
        return marked.parse(body || '', { breaks: true, gfm: true });
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

      const finalizeScroll = () => window.scrollTo({ top: document.body.scrollHeight, behavior: 'auto' });
      if (window.MathJax && window.MathJax.typesetPromise) {
        window.MathJax.typesetPromise([root]).then(finalizeScroll).catch(finalizeScroll);
      } else {
        finalizeScroll();
      }
    }

    if (document.readyState === 'loading') {
      document.addEventListener('DOMContentLoaded', renderConversation);
    } else {
      renderConversation();
    }
  </script>
</body>
</html>
)HTML").replace(QStringLiteral("__PAYLOAD_BASE64__"), payload);
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
