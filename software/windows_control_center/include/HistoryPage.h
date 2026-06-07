#pragma once

#include "BasePage.h"
#include "ConnectionState.h"

class QLabel;
class QListWidget;
class QTextEdit;

class HistoryPage : public BasePage {
public:
    explicit HistoryPage(QWidget* parent = nullptr);
    void setRecords(const ConnectionState& state);

private:
    void showRecord(int row);

    QListWidget* list_ = nullptr;
    QLabel* image_ = nullptr;
    QTextEdit* detail_ = nullptr;
    QList<ConversationRecord> records_;
    QString recordsKey_;
};
