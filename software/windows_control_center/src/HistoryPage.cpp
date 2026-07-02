#include "HistoryPage.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QListWidget>
#include <QListView>
#include <QPixmap>
#include <QSplitter>
#include <QTextEdit>
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

HistoryPage::HistoryPage(QWidget* parent)
    : BasePage("历史记录", "这里保存的是你之前的对话会话。点击任意一条，就会切回实时对话并在那条会话上继续追问。", parent) {
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    list_ = new QListWidget(splitter);
    list_->setObjectName("recentList");
    list_->setMinimumWidth(300);
    list_->setWordWrap(true);
    list_->setTextElideMode(Qt::ElideNone);
    list_->setUniformItemSizes(false);
    list_->setResizeMode(QListView::Adjust);
    list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* detailPanel = new QWidget(splitter);
    auto* detailLayout = new QVBoxLayout(detailPanel);
    detailLayout->setContentsMargins(0, 0, 0, 0);
    detailLayout->setSpacing(12);

    image_ = new QLabel("暂无历史图片", detailPanel);
    image_->setObjectName("cameraPreview");
    image_->setAlignment(Qt::AlignCenter);
    image_->setMinimumHeight(300);
    image_->setWordWrap(true);

    detail_ = new QTextEdit(detailPanel);
    detail_->setObjectName("logView");
    detail_->setReadOnly(true);

    detailLayout->addWidget(image_, 1);
    detailLayout->addWidget(detail_, 1);
    splitter->addWidget(list_);
    splitter->addWidget(detailPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    bodyLayout()->addWidget(splitter, 1);

    QObject::connect(list_, &QListWidget::currentRowChanged, this, [this](int row) {
        showRecord(row);
    });
    QObject::connect(list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr || sessionActivatedCallback_ == nullptr) {
            return;
        }
        const int row = list_->row(item);
        if (row < 0 || row >= sessions_.size()) {
            return;
        }
        sessionActivatedCallback_(sessions_[row].sessionId);
    });
}

void HistoryPage::setSessionActivatedCallback(std::function<void(const QString&)> callback) {
    sessionActivatedCallback_ = std::move(callback);
}

void HistoryPage::setSessions(const QList<ArchivedChatSession>& sessions) {
    QStringList keyParts;
    for (const ArchivedChatSession& session : sessions) {
        keyParts << session.sessionId + "|" + session.timestamp + "|" + session.summary;
    }
    const QString newKey = keyParts.join(";");
    if (newKey == sessionsKey_) {
        return;
    }

    const int previousRow = list_->currentRow();
    sessionsKey_ = newKey;
    sessions_ = sessions;
    list_->clear();
    for (int i = 0; i < sessions_.size(); ++i) {
        const ArchivedChatSession& session = sessions_[i];
        const QString timeText = QDateTime::fromString(session.timestamp, Qt::ISODate).isValid()
            ? QDateTime::fromString(session.timestamp, Qt::ISODate).toLocalTime().toString("MM-dd hh:mm")
            : session.timestamp;
        const QString label = QString("%1\n%2\n%3").arg(timeText, session.title, session.summary);
        list_->addItem(label);
    }

    if (sessions_.isEmpty()) {
        image_->setPixmap(QPixmap());
        image_->setText("暂无历史会话。先在“实时对话”里完成一轮问答吧。");
        detail_->setPlainText("暂无历史会话。");
        return;
    }

    const int row = previousRow >= 0 && previousRow < sessions_.size() ? previousRow : 0;
    list_->setCurrentRow(row);
    showRecord(row);
}

void HistoryPage::showRecord(int row) {
    if (row < 0 || row >= sessions_.size()) {
        return;
    }
    const ArchivedChatSession& session = sessions_[row];
    QString imagePath;
    QString aiText;
    QString userText;
    for (int index = session.messages.size() - 1; index >= 0; --index) {
        const AgentUiMessage& message = session.messages[index];
        if (imagePath.isEmpty() && !message.imagePath.trimmed().isEmpty()) {
            imagePath = message.imagePath;
        }
        if (aiText.isEmpty() && message.role == QStringLiteral("assistant")) {
            aiText = message.rawText;
        }
        if (userText.isEmpty() && message.role == QStringLiteral("user")) {
            userText = message.rawText;
        }
    }
    QString error;
    const QPixmap pixmap = loadPixmapFromFile(imagePath, &error);
    if (pixmap.isNull()) {
        image_->setPixmap(QPixmap());
        image_->setText(imagePath.isEmpty() ? "这条会话没有关联图片。" : "无法读取图片：\n" + imagePath + "\n\nQt 错误：" + error);
    } else {
        image_->setText(QString());
        image_->setPixmap(pixmap.scaled(760, 420, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        image_->setToolTip(imagePath);
    }

    detail_->setPlainText(
        "会话标题：\n" + session.title + "\n\n"
        "时间：\n" + session.timestamp + "\n\n"
        "最近一次用户提问：\n" + userText + "\n\n"
        "最近一次 AI 回复：\n" + aiText + "\n\n"
        "消息条数：\n" + QString::number(session.messages.size()) + "\n\n"
        "关联图片：\n" + imagePath);
}
