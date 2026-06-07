#include "ChatMessageWidget.h"

#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

namespace {
QPixmap loadPixmapFromFile(const QString& imagePath, QString* error) {
    QImageReader reader(imagePath);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull()) {
        if (error) {
            *error = reader.errorString();
        }
        return {};
    }
    return QPixmap::fromImage(image);
}
}

ChatMessageWidget::ChatMessageWidget(Role role, QWidget* parent)
    : QWidget(parent) {
    setObjectName(role == Role::User ? "userMessage" : role == Role::Assistant ? "assistantMessage" : "systemMessage");
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(12);

    avatar_ = new QLabel(role == Role::User ? "我" : role == Role::Assistant ? "AI" : "系统", this);
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
    image_->setWordWrap(true);
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
    image_->setPixmap(QPixmap());
    image_->clear();
    image_->hide();

    if (!imagePath.isEmpty()) {
        QString error;
        const QPixmap pixmap = loadPixmapFromFile(imagePath, &error);
        if (!pixmap.isNull()) {
            image_->setPixmap(pixmap.scaled(420, 260, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            image_->setText("图片已缓存，但暂时无法读取：\n" + imagePath + "\n\nQt 错误：" + error);
        }
        image_->show();
    }
}
