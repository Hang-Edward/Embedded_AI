#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Embedded AI Reality Bridge");
    QApplication::setOrganizationName("EmbeddedAI");

    MainWindow window;
    window.show();
    return QApplication::exec();
}
