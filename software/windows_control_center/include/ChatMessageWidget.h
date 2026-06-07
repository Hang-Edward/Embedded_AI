#pragma once

#include <QWidget>

class QLabel;

class ChatMessageWidget : public QWidget {
public:
    enum class Role {
        User,
        Assistant,
        System
    };

    explicit ChatMessageWidget(Role role, QWidget* parent = nullptr);

    void setMessage(const QString& title, const QString& body, const QString& imagePath = QString());

private:
    QLabel* avatar_ = nullptr;
    QLabel* title_ = nullptr;
    QLabel* body_ = nullptr;
    QLabel* image_ = nullptr;
};
