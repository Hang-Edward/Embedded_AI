#include "ChatPage.h"

#include "ChatMessageWidget.h"

#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QVBoxLayout>

ChatPage::ChatPage(QWidget* parent)
    : BasePage("实时对话", "按下三键键盘 K-B 后，语音输入、拍照画面和 AI 回复会像网页版 LLM 对话一样显示在这里。", parent) {
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

    bodyLayout()->addWidget(scroll, 1);
    bodyLayout()->addWidget(triggerButton_, 0, Qt::AlignRight);
    appendDemoConversation();
}

void ChatPage::appendDemoConversation() {
    clearMessages();
    const int insertAt = qMax(0, messages_->count() - 1);
    auto* system = new ChatMessageWidget(ChatMessageWidget::Role::System, this);
    system->setMessage("等待硬件触发",
        "连接成功后，按下三键键盘 K-B 即可开始 5 秒语音输入。完成后这里会显示摄像头画面、识别流程和 AI 回答。");
    messages_->insertWidget(insertAt, system);
}

void ChatPage::setLatestSession(const ConnectionState& state) {
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
