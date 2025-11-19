#include "dockmanager.h"
#include "controller.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Laser Backlight Tester");
    Controller c;
    DockManager w;
    QObject::connect(&w, &DockManager::modeRequested, &c, &Controller::sendMessageForMode);
    w.setModbusClient(c.modbusClient());
    w.show();
    return a.exec();
}
