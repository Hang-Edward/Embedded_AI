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
#include <QResizeEvent>
#include <QTimer>
#include <QVariantAnimation>
#include <QtMath>

#include <algorithm>

namespace {

qreal randomBetween(qreal minimum, qreal maximum) {
    return minimum + QRandomGenerator::global()->generateDouble() * (maximum - minimum);
}

void drawNavIcon(QPainter& painter, const QRectF& rect, const QString& key, const QColor& color) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(color, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    const qreal left = rect.left();
    const qreal top = rect.top();
    const qreal width = rect.width();
    const qreal height = rect.height();
    const QPointF center = rect.center();

    // 中文注释：这里用矢量线条手绘图标，避免依赖某个字符字形导致图标缺失。
    if (key == QStringLiteral("chat")) {
        QPainterPath bubble;
        bubble.addRoundedRect(QRectF(left + 4.0, top + 5.0, width - 8.0, height - 11.0), 6.0, 6.0);
        bubble.moveTo(left + 10.0, top + height - 6.0);
        bubble.lineTo(left + 12.5, top + height - 10.0);
        bubble.lineTo(left + 16.0, top + height - 7.0);
        painter.drawPath(bubble);
        painter.drawPoint(QPointF(center.x() - 5.0, center.y()));
        painter.drawPoint(QPointF(center.x(), center.y()));
        painter.drawPoint(QPointF(center.x() + 5.0, center.y()));
    } else if (key == QStringLiteral("history")) {
        QRectF arcRect(left + 5.0, top + 5.0, width - 10.0, height - 10.0);
        painter.drawArc(arcRect, 40 * 16, 250 * 16);
        QPainterPath arrow;
        arrow.moveTo(left + width - 8.0, top + 10.0);
        arrow.lineTo(left + width - 12.0, top + 9.0);
        arrow.lineTo(left + width - 10.0, top + 13.0);
        painter.drawPath(arrow);
    } else if (key == QStringLiteral("hardware")) {
        painter.drawLine(QPointF(center.x() - 5.0, top + 8.0), QPointF(center.x() - 5.0, center.y()));
        painter.drawLine(QPointF(center.x() + 5.0, top + 8.0), QPointF(center.x() + 5.0, center.y()));
        painter.drawLine(QPointF(center.x() - 8.0, center.y()), QPointF(center.x() + 8.0, center.y()));
        painter.drawLine(QPointF(center.x(), center.y()), QPointF(center.x(), top + height - 7.0));
        painter.drawArc(QRectF(center.x() - 5.0, top + height - 12.0, 10.0, 8.0), 180 * 16, 180 * 16);
    } else if (key == QStringLiteral("camera")) {
        painter.drawRoundedRect(QRectF(left + 4.5, top + 8.0, width - 9.0, height - 13.0), 4.0, 4.0);
        painter.drawEllipse(QRectF(center.x() - 4.0, center.y() - 4.0, 8.0, 8.0));
        painter.drawLine(QPointF(left + 8.0, top + 8.0), QPointF(left + 12.0, top + 5.0));
        painter.drawLine(QPointF(left + 12.0, top + 5.0), QPointF(left + 17.0, top + 5.0));
    } else if (key == QStringLiteral("logs")) {
        for (int i = 0; i < 3; ++i) {
            const qreal y = top + 8.0 + i * 6.0;
            painter.drawLine(QPointF(left + 7.0, y), QPointF(left + 11.0, y));
            painter.drawLine(QPointF(left + 14.0, y), QPointF(left + width - 6.0, y));
        }
    } else if (key == QStringLiteral("settings")) {
        painter.drawEllipse(QRectF(center.x() - 4.5, center.y() - 4.5, 9.0, 9.0));
        for (int i = 0; i < 6; ++i) {
            const qreal angle = i * 60.0;
            const qreal radians = qDegreesToRadians(angle);
            const QPointF inner(center.x() + std::cos(radians) * 6.0, center.y() + std::sin(radians) * 6.0);
            const QPointF outer(center.x() + std::cos(radians) * 9.0, center.y() + std::sin(radians) * 9.0);
            painter.drawLine(inner, outer);
        }
    } else {
        painter.drawEllipse(QRectF(left + 7.0, top + 7.0, width - 14.0, height - 14.0));
    }
    painter.restore();
}

} // namespace

LiquidNavButton::LiquidNavButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
    setObjectName("navButton");
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(86);
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

GlassCheckBox::GlassCheckBox(const QString& text, QWidget* parent)
    : QCheckBox(text, parent) {
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    setMinimumHeight(24);
    setStyleSheet(QStringLiteral("background: transparent;"));

    hoverAnimation_ = new QVariantAnimation(this);
    hoverAnimation_->setDuration(140);
    hoverAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    connect(hoverAnimation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        hoverProgress_ = value.toReal();
        update();
    });
}

