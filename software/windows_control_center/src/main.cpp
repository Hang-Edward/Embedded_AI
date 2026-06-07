#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Embedded AI Reality Bridge");
    QApplication::setOrganizationName("EmbeddedAI");
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

    MainWindow window;
    window.show();
    return QApplication::exec();
}
