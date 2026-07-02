#pragma once

#include <QElapsedTimer>
#include <QCheckBox>
#include <QPixmap>
#include <QPointF>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>
#include <QWidget>

class QTimer;
class QVariantAnimation;
class QResizeEvent;

// 中文注释：自绘导航按钮提供连续的悬停与选中动画，避免普通 QSS 按钮的平面感。
class LiquidNavButton final : public QPushButton {
public:
    explicit LiquidNavButton(const QString& text, QWidget* parent = nullptr);

protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void animateHover(qreal target);
    void animateSelection(qreal target);

    QVariantAnimation* hoverAnimation_ = nullptr;
    QVariantAnimation* selectionAnimation_ = nullptr;
    qreal hoverProgress_ = 0.0;
    qreal selectionProgress_ = 0.0;
};

// 中文注释：自绘玻璃复选框，保证勾选框和对勾在所有平台下都稳定可见。
class GlassCheckBox final : public QCheckBox {
public:
    explicit GlassCheckBox(const QString& text, QWidget* parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void animateHover(qreal target);

    QVariantAnimation* hoverAnimation_ = nullptr;
    qreal hoverProgress_ = 0.0;
};

// 中文注释：切换透明页面前主动清除 backing store，避免上一页内容留下残影。
class TransparentStackedWidget final : public QStackedWidget {
public:
    explicit TransparentStackedWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

// 中文注释：负责绘制全窗口壁纸和缓慢移动的环境光，避免 QSS 背景被子控件覆盖。
class BackgroundWidget final : public QWidget {
public:
    explicit BackgroundWidget(QWidget* parent = nullptr);
    void renderSceneInto(QPainter& painter, const QRect& targetRect, const QPoint& sourceTopLeft) const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void paintScene(QPainter& painter, const QRect& targetRect, const QPointF& sourceTopLeft) const;
    struct Meteor {
        QPointF position;
        qreal speed = 0.0;
        qreal length = 0.0;
        qreal opacity = 0.0;
    };

    struct Particle {
        QPointF position;
        QPointF velocity;
        qreal life = 0.0;
        qreal size = 0.0;
    };

    struct TwinkleStar {
        QPointF normalizedPosition;
        qreal phase = 0.0;
        qreal speed = 0.0;
        qreal size = 0.0;
        qreal brightness = 0.0;
    };

    void advanceAnimation();
    void createClickParticles(const QPointF& position);
    void rebuildWallpaperCache();
    void rebuildStarSprite();
    void rebuildGlowSprites();

    QPixmap wallpaper_;
    QPixmap scaledWallpaper_;
    QPixmap starSprite_;
    QPixmap firstGlowSprite_;
    QPixmap secondGlowSprite_;
    QElapsedTimer frameClock_;
    QTimer* animationTimer_ = nullptr;
    qreal phase_ = 0.0;
    qint64 lastFrameMs_ = 0;
    qreal meteorCooldown_ = 0.0;
    QVector<Meteor> meteors_;
    QVector<Particle> particles_;
    QVector<TwinkleStar> stars_;
};

// 中文注释：应用内统一的液态玻璃表面，背景透出、边缘高光并带轻微纵向层次。
class GlassSurface final : public QWidget {
public:
    enum class Tone {
        Regular,
        Sidebar,
        Elevated
    };

    explicit GlassSurface(Tone tone = Tone::Regular, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void rebuildSurfaceCache();

    Tone tone_;
    QPixmap surfaceCache_;
};
