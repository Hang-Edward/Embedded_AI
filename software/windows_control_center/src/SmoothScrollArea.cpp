#include "SmoothScrollArea.h"

#include <QPropertyAnimation>
#include <QScrollBar>
#include <QWheelEvent>

SmoothScrollArea::SmoothScrollArea(QWidget* parent)
    : QScrollArea(parent) {
    scrollAnimation_ = new QPropertyAnimation(verticalScrollBar(), "value", this);
    scrollAnimation_->setDuration(180);
    scrollAnimation_->setEasingCurve(QEasingCurve::OutCubic);
}

void SmoothScrollArea::wheelEvent(QWheelEvent* event) {
    if (verticalScrollBar() == nullptr || event == nullptr) {
        QScrollArea::wheelEvent(event);
        return;
    }

    const QPoint angle = event->angleDelta();
    if (angle.y() == 0) {
        QScrollArea::wheelEvent(event);
        return;
    }

    event->accept();

    // 中文注释：将滚轮离散步进改成短时动画，避免聊天区出现生硬“跳格”滚动。
    const int step = qMax(48, viewport()->height() / 7);
    const int currentValue = verticalScrollBar()->value();
    const int targetValue = qBound(
        verticalScrollBar()->minimum(),
        currentValue - (angle.y() / 120) * step,
        verticalScrollBar()->maximum());

    scrollAnimation_->stop();
    scrollAnimation_->setStartValue(currentValue);
    scrollAnimation_->setEndValue(targetValue);
    scrollAnimation_->start();
}
