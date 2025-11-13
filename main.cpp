#include "dockmanager.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("DockManagerDemo");
    QCoreApplication::setApplicationName("DockManager");
    DockManager w;
    w.show();
    return a.exec();
}
