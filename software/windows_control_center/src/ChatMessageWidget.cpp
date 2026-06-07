#include "ChatMessageWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

ChatMessageWidget::ChatMessageWidget(Role role, QWidget* parent)
    : QWidget(parent) {
    setObjectName(role == Role::User ? "userMessage" : role == Role::Assistant ? "assistantMessage" : "systemMessage");
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(12);

    avatar_ = new QLabel(role == Role::User ? "You" : role == Role::Assistant ? "AI" : "Sys", this);
    avatar_->setObjectName("avatar");
    avatar_->setFixedSize(42, 42);
    avatar_->setAlignment(Qt::AlignCenter);

    auto* bubble = new QWidget(this);
    bubble->setObjectName("messageBubble");
    auto* bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(16, 14, 16, 14);
    bubbleLayout->setSpacing(8);
    title_ = new QLabel(this);
    title_->setObjectName("messageTitle");
    body_ = new QLabel(this);
    body_->setObjectName("messageBody");
    body_->setWordWrap(true);
    image_ = new QLabel(this);
    image_->setObjectName("messageImage");
    image_->setMinimumHeight(120);
    image_->setAlignment(Qt::AlignCenter);
    image_->hide();
    bubbleLayout->addWidget(title_);
    bubbleLayout->addWidget(body_);
    bubbleLayout->addWidget(image_);

    root->addWidget(avatar_, 0, Qt::AlignTop);
    root->addWidget(bubble, 1);
}

void ChatMessageWidget::setMessage(const QString& title, const QString& body, const QString& imagePath) {
    title_->setText(title);
    body_->setText(body);
    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            image_->setPixmap(pixmap.scaled(360, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            image_->setText("Image attachment: " + imagePath);
        }
        image_->show();
    }
}