QSize GlassCheckBox::sizeHint() const {
    QFont textFont = font();
    textFont.setWeight(QFont::DemiBold);
    const QFontMetrics metrics(textFont);
    const int indicatorWidth = 18;
    const int spacing = 10;
    const int horizontalPadding = 14;
    const int widthHint = indicatorWidth + spacing + metrics.horizontalAdvance(text()) + horizontalPadding;
    const int heightHint = qMax(26, metrics.height() + 8);
    return QSize(widthHint, heightHint);
}

QSize GlassCheckBox::minimumSizeHint() const {
    return sizeHint();
}

bool GlassCheckBox::event(QEvent* event) {
    switch (event->type()) {
    case QEvent::Enter:
        animateHover(1.0);
        break;
    case QEvent::Leave:
        animateHover(0.0);
        break;
    default:
        break;
    }
    return QCheckBox::event(event);
}

void GlassCheckBox::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF boxRect(0.0, (height() - 18.0) / 2.0, 18.0, 18.0);
    const qreal glow = qBound<qreal>(0.0, hoverProgress_, 1.0);
    const bool checked = isChecked();

    painter.setPen(QPen(QColor(176, 221, 255, checked ? 210 : 122), checked ? 1.45 : 1.1));
    painter.setBrush(checked
                         ? QColor(87, 154, 255, 226)
                         : QColor(18, 38, 76, 155 + static_cast<int>(glow * 18.0)));
    painter.drawRoundedRect(boxRect, 6.0, 6.0);

    if (checked) {
        painter.setPen(QPen(QColor(255, 255, 255, 245), 2.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPainterPath tick;
        tick.moveTo(boxRect.left() + 4.2, boxRect.top() + 9.4);
        tick.lineTo(boxRect.left() + 7.6, boxRect.top() + 12.7);
        tick.lineTo(boxRect.left() + 13.8, boxRect.top() + 5.9);
        painter.drawPath(tick);
    }

    painter.setPen(QColor(226, 239, 255, 234));
    QFont textFont = font();
    textFont.setWeight(QFont::DemiBold);
    painter.setFont(textFont);
    const QRectF textRect(boxRect.right() + 9.0, 0.0, width() - boxRect.right() - 6.0, height());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text());
}

void GlassCheckBox::animateHover(qreal target) {
    if (hoverAnimation_ == nullptr) {
        return;
    }
    hoverAnimation_->stop();
    hoverAnimation_->setStartValue(hoverProgress_);
    hoverAnimation_->setEndValue(target);
    hoverAnimation_->start();
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

    const QString subtitle = property("subtitle").toString();
    const QString glyph = property("glyph").toString();
    const qreal active = qMax(selectionProgress_, property("active").toBool() ? 1.0 : 0.0);
    const qreal energy = qBound<qreal>(0.0, hoverProgress_ * 0.62 + active, 1.0);
    const QRectF bounds = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
    const qreal radius = 17.0;
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

    qreal textLeft = bounds.left() + 18.0 + active * 2.5;
    if (!glyph.isEmpty()) {
        const QRectF glyphRect(bounds.left() + 14.0, bounds.center().y() - 14.0, 28.0, 28.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(220, 242, 255, 18 + static_cast<int>(energy * 42)));
        painter.drawEllipse(glyphRect);
        drawNavIcon(painter, glyphRect, property("navKey").toString(), QColor(239, 248, 255, 232));
        textLeft = glyphRect.right() + 14.0;
    }

    const QRectF textBounds = bounds.adjusted(textLeft - bounds.left(), 12.0, -16.0, -10.0);
    painter.setPen(QColor(226 + static_cast<int>(energy * 29),
                          238 + static_cast<int>(energy * 17), 255));

    QFont titleFont = font();
    titleFont.setWeight(active > 0.45 ? QFont::DemiBold : QFont::Medium);
    titleFont.setPointSizeF(titleFont.pointSizeF() + 0.1);
    painter.setFont(titleFont);

    if (subtitle.isEmpty()) {
        painter.drawText(textBounds, Qt::AlignVCenter | Qt::AlignLeft, text());
    } else {
        const QRectF titleRect(textBounds.left(), textBounds.top(), textBounds.width(), textBounds.height() * 0.38);
        painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, text());

        QFont subtitleFont = font();
        subtitleFont.setPointSizeF(qMax(8.8, subtitleFont.pointSizeF() - 1.7));
        subtitleFont.setWeight(QFont::Medium);
        painter.setFont(subtitleFont);
        painter.setPen(QColor(182 + static_cast<int>(energy * 20),
                              206 + static_cast<int>(energy * 14),
                              232 + static_cast<int>(energy * 12), 228));
        const QRectF subtitleRect(textBounds.left(), textBounds.top() + textBounds.height() * 0.36,
                                  textBounds.width(), textBounds.height() * 0.58);
        painter.drawText(subtitleRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, subtitle);
    }
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
    rebuildStarSprite();

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
    frameClock_.start();
    lastFrameMs_ = frameClock_.elapsed();
    // 中文注释：背景节拍提升到约 30 FPS，同时运动量按真实时间推进。
    animationTimer_->setInterval(33);
    connect(animationTimer_, &QTimer::timeout, this, [this]() {
        if (isVisible() && !window()->isMinimized()) {
            advanceAnimation();
        }
    });
    animationTimer_->start();
}

void BackgroundWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuildWallpaperCache();
    rebuildGlowSprites();
}

void BackgroundWidget::rebuildWallpaperCache() {
    if (wallpaper_.isNull() || size().isEmpty()) {
        scaledWallpaper_ = QPixmap();
        return;
    }
    // 中文注释：高质量壁纸缩放只在窗口尺寸改变时执行，避免动画每帧重复缩放大图。
    scaledWallpaper_ = wallpaper_.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation);
}

void BackgroundWidget::rebuildStarSprite() {
    constexpr int side = 96;
    starSprite_ = QPixmap(side, side);
    starSprite_.fill(Qt::transparent);

    QPainter painter(&starSprite_);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QPointF center(side / 2.0, side / 2.0);

    QRadialGradient glow(center, side * 0.46);
    glow.setColorAt(0.0, QColor(246, 253, 255, 220));
    glow.setColorAt(0.24, QColor(167, 224, 255, 120));
    glow.setColorAt(1.0, QColor(96, 158, 255, 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(glow);
    painter.drawEllipse(center, side * 0.46, side * 0.46);

    const qreal radius = 9.2;
    QPainterPath sparkle;
    sparkle.moveTo(center.x(), center.y() - radius * 3.0);
    sparkle.lineTo(center.x() + radius * 0.36, center.y() - radius * 0.36);
    sparkle.lineTo(center.x() + radius * 3.0, center.y());
    sparkle.lineTo(center.x() + radius * 0.36, center.y() + radius * 0.36);
    sparkle.lineTo(center.x(), center.y() + radius * 3.0);
    sparkle.lineTo(center.x() - radius * 0.36, center.y() + radius * 0.36);
    sparkle.lineTo(center.x() - radius * 3.0, center.y());
    sparkle.lineTo(center.x() - radius * 0.36, center.y() - radius * 0.36);
    sparkle.closeSubpath();
    painter.setBrush(QColor(246, 253, 255, 238));
    painter.drawPath(sparkle);
}

void BackgroundWidget::rebuildGlowSprites() {
    if (size().isEmpty()) {
        firstGlowSprite_ = QPixmap();
        secondGlowSprite_ = QPixmap();
        return;
    }

    const int firstSide = qMax(width(), height());
    firstGlowSprite_ = QPixmap(firstSide, firstSide);
    firstGlowSprite_.fill(Qt::transparent);
    {
        QPainter painter(&firstGlowSprite_);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QPointF center(firstSide / 2.0, firstSide / 2.0);
        QRadialGradient glow(center, firstSide * 0.48);
        glow.setColorAt(0.0, QColor(92, 138, 255, 52));
        glow.setColorAt(0.46, QColor(57, 104, 255, 20));
        glow.setColorAt(1.0, QColor(14, 36, 92, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(center, firstSide * 0.48, firstSide * 0.48);
    }

    const int secondSide = static_cast<int>(qMax(width(), height()) * 0.82);
    secondGlowSprite_ = QPixmap(secondSide, secondSide);
    secondGlowSprite_.fill(Qt::transparent);
    {
        QPainter painter(&secondGlowSprite_);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QPointF center(secondSide / 2.0, secondSide / 2.0);
        QRadialGradient glow(center, secondSide * 0.40);
        glow.setColorAt(0.0, QColor(46, 210, 220, 30));
        glow.setColorAt(1.0, QColor(8, 48, 77, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(center, secondSide * 0.40, secondSide * 0.40);
    }
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
    const qint64 nowMs = frameClock_.elapsed();
    const qint64 rawDeltaMs = nowMs - lastFrameMs_;
    lastFrameMs_ = nowMs;
    const qreal deltaMs = qBound<qreal>(16.0, static_cast<qreal>(rawDeltaMs), 45.0);
    const qreal step = deltaMs / 40.0;

    phase_ += 0.008 * step;
    if (phase_ > 6.283185307) {
        phase_ = 0.0;
    }

    meteorCooldown_ -= step;
    if (meteorCooldown_ <= 0.0 && meteors_.size() < 20 && width() > 0) {
        Meteor meteor;
        meteor.position = QPointF(QRandomGenerator::global()->bounded(qMax(1, width())),
                                  -QRandomGenerator::global()->bounded(40, 180));
        meteor.speed = randomBetween(8.0, 14.5);
        meteor.length = randomBetween(90.0, 185.0);
        meteor.opacity = randomBetween(0.42, 0.82);
        meteors_.push_back(meteor);
        meteorCooldown_ = static_cast<qreal>(QRandomGenerator::global()->bounded(3, 10));
    }

    for (Meteor& meteor : meteors_) {
        meteor.position += QPointF(-meteor.speed * 0.38 * step, meteor.speed * step);
    }
    meteors_.erase(std::remove_if(meteors_.begin(), meteors_.end(), [this](const Meteor& meteor) {
        return meteor.position.y() - meteor.length > height() || meteor.position.x() < -meteor.length;
    }), meteors_.end());

    for (Particle& particle : particles_) {
        particle.position += particle.velocity * step;
        particle.velocity *= qPow(0.92, step);
        particle.life -= 0.096 * step;
    }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](const Particle& particle) {
        return particle.life <= 0.0;
    }), particles_.end());
    for (TwinkleStar& star : stars_) {
        star.phase += star.speed * step;
        if (star.phase > 6.283185307) {
            star.phase -= 6.283185307;
        }
    }
    update();
}

void BackgroundWidget::createClickParticles(const QPointF& position) {
    for (int i = 0; i < 34; ++i) {
        const qreal angle = (6.283185307 * i / 34.0) + randomBetween(-0.18, 0.18);
        const qreal speed = randomBetween(8.2, 15.1);
        Particle particle;
        particle.position = position;
        particle.velocity = QPointF(qCos(angle) * speed, qSin(angle) * speed);
        particle.life = randomBetween(0.58, 0.80);
        particle.size = randomBetween(2.4, 5.6);
        particles_.push_back(particle);
    }
}

void BackgroundWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), QColor(3, 8, 22));

    if (!scaledWallpaper_.isNull()) {
        const QPoint origin((width() - scaledWallpaper_.width()) / 2,
                            (height() - scaledWallpaper_.height()) / 2);
        painter.drawPixmap(origin, scaledWallpaper_);
    }

    painter.fillRect(rect(), QColor(1, 5, 18, 14));

    const QPointF firstCenter(width() * (0.72 + 0.035 * qSin(phase_)),
                              height() * (0.18 + 0.025 * qCos(phase_ * 0.8)));
    if (!firstGlowSprite_.isNull()) {
        painter.drawPixmap(QPointF(firstCenter.x() - firstGlowSprite_.width() / 2.0,
                                   firstCenter.y() - firstGlowSprite_.height() / 2.0),
                           firstGlowSprite_);
    }

    const QPointF secondCenter(width() * (0.24 + 0.025 * qCos(phase_ * 0.7)),
                               height() * (0.72 + 0.03 * qSin(phase_ * 0.6)));
    if (!secondGlowSprite_.isNull()) {
        painter.drawPixmap(QPointF(secondCenter.x() - secondGlowSprite_.width() / 2.0,
                                   secondCenter.y() - secondGlowSprite_.height() / 2.0),
                           secondGlowSprite_);
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
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
        const qreal wave = qMax<qreal>(0.0, qSin(star.phase));
        const qreal pulse = wave * wave * wave * wave;
        if (pulse < 0.025) {
            continue;
        }
        const QPointF center(star.normalizedPosition.x() * width(), star.normalizedPosition.y() * height());
        const qreal side = star.size * (9.0 + pulse * 11.0);
        const qreal opacity = qBound<qreal>(0.0, (0.20 + pulse * 0.78) * star.brightness, 1.0);
        painter.setOpacity(opacity);
        painter.drawPixmap(QRectF(center.x() - side / 2.0, center.y() - side / 2.0, side, side),
                           starSprite_, QRectF(starSprite_.rect()));
        painter.setOpacity(1.0);
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

void GlassSurface::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuildSurfaceCache();
}

void GlassSurface::rebuildSurfaceCache() {
    if (size().isEmpty()) {
        surfaceCache_ = QPixmap();
        return;
    }

    surfaceCache_ = QPixmap(size());
    surfaceCache_.fill(Qt::transparent);
    QPainter painter(&surfaceCache_);
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

void GlassSurface::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    if (surfaceCache_.size() != size()) {
        rebuildSurfaceCache();
    }
    QPainter painter(this);
    painter.drawPixmap(0, 0, surfaceCache_);
}
