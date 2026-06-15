#include "GlassSurface.h"

#include <QApplication>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QTimer>
#include <QVariantAnimation>
#include <QtMath>

#include <algorithm>

namespace {

qreal randomBetween(qreal minimum, qreal maximum) {
    return minimum + QRandomGenerator::global()->generateDouble() * (maximum - minimum);
}

} // namespace

LiquidNavButton::LiquidNavButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
    setObjectName("navButton");
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(54);
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFlat(true);

    hoverAnimation_ = new QVariantAnimation(this);
    hoverAnimation_->setDuration(150);
    hoverAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    connect(hoverAnimation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        hoverProgress_ = value.toReal();
        update();
    });

    selectionAnimation_ = new QVariantAnimation(this);
    selectionAnimation_->setDuration(230);
    selectionAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    connect(selectionAnimation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        selectionProgress_ = value.toReal();
        update();
    });
}

bool LiquidNavButton::event(QEvent* event) {
    if (event->type() == QEvent::Enter) {
        animateHover(1.0);
    } else if (event->type() == QEvent::Leave) {
        animateHover(0.0);
    } else if (event->type() == QEvent::DynamicPropertyChange) {
        animateSelection(property("active").toBool() ? 1.0 : 0.0);
    }
    return QPushButton::event(event);
}

void LiquidNavButton::animateHover(qreal target) {
    hoverAnimation_->stop();
    hoverAnimation_->setStartValue(hoverProgress_);
    hoverAnimation_->setEndValue(target);
    hoverAnimation_->start();
}

void LiquidNavButton::animateSelection(qreal target) {
    selectionAnimation_->stop();
    selectionAnimation_->setStartValue(selectionProgress_);
    selectionAnimation_->setEndValue(target);
    selectionAnimation_->start();
}

void LiquidNavButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const qreal active = qMax(selectionProgress_, property("active").toBool() ? 1.0 : 0.0);
    const qreal energy = qBound<qreal>(0.0, hoverProgress_ * 0.62 + active, 1.0);
    const QRectF bounds = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
    const qreal radius = 15.0;
    QPainterPath shape;
    shape.addRoundedRect(bounds, radius, radius);

    // 中文注释：三段渐变模拟玻璃厚度，选中时提高蓝色折射但保持背景可见。
    QLinearGradient fill(bounds.topLeft(), bounds.bottomRight());
    fill.setColorAt(0.0, QColor(218, 241, 255, 18 + static_cast<int>(energy * 28)));
    fill.setColorAt(0.38, QColor(72, 139, 230, 18 + static_cast<int>(energy * 58)));
    fill.setColorAt(1.0, QColor(9, 25, 57, 42 + static_cast<int>(active * 32)));
    painter.fillPath(shape, fill);

    painter.save();
    painter.setClipPath(shape);
    QLinearGradient sheen(bounds.topLeft(), QPointF(bounds.right(), bounds.top() + bounds.height() * 0.72));
    sheen.setColorAt(0.0, QColor(255, 255, 255, 30 + static_cast<int>(energy * 32)));
    sheen.setColorAt(0.22, QColor(181, 226, 255, 9 + static_cast<int>(energy * 20)));
    sheen.setColorAt(0.52, QColor(105, 174, 255, 0));
    painter.fillRect(QRectF(bounds.left(), bounds.top(), bounds.width(), bounds.height() * 0.48), sheen);

    if (energy > 0.02) {
        QRadialGradient glow(QPointF(bounds.left() + bounds.width() * 0.28, bounds.center().y()),
                             bounds.width() * 0.62);
        glow.setColorAt(0.0, QColor(83, 178, 255, static_cast<int>(energy * 54)));
        glow.setColorAt(1.0, QColor(50, 130, 255, 0));
        painter.fillRect(bounds, glow);
    }
    painter.restore();

    painter.setPen(QPen(QColor(210, 239, 255, 42 + static_cast<int>(energy * 104)),
                        active > 0.5 ? 1.35 : 1.0));
    painter.drawPath(shape);

    QPainterPath inner;
    inner.addRoundedRect(bounds.adjusted(1.4, 1.4, -1.4, -1.4), radius - 1.5, radius - 1.5);
    painter.setPen(QPen(QColor(255, 255, 255, 14 + static_cast<int>(energy * 34)), 0.8));
    painter.drawPath(inner);

    if (active > 0.01) {
        const QRectF indicator(bounds.left() + 5.0, bounds.center().y() - 12.0, 3.5, 24.0);
        QLinearGradient indicatorGlow(indicator.topLeft(), indicator.bottomLeft());
        indicatorGlow.setColorAt(0.0, QColor(191, 238, 255, 50));
        indicatorGlow.setColorAt(0.5, QColor(105, 201, 255, 245));
        indicatorGlow.setColorAt(1.0, QColor(87, 156, 255, 60));
        painter.setPen(Qt::NoPen);
        painter.setBrush(indicatorGlow);
        painter.drawRoundedRect(indicator, 2.0, 2.0);
    }

    QFont labelFont = font();
    labelFont.setWeight(active > 0.45 ? QFont::DemiBold : QFont::Medium);
    painter.setFont(labelFont);
    painter.setPen(QColor(226 + static_cast<int>(energy * 29),
                          238 + static_cast<int>(energy * 17), 255));
    painter.drawText(bounds.adjusted(18.0 + active * 3.0, 0.0, -12.0, 0.0),
                     Qt::AlignVCenter | Qt::AlignLeft, text());
}

