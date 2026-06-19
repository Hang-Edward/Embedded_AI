#include "ChatPage.h"

#include "ChatMessageWidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

QString stageTitleText(const ConnectionState& state) {
    switch (state.assistantStatus) {
    case AssistantStatus::Ready:
        return QStringLiteral("任务舞台已就绪");
    case AssistantStatus::Listening:
        return QStringLiteral("正在接收语音指令");
    case AssistantStatus::Thinking:
        return QStringLiteral("AI 正在分析当前画面");
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
    parts << QStringLiteral("触发：三键键盘 K-B");
    parts << QStringLiteral("摄像头：Logitech C270");
    if (!state.activeHost.isEmpty()) {
        parts << QStringLiteral("主机：%1").arg(state.activeHost);
    }
    if (!state.recentRecords.isEmpty()) {
        parts << QStringLiteral("最新记录：%1").arg(state.recentRecords.first().title);
    } else if (!state.localFramePath.isEmpty()) {
        parts << QStringLiteral("画面缓存：已同步");
    } else {
        parts << QStringLiteral("画面缓存：等待同步");
    }
    return parts.join(QStringLiteral("    ·    "));
}

} // namespace

ChatPage::ChatPage(QWidget* parent)
    : BasePage("实时对话", "按下三键键盘 K-B 后，语音输入、拍照画面和 AI 回复会像网页版 LLM 对话一样显示在这里。", parent) {
    auto* stagePanel = new QWidget(this);
    stagePanel->setObjectName("chatStagePanel");
    auto* stageLayout = new QVBoxLayout(stagePanel);
    stageLayout->setContentsMargins(16, 14, 16, 14);
    stageLayout->setSpacing(6);

    stageTitle_ = new QLabel(QStringLiteral("任务舞台已就绪"), stagePanel);
    stageTitle_->setObjectName("chatStageTitle");
    stageStatus_ = new QLabel(QStringLiteral("连接成功后，这里会即时显示录音、识别和分析阶段。"), stagePanel);
    stageStatus_->setObjectName("chatStageStatus");
    stageStatus_->setWordWrap(true);
    stageMeta_ = new QLabel(QStringLiteral("触发：三键键盘 K-B    ·    摄像头：Logitech C270    ·    结果：等待首条记录"), stagePanel);
    stageMeta_->setObjectName("chatStageMeta");
    stageMeta_->setWordWrap(true);

    stageLayout->addWidget(stageTitle_);
    stageLayout->addWidget(stageStatus_);
    stageLayout->addWidget(stageMeta_);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setObjectName("chatScroll");
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setAttribute(Qt::WA_TranslucentBackground, true);
    scroll->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    scroll->viewport()->setAutoFillBackground(false);

    auto* inner = new QWidget(scroll);
    inner->setAttribute(Qt::WA_TranslucentBackground, true);
    inner->setAutoFillBackground(false);
    messages_ = new QVBoxLayout(inner);
    messages_->setContentsMargins(4, 4, 4, 4);
    messages_->setSpacing(16);
    messages_->addStretch(1);
    scroll->setWidget(inner);

    triggerButton_ = new QPushButton("刷新最新对话", this);
    triggerButton_->setObjectName("primaryButton");
    QObject::connect(triggerButton_, &QPushButton::clicked, this, [this]() {
        appendDemoConversation();
    });

    bodyLayout()->addWidget(stagePanel);
    bodyLayout()->addWidget(scroll, 1);
    bodyLayout()->addWidget(triggerButton_, 0, Qt::AlignRight);
    appendDemoConversation();
}

void ChatPage::appendDemoConversation() {
    lastSessionKey_.clear();
    clearMessages();
    const int insertAt = qMax(0, messages_->count() - 1);
    auto* system = new ChatMessageWidget(ChatMessageWidget::Role::System, this);
    system->setMessage("等待硬件触发",
        "连接成功后，按下三键键盘 K-B 即可开始 5 秒语音输入。完成后这里会显示摄像头画面、识别流程和 AI 回答。");
    messages_->insertWidget(insertAt, system);

    ConnectionState placeholder;
    placeholder.assistantStatus = AssistantStatus::Connecting;
    updateStagePanel(placeholder);
}

void ChatPage::setLatestSession(const ConnectionState& state) {
    updateStagePanel(state);

    QString newKey;
    if (state.recentRecords.isEmpty()) {
        newKey = "empty|" + state.assistantStatusText + "|" + state.localFramePath;
    } else {
        const ConversationRecord& record = state.recentRecords.first();
        newKey = record.title + "|" + record.userText + "|" + record.flowText + "|" + record.aiText + "|" + record.imagePath + "|" + state.localFramePath;
    }
    if (newKey == lastSessionKey_) {
        return;
    }
    lastSessionKey_ = newKey;

    clearMessages();
    const int insertAt = qMax(0, messages_->count() - 1);

    if (state.recentRecords.isEmpty()) {
        auto* system = new ChatMessageWidget(ChatMessageWidget::Role::System, this);
        system->setMessage("暂无成功对话",
            state.assistantStatusText.isEmpty()
                ? "暂时没有从树莓派读取到完整的 AI 分析记录。"
                : state.assistantStatusText,
            state.localFramePath);
        messages_->insertWidget(insertAt, system);
        return;
    }

    const ConversationRecord& record = state.recentRecords.first();
    const QString imagePath = record.imagePath.isEmpty() ? state.localFramePath : record.imagePath;

    auto* user = new ChatMessageWidget(ChatMessageWidget::Role::User, this);
    user->setMessage("用户输入 / 摄像头画面", record.userText, imagePath);
    messages_->insertWidget(insertAt, user);

    auto* flow = new ChatMessageWidget(ChatMessageWidget::Role::System, this);
    flow->setMessage("执行流程", record.flowText.isEmpty() ? "暂无流程日志。" : record.flowText);
    messages_->insertWidget(insertAt + 1, flow);

    auto* assistant = new ChatMessageWidget(ChatMessageWidget::Role::Assistant, this);
    assistant->setMessage("AI 回复 / 分析", record.aiText.isEmpty() ? "当前日志中没有解析到 AI 回复。" : record.aiText);
    messages_->insertWidget(insertAt + 2, assistant);
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
    stageTitle_->setText(stageTitleText(state));
    stageStatus_->setText(stageStatusText(state));
    stageMeta_->setText(stageMetaText(state));
}
