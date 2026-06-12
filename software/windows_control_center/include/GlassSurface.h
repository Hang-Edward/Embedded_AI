#pragma once

#include <QPixmap>
#include <QPointF>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>
#include <QWidget>

class QTimer;
class QVariantAnimation;

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

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
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

    void advanceAnimation();
    void createClickParticles(const QPointF& position);

    QPixmap wallpaper_;
    QTimer* animationTimer_ = nullptr;
    qreal phase_ = 0.0;
    int meteorCooldown_ = 0;
    QVector<Meteor> meteors_;
    QVector<Particle> particles_;
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

private:
    Tone tone_;
};
