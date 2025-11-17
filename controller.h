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

public slots:
    void sendMessageAutoMode();
    void sendMessageManualMode();
    void sendMessageDutyMode();
    void sendMessagePrepareMode();
    void sendMessageWorkMode();

signals:
    void sendMessage(int address, quint16 value, int serverAddress = 1);
    void sendMessages(int startAddress, const QVector<quint16> &values, int serverAddress = 1);

    void connectToTcpPort(const QString &host, quint16 port);
    void disconnectFromTcp();

private:
    ModbusClient *m_modebusClient = nullptr;
    QThread *m_modebusClientThread = nullptr;
};

#endif // CONTROLLER_H
