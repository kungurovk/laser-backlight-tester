// Copyright (c) 2025
#pragma once

#include <QObject>
#include <QHash>
#include <QList>
#include <QPointer>
#include <QString>
#include <QVector>
#include <QTimer>

class QModbusReply;
class QModbusTcpClient;
class ModbusClient;

class ModbusBase
{
public:
    virtual void setModbusClient(ModbusClient *client) = 0;

    virtual void requestAllValues() const = 0;
};

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

    bool isConnected() const;

    void readHoldingRegisters(int startAddress, quint16 numberOfEntries, int serverAddress = 1);

public slots:
    bool connectDevice(const QString &host, quint16 port);
    void disconnectDevice();

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

    void enqueueMessage(int address,
                        quint16 numberOfEntries,
                        const QVector<quint16> &values,
                        int serverAddress,
                        bool isRead);
    void dispatchQueuedMessages();
    void sendNextQueuedMessage();
    QModbusReply *sendReadRequest(int startAddress, quint16 numberOfEntries, int serverAddress);
    QModbusReply *sendWriteRequest(int startAddress, const QVector<quint16> &values, int serverAddress);
    void startDispatchTimerIfNeeded();
    void stopDispatching();
    void onReplySettled();
    void startReplyTimeout();

    struct QueuedMessage
    {
        bool isRead = true;
        quint16 numberOfEntries = 0;
        QVector<quint16> values;
        int serverAddress = 1;
    };

    QString m_host;
    quint16 m_port = 502;
    int m_timeoutMs = 1000;

    QPointer<QModbusTcpClient> m_client;

    QHash<int, QueuedMessage> m_messageQueue;
    QList<int> m_messageOrder;
    QList<int> m_pendingDispatch;
    bool m_isDispatching = false;
    QTimer *m_dispatchTimer = nullptr;
    QTimer *m_replyTimeout = nullptr;
    QPointer<QModbusReply> m_activeReply;
};

