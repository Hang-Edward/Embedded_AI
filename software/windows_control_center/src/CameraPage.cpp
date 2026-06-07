#include "CameraPage.h"

#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

CameraPage::CameraPage(QWidget* parent)
    : BasePage("Camera Preview", "Latest Raspberry Pi capture from Logitech C270.", parent) {
    preview_ = new QLabel("Waiting for SSH image fetch from ~/Embedded_AI/captures/latest-frame.jpg", this);
    preview_->setObjectName("cameraPreview");
    preview_->setAlignment(Qt::AlignCenter);
    preview_->setWordWrap(true);
    preview_->setMinimumHeight(360);
    bodyLayout()->addWidget(preview_, 1);
}

void CameraPage::setImagePath(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        setStatusText("No latest-frame.jpg has been fetched yet.\nPress the NUCLEO blue button or run one analysis on the Raspberry Pi.");
        return;
    }

    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        setStatusText("Fetched image exists, but Qt could not read it:\n" + imagePath);
        return;
    }

    preview_->setPixmap(pixmap.scaled(860, 520, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    preview_->setToolTip(imagePath);
}

void CameraPage::setStatusText(const QString& text) {
    preview_->setPixmap(QPixmap());
    preview_->setText(text);
}