TransparentStackedWidget::TransparentStackedWidget(QWidget* parent)
    : QStackedWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_StaticContents, false);
    setAutoFillBackground(false);
}

void TransparentStackedWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    // 中文注释：页面栈本身不绘制静态底图，直接透出父级实时流星和粒子动画。
}

BackgroundWidget::BackgroundWidget(QWidget* parent)
    : QWidget(parent), wallpaper_(":/assets/liquid_space_wallpaper.png") {
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    qApp->installEventFilter(this);

    // 四角星使用归一化坐标，窗口缩放后仍能均匀分布。
    for (int i = 0; i < 46; ++i) {
        TwinkleStar star;
        star.normalizedPosition = QPointF(randomBetween(0.02, 0.98), randomBetween(0.03, 0.97));
        star.phase = randomBetween(0.0, 6.283185307);
        star.speed = randomBetween(0.035, 0.095);
        star.size = randomBetween(1.4, 4.2);
        star.brightness = randomBetween(0.35, 1.0);
        stars_.push_back(star);
    }

    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(40);
    connect(animationTimer_, &QTimer::timeout, this, [this]() {
        advanceAnimation();
    });
    animationTimer_->start();
}

bool BackgroundWidget::eventFilter(QObject* watched, QEvent* event) {
    Q_UNUSED(watched)
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint local = mapFromGlobal(mouseEvent->globalPosition().toPoint());
        if (rect().contains(local)) {
            createClickParticles(local);
        }
    }
    return false;
}

void BackgroundWidget::advanceAnimation() {
    phase_ += 0.008;
    if (phase_ > 6.283185307) {
        phase_ = 0.0;
    }

    if (--meteorCooldown_ <= 0 && meteors_.size() < 20 && width() > 0) {
        Meteor meteor;
        meteor.position = QPointF(QRandomGenerator::global()->bounded(qMax(1, width())),
                                  -QRandomGenerator::global()->bounded(40, 180));
        meteor.speed = randomBetween(8.0, 14.5);
        meteor.length = randomBetween(90.0, 185.0);
        meteor.opacity = randomBetween(0.42, 0.82);
        meteors_.push_back(meteor);
        meteorCooldown_ = QRandomGenerator::global()->bounded(3, 10);
    }

    for (Meteor& meteor : meteors_) {
        meteor.position += QPointF(-meteor.speed * 0.38, meteor.speed);
    }
    meteors_.erase(std::remove_if(meteors_.begin(), meteors_.end(), [this](const Meteor& meteor) {
        return meteor.position.y() - meteor.length > height() || meteor.position.x() < -meteor.length;
    }), meteors_.end());

    for (Particle& particle : particles_) {
        particle.position += particle.velocity;
        particle.velocity *= 0.94;
        particle.life -= 0.072;
    }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](const Particle& particle) {
        return particle.life <= 0.0;
    }), particles_.end());
    for (TwinkleStar& star : stars_) {
        star.phase += star.speed;
        if (star.phase > 6.283185307) {
            star.phase -= 6.283185307;
        }
    }
    update();
}

