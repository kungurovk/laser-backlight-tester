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
    QObject::connect(&w, &DockManager::setAutoMode, &c, &Controller::sendMessageAutoMode);
    QObject::connect(&w, &DockManager::setManualMode, &c, &Controller::sendMessageManualMode);
    QObject::connect(&w, &DockManager::setDutyMode, &c, &Controller::sendMessageDutyMode);
    QObject::connect(&w, &DockManager::setPrepareMode, &c, &Controller::sendMessagePrepareMode);
    QObject::connect(&w, &DockManager::setWorkMode, &c, &Controller::sendMessageWorkMode);
    w.show();
    return a.exec();
}
