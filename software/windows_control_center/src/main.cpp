#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QSplashScreen>
#include <QVBoxLayout>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Embedded AI Reality Bridge");
    QApplication::setOrganizationName("EmbeddedAI");
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    QApplication::setWindowIcon(QIcon(":/assets/app_icon.png"));

    QPixmap splashPixmap(520, 360);
    splashPixmap.fill(Qt::transparent);
    QSplashScreen splash(splashPixmap);
    splash.setWindowFlag(Qt::FramelessWindowHint);

    QWidget splashContent;
    splashContent.setStyleSheet(
        "QWidget { background: rgba(6, 14, 34, 228); border-radius: 28px; }"
        "QLabel#logo { background: transparent; }"
        "QLabel#title { color: white; font: 700 24px 'Microsoft YaHei UI'; background: transparent; }"
        "QLabel#sub { color: #b9d8ff; font: 13px 'Microsoft YaHei UI'; background: transparent; }");
    auto* layout = new QVBoxLayout(&splashContent);
    layout->setContentsMargins(36, 28, 36, 28);
    layout->setSpacing(12);
    auto* logo = new QLabel(&splashContent);
    logo->setObjectName("logo");
    logo->setAlignment(Qt::AlignCenter);
    logo->setPixmap(QPixmap(":/assets/app_icon.png").scaled(132, 132, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto* title = new QLabel("Embedded AI Reality Bridge", &splashContent);
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);
    auto* subtitle = new QLabel("AI 看见世界，硬件听懂你的指令", &splashContent);
    subtitle->setObjectName("sub");
    subtitle->setAlignment(Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(logo);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addStretch(1);

    splashContent.resize(splashPixmap.size());
    splashContent.render(&splashPixmap);
    splash.setPixmap(splashPixmap);
    splash.show();

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 650) {
        QApplication::processEvents();
    }

    MainWindow window;
    window.show();
    splash.finish(&window);
    return QApplication::exec();
}
