#include "ChatMessageWidget.h"

#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QTextDocument>
#include <QTextBrowser>
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

class MessageTextBrowser final : public QTextBrowser {
public:
    explicit MessageTextBrowser(QWidget* parent = nullptr)
        : QTextBrowser(parent) {
        setFrameShape(QFrame::NoFrame);
        setOpenExternalLinks(true);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setReadOnly(true);
        setObjectName("messageBody");
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setStyleSheet("background: transparent; border: none;");
        document()->setDocumentMargin(0);
        QObject::connect(document(), &QTextDocument::contentsChanged, this, [this]() {
            updateHeight();
        });
    }

    void updateHeight() {
        document()->setTextWidth(viewport()->width());
        const int documentHeight = qCeil(document()->size().height()) + 8;
        setMinimumHeight(documentHeight);
        setMaximumHeight(documentHeight + 4);
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QTextBrowser::resizeEvent(event);
        updateHeight();
    }
};
}

ChatMessageWidget::ChatMessageWidget(Role role, QWidget* parent)
    : QWidget(parent)
    , role_(role) {
    const QString roleKey = role == Role::User ? QStringLiteral("user")
        : role == Role::Assistant ? QStringLiteral("assistant")
                                  : QStringLiteral("system");
    setObjectName(role == Role::User ? "userMessage" : role == Role::Assistant ? "assistantMessage" : "systemMessage");
    setProperty("role", roleKey);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
    bubble_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bubble_->setMinimumWidth(260);
    auto* bubbleLayout = new QVBoxLayout(bubble_);
    bubbleLayout->setContentsMargins(18, 16, 18, 16);
    bubbleLayout->setSpacing(10);

    title_ = new QLabel(this);
    title_->setObjectName("messageTitle");
    body_ = new MessageTextBrowser(this);

    image_ = new QLabel(this);
    image_->setObjectName("messageImage");
    image_->setMinimumHeight(128);
    image_->setAlignment(Qt::AlignCenter);
    image_->setWordWrap(true);
    image_->hide();

    bubbleLayout->addWidget(title_);
    bubbleLayout->addWidget(body_);
    bubbleLayout->addWidget(image_);

    // 中文注释：用户消息采用右侧出气泡的常见聊天布局，AI / 系统消息保持左侧。
    if (role == Role::User) {
        root->addStretch(1);
        root->addWidget(bubble_, 1, Qt::AlignTop);
        root->addWidget(avatar_, 0, Qt::AlignTop);
    } else {
        root->addWidget(avatar_, 0, Qt::AlignTop);
        root->addWidget(bubble_, 1, Qt::AlignTop);
        root->addStretch(1);
    }

    updateBubbleWidth();
}

void ChatMessageWidget::setMessage(const QString& title, const QString& body, const QString& imagePath) {
    title_->setText(title);
    body_->setPlainText(body);
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

void ChatMessageWidget::setRichMessage(const QString& title, const QString& htmlBody, const QString& imagePath) {
    title_->setText(title);
    body_->setHtml(htmlBody);
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

void ChatMessageWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateBubbleWidth();
}

void ChatMessageWidget::updateBubbleWidth() {
    if (bubble_ == nullptr) {
        return;
    }

    const int available = qMax(320, width());
    const qreal ratio = role_ == Role::User ? 0.72 : 0.94;
    const int maxWidth = qBound(260, static_cast<int>(available * ratio), 1200);
    bubble_->setMaximumWidth(maxWidth);
    bubble_->updateGeometry();
}
