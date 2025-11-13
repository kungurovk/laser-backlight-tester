// Copyright (c) 2025
#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

class QModbusReply;
class QModbusTcpClient;

/**
 * @brief Qt wrapper around QModbusTcpClient that exposes a high-level API for Modbus TCP communication.
 */
class ModbusClient : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ModbusClient)

public:
    explicit ModbusClient(QObject *parent = nullptr);
    ~ModbusClient() override;

    void setConnectionParameters(const QString &host, quint16 port, int timeoutMs = 1000);

    bool connectDevice();
    void disconnectDevice();

    bool isConnected() const;

    void readHoldingRegisters(int startAddress, quint16 numberOfEntries, int serverAddress = 1);
    void writeSingleRegister(int address, quint16 value, int serverAddress = 1);
    void writeMultipleRegisters(int startAddress, const QVector<quint16> &values, int serverAddress = 1);

signals:
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString &message);

    void readCompleted(int startAddress, const QVector<quint16> &values);
    void writeCompleted(int startAddress, quint16 numberOfEntries);

private:
    void handleReplyFinished(QModbusReply *reply, bool isReadOperation);
    void handleError(const QString &context, QModbusReply *reply = nullptr);

    QString m_host;
    quint16 m_port = 502;
    int m_timeoutMs = 1000;

    QPointer<QModbusTcpClient> m_client;
};

