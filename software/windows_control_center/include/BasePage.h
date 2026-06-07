#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;

class BasePage : public QWidget {
public:
    explicit BasePage(const QString& title, const QString& subtitle, QWidget* parent = nullptr);

protected:
    QVBoxLayout* bodyLayout() const;
    QLabel* titleLabel() const;

private:
    QLabel* title_ = nullptr;
    QLabel* subtitle_ = nullptr;
    QVBoxLayout* body_ = nullptr;
};
