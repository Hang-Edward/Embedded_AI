#include "CameraPage.h"

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

CameraPage::CameraPage(QWidget* parent)
    : BasePage("摄像头画面", "显示从树莓派拉取的 Logitech C270 最新抓拍图片。", parent) {
    preview_ = new QLabel("等待通过 SSH 拉取 ~/Embedded_AI/captures/latest-frame.jpg", this);
    preview_->setObjectName("cameraPreview");
    preview_->setAlignment(Qt::AlignCenter);
    preview_->setWordWrap(true);
    preview_->setMinimumHeight(360);
    bodyLayout()->addWidget(preview_, 1);
}

void CameraPage::setImagePath(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        setStatusText("尚未拉取 latest-frame.jpg。\n请按一次三键键盘 K-B，等待 AI 完成分析。");
        return;
    }

    if (imagePath == lastImagePath_) {
        return;
    }

    QString error;
    const QPixmap pixmap = loadPixmapFromFile(imagePath, &error);
    if (pixmap.isNull()) {
        setStatusText("图片已缓存，但 Qt 暂时无法读取：\n" + imagePath + "\n\nQt 错误：" + error);
        return;
    }

    preview_->setText(QString());
    preview_->setPixmap(pixmap.scaled(900, 560, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    preview_->setToolTip(imagePath);
    lastImagePath_ = imagePath;
    lastStatusText_.clear();
}

void CameraPage::setStatusText(const QString& text) {
    if (text == lastStatusText_) {
        return;
    }
    preview_->setPixmap(QPixmap());
    preview_->setText(text);
    preview_->setToolTip(QString());
    lastStatusText_ = text;
    lastImagePath_.clear();
}
