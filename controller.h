#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>

#include "modbusclient.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();

signals:
    void connectToTcpPort(const QString &host, quint16 port);
    void disconnectFromTcp();

private:
    ModbusClient *m_modebusClient = nullptr;
    QThread *m_modebusClientThread = nullptr;
};

#endif // CONTROLLER_H
