#include "ioMainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("OscilloscopeCS565"));
    QApplication::setOrganizationName(QStringLiteral("StevensCS565"));

    ioMainWindow::MainWindow window;
    window.resize(960, 640);
    window.show();

    return application.exec();
}
