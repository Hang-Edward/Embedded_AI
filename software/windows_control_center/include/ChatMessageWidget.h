#pragma once

#include <QWidget>

class QLabel;
class QResizeEvent;
class QTextBrowser;

class ChatMessageWidget : public QWidget {
public:
    enum class Role {
        User,
        Assistant,
        System
    };

    explicit ChatMessageWidget(Role role, QWidget* parent = nullptr);

    void setMessage(const QString& title, const QString& body, const QString& imagePath = QString());
    void setRichMessage(const QString& title, const QString& htmlBody, const QString& imagePath = QString());

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateBubbleWidth();

    Role role_ = Role::System;
    int mirroredInsetWidth_ = 46;
    QLabel* avatar_ = nullptr;
    QLabel* title_ = nullptr;
    QTextBrowser* body_ = nullptr;
    QLabel* image_ = nullptr;
    QWidget* bubble_ = nullptr;
    bool useBubbleBackground_ = true;
};
