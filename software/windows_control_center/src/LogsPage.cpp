#include "LogsPage.h"

#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

LogsPage::LogsPage(QWidget* parent)
    : BasePage("原始日志", "通过 SSH 读取 ~/Embedded_AI/logs/embedded-ai.log 的尾部内容。", parent) {
    logView_ = new QTextEdit(this);
    logView_->setObjectName("logView");
    logView_->setReadOnly(true);
    bodyLayout()->addWidget(logView_, 1);
    setDemoLog();
}

void LogsPage::setDemoLog() {
    logView_->setPlainText(
        "[等待] 连接树莓派后会加载 embedded-ai.log\n"
        "[提示] 树莓派在线且 SSH key 登录正常后，点击重新连接。\n");
}

void LogsPage::setLogText(const QString& text) {
    logView_->setPlainText(text.trimmed().isEmpty() ? "树莓派没有返回日志内容。" : text);
    logView_->moveCursor(QTextCursor::End);
}
