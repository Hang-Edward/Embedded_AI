#pragma once

#include "BasePage.h"
#include "ChatSessionModels.h"

#include <functional>

class QLabel;
class QListWidget;
class QTextEdit;

class HistoryPage : public BasePage {
public:
    explicit HistoryPage(QWidget* parent = nullptr);
    void setSessions(const QList<ArchivedChatSession>& sessions);
    void setSessionActivatedCallback(std::function<void(const QString&)> callback);

private:
    void showRecord(int row);

    QListWidget* list_ = nullptr;
    QLabel* image_ = nullptr;
    QTextEdit* detail_ = nullptr;
    QList<ArchivedChatSession> sessions_;
    QString sessionsKey_;
    std::function<void(const QString&)> sessionActivatedCallback_;
};
