#include "LogsPage.h"

#include <QTextEdit>
#include <QTextCursor>
#include <QVBoxLayout>

LogsPage::LogsPage(QWidget* parent)
    : BasePage("Event Logs", "Live tail from ~/Embedded_AI/logs/embedded-ai.log over SSH.", parent) {
    logView_ = new QTextEdit(this);
    logView_->setObjectName("logView");
    logView_->setReadOnly(true);
    bodyLayout()->addWidget(logView_, 1);
    setDemoLog();
}

void LogsPage::setDemoLog() {
    logView_->setPlainText(
        "[waiting] Connect to Raspberry Pi to load embedded-ai.log\n"
        "[hint] Press Reconnect after the Pi is online and SSH key login is ready.\n");
}

void LogsPage::setLogText(const QString& text) {
    logView_->setPlainText(text.trimmed().isEmpty() ? "No log output returned from Raspberry Pi." : text);
    logView_->moveCursor(QTextCursor::End);
}