void BackgroundWidget::createClickParticles(const QPointF& position) {
    for (int i = 0; i < 34; ++i) {
        const qreal angle = (6.283185307 * i / 34.0) + randomBetween(-0.18, 0.18);
        const qreal speed = randomBetween(6.2, 12.5);
        Particle particle;
        particle.position = position;
        particle.velocity = QPointF(qCos(angle) * speed, qSin(angle) * speed);
        particle.life = randomBetween(0.68, 0.90);
        particle.size = randomBetween(2.8, 6.8);
        particles_.push_back(particle);
    }
}

void BackgroundWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(3, 8, 22));

    if (!wallpaper_.isNull()) {
        const QPixmap scaled = wallpaper_.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const QPoint origin((width() - scaled.width()) / 2, (height() - scaled.height()) / 2);
        painter.drawPixmap(origin, scaled);
    }

    painter.fillRect(rect(), QColor(1, 5, 18, 14));

    const QPointF firstCenter(width() * (0.72 + 0.035 * qSin(phase_)),
                              height() * (0.18 + 0.025 * qCos(phase_ * 0.8)));
    QRadialGradient firstGlow(firstCenter, qMax(width(), height()) * 0.48);
    firstGlow.setColorAt(0.0, QColor(92, 138, 255, 52));
    firstGlow.setColorAt(0.46, QColor(57, 104, 255, 20));
    firstGlow.setColorAt(1.0, QColor(14, 36, 92, 0));
    painter.fillRect(rect(), firstGlow);

    const QPointF secondCenter(width() * (0.24 + 0.025 * qCos(phase_ * 0.7)),
                               height() * (0.72 + 0.03 * qSin(phase_ * 0.6)));
    QRadialGradient secondGlow(secondCenter, qMax(width(), height()) * 0.40);
    secondGlow.setColorAt(0.0, QColor(46, 210, 220, 30));
    secondGlow.setColorAt(1.0, QColor(8, 48, 77, 0));
    painter.fillRect(rect(), secondGlow);

    for (const Meteor& meteor : meteors_) {
        const QPointF tail = meteor.position + QPointF(meteor.length * 0.38, -meteor.length);
        QLinearGradient trail(tail, meteor.position);
        trail.setColorAt(0.0, QColor(142, 205, 255, 0));
        trail.setColorAt(0.72, QColor(166, 220, 255, static_cast<int>(meteor.opacity * 105)));
        trail.setColorAt(1.0, QColor(240, 250, 255, static_cast<int>(meteor.opacity * 255)));
        painter.setPen(QPen(QBrush(trail), 2.2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(tail, meteor.position);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(235, 249, 255, static_cast<int>(meteor.opacity * 220)));
        painter.drawEllipse(meteor.position, 3.0, 3.0);
    }

    for (const TwinkleStar& star : stars_) {
        const qreal pulse = qPow(qMax<qreal>(0.0, qSin(star.phase)), 5.0);
        if (pulse < 0.025) {
            continue;
        }
        const QPointF center(star.normalizedPosition.x() * width(), star.normalizedPosition.y() * height());
        const qreal radius = star.size * (0.68 + pulse * 1.35);
        const int alpha = static_cast<int>((28.0 + pulse * 210.0) * star.brightness);

        QPainterPath sparkle;
        sparkle.moveTo(center.x(), center.y() - radius * 2.8);
        sparkle.lineTo(center.x() + radius * 0.34, center.y() - radius * 0.34);
        sparkle.lineTo(center.x() + radius * 2.8, center.y());
        sparkle.lineTo(center.x() + radius * 0.34, center.y() + radius * 0.34);
        sparkle.lineTo(center.x(), center.y() + radius * 2.8);
        sparkle.lineTo(center.x() - radius * 0.34, center.y() + radius * 0.34);
        sparkle.lineTo(center.x() - radius * 2.8, center.y());
        sparkle.lineTo(center.x() - radius * 0.34, center.y() - radius * 0.34);
        sparkle.closeSubpath();

        QRadialGradient starGlow(center, radius * 4.2);
        starGlow.setColorAt(0.0, QColor(241, 251, 255, alpha));
        starGlow.setColorAt(0.30, QColor(156, 218, 255, alpha / 2));
        starGlow.setColorAt(1.0, QColor(91, 151, 255, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(starGlow);
        painter.drawEllipse(center, radius * 4.2, radius * 4.2);
        painter.setBrush(QColor(242, 252, 255, qMin(245, alpha + 25)));
        painter.drawPath(sparkle);
    }

    for (const Particle& particle : particles_) {
        const int alpha = qMin(235, static_cast<int>(particle.life * 215));
        QRadialGradient glow(particle.position, particle.size * 3.8);
        glow.setColorAt(0.0, QColor(230, 249, 255, alpha));
        glow.setColorAt(0.45, QColor(95, 188, 255, alpha / 2));
        glow.setColorAt(1.0, QColor(70, 135, 255, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(particle.position, particle.size * 3.8, particle.size * 3.8);
        painter.setBrush(QColor(222, 247, 255, qMin(245, alpha + 30)));
        painter.drawEllipse(particle.position, qMax<qreal>(1.2, particle.size * 0.42),
                            qMax<qreal>(1.2, particle.size * 0.42));
    }
}

GlassSurface::GlassSurface(Tone tone, QWidget* parent)
    : QWidget(parent), tone_(tone) {
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);

    // 中文注释：普通页面容器包含 QStackedWidget，图形效果会缓存旧页面并造成透明切页残影。
    if (tone != Tone::Regular) {
        auto* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(tone == Tone::Elevated ? 34.0 : 26.0);
        shadow->setOffset(0, tone == Tone::Elevated ? 10 : 7);
        shadow->setColor(QColor(0, 3, 14, tone == Tone::Sidebar ? 115 : 145));
        setGraphicsEffect(shadow);
    }
}

void GlassSurface::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const qreal radius = tone_ == Tone::Sidebar ? 22.0 : 24.0;
    const QRectF bounds = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
    QPainterPath shape;
    shape.addRoundedRect(bounds, radius, radius);

    QLinearGradient glass(bounds.topLeft(), bounds.bottomLeft());
    if (tone_ == Tone::Sidebar) {
        glass.setColorAt(0.0, QColor(20, 38, 73, 158));
        glass.setColorAt(0.55, QColor(9, 24, 52, 142));
        glass.setColorAt(1.0, QColor(5, 16, 38, 157));
    } else if (tone_ == Tone::Elevated) {
        glass.setColorAt(0.0, QColor(35, 62, 108, 122));
        glass.setColorAt(0.45, QColor(15, 34, 70, 102));
        glass.setColorAt(1.0, QColor(7, 20, 47, 121));
    } else {
        glass.setColorAt(0.0, QColor(31, 57, 101, 112));
        glass.setColorAt(0.50, QColor(13, 32, 67, 88));
        glass.setColorAt(1.0, QColor(6, 20, 47, 112));
    }
    painter.fillPath(shape, glass);

    painter.save();
    painter.setClipPath(shape);

    // 中文注释：固定网格上的微弱颗粒模拟磨砂漫反射，不引入随机闪烁。
    painter.setPen(Qt::NoPen);
    for (int y = 10; y < height(); y += 18) {
        for (int x = 9 + ((y / 18) % 2) * 7; x < width(); x += 23) {
            const int hash = (x * 17 + y * 31) % 19;
            painter.setBrush(QColor(225, 244, 255, 4 + hash / 4));
            painter.drawEllipse(QPointF(x, y), 0.55, 0.55);
        }
    }

    // 中文注释：整块面板使用统一的轻微斜向折射，不再单独突出顶部区域。
    QLinearGradient refraction(bounds.topLeft(), bounds.bottomRight());
    refraction.setColorAt(0.0, QColor(205, 235, 255, 10));
    refraction.setColorAt(0.42, QColor(110, 180, 255, 3));
    refraction.setColorAt(0.68, QColor(225, 246, 255, 8));
    refraction.setColorAt(1.0, QColor(80, 145, 235, 2));
    painter.fillRect(bounds, refraction);
    painter.restore();

    painter.setPen(QPen(QColor(215, 239, 255, tone_ == Tone::Elevated ? 104 : 78), 1.0));
    painter.drawPath(shape);

    QPainterPath inner;
    inner.addRoundedRect(bounds.adjusted(1.5, 1.5, -1.5, -1.5), radius - 2.0, radius - 2.0);
    painter.setPen(QPen(QColor(255, 255, 255, 22), 1.0));
    painter.drawPath(inner);
}
