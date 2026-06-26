#include "ChatMessageWidget.h"

#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace {
class DarkMessageBubble final : public QWidget {
public:
    explicit DarkMessageBubble(QWidget* parent = nullptr)
        : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRectF bounds = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        QPainterPath shape;
        shape.addRoundedRect(bounds, 16.0, 16.0);
        // 中文注释：色值对齐原始日志框，让气泡保持半透明深蓝，而不是过度压暗。
        painter.fillPath(shape, QColor(8, 18, 38, 118));
        painter.setPen(QPen(QColor(170, 220, 255, 58), 1.0));
        painter.drawPath(shape);
    }
};

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
    const QString roleKey = role == Role::User ? QStringLiteral("user")
        : role == Role::Assistant ? QStringLiteral("assistant")
                                  : QStringLiteral("system");
    setObjectName(role == Role::User ? "userMessage" : role == Role::Assistant ? "assistantMessage" : "systemMessage");
    setProperty("role", roleKey);
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(10);

    avatar_ = new QLabel(role == Role::User ? "我" : role == Role::Assistant ? "AI" : "系统", this);
    avatar_->setObjectName("avatar");
    avatar_->setProperty("role", roleKey);
    avatar_->setFixedSize(36, 36);
    avatar_->setAlignment(Qt::AlignCenter);

    bubble_ = new DarkMessageBubble(this);
    bubble_->setObjectName("messageBubble");
    bubble_->setProperty("role", roleKey);
    bubble_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    bubble_->setMaximumWidth(820);
    auto* bubbleLayout = new QVBoxLayout(bubble_);
    bubbleLayout->setContentsMargins(18, 16, 18, 16);
    bubbleLayout->setSpacing(10);

    title_ = new QLabel(this);
    title_->setObjectName("messageTitle");
    body_ = new QLabel(this);
    body_->setObjectName("messageBody");
    body_->setWordWrap(true);
    body_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    image_ = new QLabel(this);
    image_->setObjectName("messageImage");
    image_->setMinimumHeight(128);
    image_->setAlignment(Qt::AlignCenter);
    image_->setWordWrap(true);
    image_->hide();

    bubbleLayout->addWidget(title_);
    bubbleLayout->addWidget(body_);
    bubbleLayout->addWidget(image_);

    root->addWidget(avatar_, 0, Qt::AlignTop);
    root->addWidget(bubble_, 0, Qt::AlignTop);
    root->addStretch(1);
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
            image_->setPixmap(pixmap.scaled(520, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            image_->setText("图片已缓存，但暂时无法读取：\n" + imagePath + "\n\nQt 错误：" + error);
        }
        image_->show();
    }
}
