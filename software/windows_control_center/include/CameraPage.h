#pragma once

#include "BasePage.h"

class QLabel;

class CameraPage : public BasePage {
public:
    explicit CameraPage(QWidget* parent = nullptr);
    void setImagePath(const QString& imagePath);
    void setStatusText(const QString& text);

private:
    QLabel* preview_ = nullptr;
    QString lastImagePath_;
    QString lastStatusText_;
};
