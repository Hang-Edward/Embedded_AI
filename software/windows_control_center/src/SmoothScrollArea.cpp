#include "SmoothScrollArea.h"

#include <QPalette>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QWheelEvent>

SmoothScrollArea::SmoothScrollArea(QWidget* parent)
    : QScrollArea(parent) {
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);
    setStyleSheet(QStringLiteral("background: transparent; border: none;"));

    if (viewport() != nullptr) {
        viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
        viewport()->setAttribute(Qt::WA_OpaquePaintEvent, false);
        viewport()->setAutoFillBackground(false);
        viewport()->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
        QPalette palette = viewport()->palette();
        palette.setColor(QPalette::Base, Qt::transparent);
        palette.setColor(QPalette::Window, Qt::transparent);
        viewport()->setPalette(palette);
    }

    scrollAnimation_ = new QPropertyAnimation(verticalScrollBar(), "value", this);
    scrollAnimation_->setDuration(220);
    scrollAnimation_->setEasingCurve(QEasingCurve::OutCubic);
}

void SmoothScrollArea::wheelEvent(QWheelEvent* event) {
    if (verticalScrollBar() == nullptr || event == nullptr) {
        QScrollArea::wheelEvent(event);
        return;
    }

    const QPoint pixel = event->pixelDelta();
    const QPoint angle = event->angleDelta();
    if (angle.y() == 0 && pixel.y() == 0) {
        QScrollArea::wheelEvent(event);
        return;
    }

    event->accept();

    // 中文注释：如果上一次动画还没结束，就从“目标值”继续推，而不是硬回到当前值重启动画，
    // 这样连续滚动会顺很多。
    const int step = qMax(42, viewport()->height() / 8);
    const int currentValue = animationBaseValue();
    int delta = 0;
    if (pixel.y() != 0) {
        delta = -pixel.y();
    } else {
        delta = -(angle.y() / 120) * step;
    }
    const int targetValue = qBound(
        verticalScrollBar()->minimum(),
        currentValue + delta,
        verticalScrollBar()->maximum());

    scrollAnimation_->stop();
    scrollAnimation_->setStartValue(currentValue);
    scrollAnimation_->setEndValue(targetValue);
    scrollAnimation_->start();
}

int SmoothScrollArea::animationBaseValue() const {
    if (verticalScrollBar() == nullptr || scrollAnimation_ == nullptr) {
        return 0;
    }
    if (scrollAnimation_->state() == QAbstractAnimation::Running) {
        const QVariant endValue = scrollAnimation_->endValue();
        if (endValue.isValid()) {
            return endValue.toInt();
        }
    }
    return verticalScrollBar()->value();
}
