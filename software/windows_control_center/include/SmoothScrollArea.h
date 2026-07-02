#pragma once

#include <QScrollArea>

class QPropertyAnimation;
class QWheelEvent;
class QScrollBar;

class SmoothScrollArea : public QScrollArea {
public:
    explicit SmoothScrollArea(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    int animationBaseValue() const;
    QPropertyAnimation* scrollAnimation_ = nullptr;
};
