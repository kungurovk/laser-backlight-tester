#include "dockmanager.h"
#include "controller.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Laser Backlight Tester");
    DockManager w;
    Controller c;
    QObject::connect(&w, &DockManager::modeRequested, &c, &Controller::sendMessageForMode);
    w.show();
    return a.exec();
}
