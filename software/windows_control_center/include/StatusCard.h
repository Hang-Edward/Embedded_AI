#pragma once

#include "ConnectionState.h"

#include <QWidget>

class QLabel;

class StatusCard : public QWidget {
public:
    explicit StatusCard(QWidget* parent = nullptr);
    StatusCard(const QString& title, const QString& detail, HealthLevel level, QWidget* parent = nullptr);

    void setStatus(const QString& title, const QString& detail, HealthLevel level);

private:
    QLabel* indicator_ = nullptr;
    QLabel* title_ = nullptr;
    QLabel* detail_ = nullptr;
};
