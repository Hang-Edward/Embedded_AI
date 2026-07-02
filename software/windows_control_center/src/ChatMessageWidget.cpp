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
class AvatarBadge final : public QLabel {
public:
    explicit AvatarBadge(const QString& text, const QString& roleKey, QWidget* parent = nullptr)
        : QLabel(text, parent)
        , roleKey_(roleKey) {
        setFixedSize(36, 36);
        setAlignment(Qt::AlignCenter);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QColor fill(75, 150, 255, 118);
        QColor border(218, 240, 255, 88);
        QColor textColor(Qt::white);

        if (roleKey_ == QStringLiteral("assistant")) {
            fill = QColor(103, 126, 255, 96);
            border = QColor(211, 223, 255, 76);
        } else if (roleKey_ == QStringLiteral("system")) {
            fill = QColor(105, 125, 154, 72);
            border = QColor(210, 224, 240, 62);
            textColor = QColor(217, 230, 243);
        }

        const QRectF bounds = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        painter.setBrush(fill);
        painter.setPen(QPen(border, 1.0));
        painter.drawEllipse(bounds);

        painter.setPen(textColor);
        QFont font = painter.font();
        font.setBold(true);
        font.setPointSize(roleKey_ == QStringLiteral("system") ? 9 : 10);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, text());
    }

private:
    QString roleKey_;
};

class DarkMessageBubble final : public QWidget {
public:
    explicit DarkMessageBubble(bool paintedBackground, QWidget* parent = nullptr)
        : QWidget(parent)
        , paintedBackground_(paintedBackground) {
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event)
        if (!paintedBackground_) {
            return;
        }
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRectF bounds = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        QPainterPath shape;
        shape.addRoundedRect(bounds, 16.0, 16.0);
        // 中文注释：色值对齐原始日志框，让气泡保持半透明深蓝，而不是过度压暗。
        painter.fillPath(shape, QColor(8, 18, 38, 92));
        painter.setPen(QPen(QColor(170, 220, 255, 58), 1.0));
        painter.drawPath(shape);
    }

private:
    bool paintedBackground_ = true;
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
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAutoFillBackground(false);
        setStyleSheet("background: transparent; border: none;");
        viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
        viewport()->setAttribute(Qt::WA_NoSystemBackground, true);
        viewport()->setAutoFillBackground(false);
        viewport()->setStyleSheet("background: transparent; border: none;");
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

    avatar_ = new AvatarBadge(role == Role::User ? QStringLiteral("我")
                                                 : role == Role::Assistant ? QStringLiteral("AI")
                                                                           : QStringLiteral("系统"),
                              roleKey,
                              this);
    avatar_->setObjectName("avatar");
    avatar_->setProperty("role", roleKey);

    useBubbleBackground_ = (role == Role::User);
    bubble_ = new DarkMessageBubble(useBubbleBackground_, this);
    bubble_->setObjectName("messageBubble");
    bubble_->setProperty("role", roleKey);
    bubble_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bubble_->setMinimumWidth(role == Role::User ? 260 : 0);
    auto* bubbleLayout = new QVBoxLayout(bubble_);
    bubbleLayout->setContentsMargins(useBubbleBackground_ ? 18 : 0,
                                     useBubbleBackground_ ? 16 : 0,
                                     useBubbleBackground_ ? 18 : 0,
                                     useBubbleBackground_ ? 16 : 0);
    bubbleLayout->setSpacing(useBubbleBackground_ ? 10 : 8);

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

    // 中文注释：用户消息靠右；AI / 系统消息采用“左侧身份 + 中置大气泡”的布局，
    // 右侧补一个与头像区等宽的留白，避免视觉上只占左半边。
    if (role != Role::User) {
        title_->hide();
    }

    if (role == Role::User) {
        root->addSpacing(mirroredInsetWidth_);
        root->addStretch(1);
        root->addWidget(bubble_, 1, Qt::AlignTop);
        root->addWidget(avatar_, 0, Qt::AlignTop);
    } else {
        avatar_->hide();
        root->addWidget(bubble_, 1, Qt::AlignTop);
    }

    updateBubbleWidth();
}

void ChatMessageWidget::setMessage(const QString& title, const QString& body, const QString& imagePath) {
    title_->setText(title);
    title_->setVisible(role_ == Role::User && !title.trimmed().isEmpty());
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
    title_->setVisible(role_ == Role::User && !title.trimmed().isEmpty());
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

    const int reserved = role_ == Role::User ? mirroredInsetWidth_ + 46 : 24;
    const int available = qMax(320, width() - reserved);
    const qreal ratio = role_ == Role::User ? 0.72 : 0.96;
    const int maxWidth = qBound(260, static_cast<int>(available * ratio), 1280);
    bubble_->setMaximumWidth(maxWidth);
    bubble_->updateGeometry();
}
