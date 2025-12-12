#include "dockmanager.h"
#include "controller.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Laser Backlight Tester");
    qRegisterMetaType<QVector<quint16>>("QVector<quint16>");
    Controller c;
    DockManager w;
    QObject::connect(&w, &DockManager::modeRequested, &c, &Controller::sendMessageForMode);
    QObject::connect(&w, &DockManager::connectToTcpPort, &c, &Controller::connectToTcpPort);
    QObject::connect(&w, &DockManager::disconnectFromTcp, &c, &Controller::disconnectFromTcp);
    w.setModbusClient(c.modbusClient());
    w.show();
    return a.exec();
}
