#pragma once

#include "BasePage.h"

class QTextEdit;

class LogsPage : public BasePage {
public:
    explicit LogsPage(QWidget* parent = nullptr);
    void setDemoLog();
    void setLogText(const QString& text);

private:
    QTextEdit* logView_ = nullptr;
};
