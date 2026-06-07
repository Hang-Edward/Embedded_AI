#include "ChatPage.h"

#include "ChatMessageWidget.h"

#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

ChatPage::ChatPage(QWidget* parent)
    : BasePage("LLM Conversation", "Blue-button voice commands and Raspberry Pi camera frames are rendered as chat turns.", parent) {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setObjectName("chatScroll");
    auto* inner = new QWidget(scroll);
    messages_ = new QVBoxLayout(inner);
    messages_->setContentsMargins(4, 4, 4, 4);
    messages_->setSpacing(16);
    messages_->addStretch(1);
    scroll->setWidget(inner);

    triggerButton_ = new QPushButton("Refresh Latest Session", this);
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
    system->setMessage("Waiting for Raspberry Pi",
        "Connect over SSH to show the latest blue-button voice command, captured frame, and Qwen response here.");
    messages_->insertWidget(insertAt, system);
}

void ChatPage::setLatestSession(const QString& logText, const QString& imagePath) {
    clearMessages();
    const int insertAt = qMax(0, messages_->count() - 1);

    const QString tail = logText.right(1600).trimmed();
    auto* user = new ChatMessageWidget(ChatMessageWidget::Role::User, this);
    user->setMessage("Raspberry Pi latest capture",
        "Latest frame fetched from ~/Embedded_AI/captures/latest-frame.jpg.\n\nThe log tail below is the real output from the running assistant service.",
        imagePath);
    messages_->insertWidget(insertAt, user);

    auto* assistant = new ChatMessageWidget(ChatMessageWidget::Role::Assistant, this);
    assistant->setMessage("embedded-ai.log tail",
        tail.isEmpty() ? "No embedded-ai.log content returned yet." : tail);
    messages_->insertWidget(insertAt + 1, assistant);
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
