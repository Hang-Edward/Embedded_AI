#include "HistoryPage.h"

#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QListWidget>
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
    : BasePage("历史记录", "最近十次成功完成的 AI 分析。录音中、失败或未完成的流程不会写入这里。", parent) {
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    list_ = new QListWidget(splitter);
    list_->setObjectName("recentList");
    list_->setMinimumWidth(300);

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
}

void HistoryPage::setRecords(const ConnectionState& state) {
    QStringList keyParts;
    for (const ConversationRecord& record : state.recentRecords) {
        keyParts << record.title + "|" + record.imagePath;
    }
    const QString newKey = keyParts.join(";");
    if (newKey == recordsKey_) {
        return;
    }

    const int previousRow = list_->currentRow();
    recordsKey_ = newKey;
    records_ = state.recentRecords;
    list_->clear();
    for (int i = 0; i < records_.size(); ++i) {
        const ConversationRecord& record = records_[i];
        const QString label = QString("AI REPLY %1  %2").arg(-i).arg(record.userText.left(44));
        list_->addItem(label);
    }

    if (records_.isEmpty()) {
        image_->setPixmap(QPixmap());
        image_->setText("暂无历史图片。请按一次旋钮，等待 AI 完成分析。");
        detail_->setPlainText("暂无成功历史记录。");
        return;
    }

    const int row = previousRow >= 0 && previousRow < records_.size() ? previousRow : 0;
    list_->setCurrentRow(row);
    showRecord(row);
}

void HistoryPage::showRecord(int row) {
    if (row < 0 || row >= records_.size()) {
        return;
    }
    const ConversationRecord& record = records_[row];
    QString error;
    const QPixmap pixmap = loadPixmapFromFile(record.imagePath, &error);
    if (pixmap.isNull()) {
        image_->setPixmap(QPixmap());
        image_->setText(record.imagePath.isEmpty() ? "这条记录没有关联图片。" : "无法读取图片：\n" + record.imagePath + "\n\nQt 错误：" + error);
    } else {
        image_->setText(QString());
        image_->setPixmap(pixmap.scaled(760, 420, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        image_->setToolTip(record.imagePath);
    }

    detail_->setPlainText(
        "用户 / 触发：\n" + record.userText + "\n\n"
        "执行流程：\n" + record.flowText + "\n\n"
        "AI 回复：\n" + record.aiText + "\n\n"
        "本地图片：\n" + record.imagePath);
}
