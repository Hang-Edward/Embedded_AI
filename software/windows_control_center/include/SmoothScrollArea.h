#pragma once

#include <QScrollArea>

class QPropertyAnimation;
class QWheelEvent;

class SmoothScrollArea : public QScrollArea {
public:
    explicit SmoothScrollArea(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    QPropertyAnimation* scrollAnimation_ = nullptr;
};
