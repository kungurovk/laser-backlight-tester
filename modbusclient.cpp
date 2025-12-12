#include "modbusclient.h"

#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusReply>
#include <QtSerialBus/QModbusDevice>
#include <QtSerialBus/QModbusTcpClient>
#include <QLoggingCategory>
#include <QVariant>

#include <QTimer>

namespace {
QLoggingCategory lcModbusClient("modbus.client");
}

ModbusClient::ModbusClient(QObject *parent)
    : QObject(parent)
    , m_client(new QModbusTcpClient(this))
    , m_dispatchTimer(new QTimer(this))
    , m_replyTimeout(new QTimer(this))
{
    connect(m_client, &QModbusTcpClient::stateChanged, this, [this](QModbusDevice::State state) {
        if (state == QModbusDevice::ConnectedState) {
            emit connectionStateChanged(true);
            startDispatchTimerIfNeeded();
        } else if (state == QModbusDevice::UnconnectedState) {
            stopDispatching();
            emit connectionStateChanged(false);
        }
    });

    connect(m_client, &QModbusTcpClient::errorOccurred, this, [this](QModbusDevice::Error error) {
        if (error == QModbusDevice::NoError) {
            return;
        }
        emit errorOccurred(tr("Modbus client error: %1").arg(m_client->errorString()));
    });

    m_dispatchTimer->setInterval(100);
    m_dispatchTimer->setSingleShot(false);
    connect(m_dispatchTimer, &QTimer::timeout, this, [this]() {
        dispatchQueuedMessages();
    });

    m_replyTimeout->setSingleShot(true);
    connect(m_replyTimeout, &QTimer::timeout, this, [this]() {
        if (!m_activeReply) {
            return;
        }
        handleError(tr("Modbus request timeout"), m_activeReply);
        m_activeReply->deleteLater();
        m_activeReply.clear();
        onReplySettled();
    });
}

ModbusClient::~ModbusClient() = default;

void ModbusClient::setConnectionParameters(const QString &host, quint16 port, int timeoutMs)
{
    m_host = host;
    m_port = port;
    m_timeoutMs = timeoutMs;

    if (!m_client) {
        return;
    }

    m_client->setConnectionParameter(QModbusDevice::NetworkAddressParameter, m_host);
    m_client->setConnectionParameter(QModbusDevice::NetworkPortParameter, m_port);
    m_client->setTimeout(m_timeoutMs);
}

bool ModbusClient::connectDevice(const QString &host, quint16 port)
{
    setConnectionParameters(host, port);

    if (!m_client) {
        handleError(tr("Unable to connect: Modbus client is unavailable."));
        return false;
    }

    if (m_host.isEmpty()) {
        handleError(tr("Unable to connect: target host is not set."));
        return false;
    }

    if (m_client->state() == QModbusDevice::ConnectedState) {
        return true;
    }

    if (!m_client->connectDevice()) {
        handleError(tr("Failed to connect to %1:%2").arg(m_host).arg(m_port));
        return false;
    }

    qCDebug(lcModbusClient) << "Connecting to" << m_host << ":" << m_port;
    return true;
}

void ModbusClient::disconnectDevice()
{
    if (!m_client) {
        return;
    }

    if (m_client->state() == QModbusDevice::ConnectedState) {
        m_client->disconnectDevice();
    }
}

bool ModbusClient::isConnected() const
{
    return m_client && m_client->state() == QModbusDevice::ConnectedState;
}

void ModbusClient::readHoldingRegisters(int startAddress, quint16 numberOfEntries, int serverAddress)
{
    if (!m_client) {
        handleError(tr("Unable to read holding registers: Modbus client is unavailable."));
        return;
    }

    enqueueMessage(startAddress, numberOfEntries, {}, serverAddress, true);
    startDispatchTimerIfNeeded();
}

void ModbusClient::writeSingleRegister(int address, quint16 value, int serverAddress)
{
    qDebug() << address << value;
    writeMultipleRegisters(address, QVector<quint16>{value}, serverAddress);
}

void ModbusClient::writeMultipleRegisters(int startAddress, const QVector<quint16> &values, int serverAddress)
{
    if (!m_client) {
        handleError(tr("Unable to write registers: Modbus client is unavailable."));
        return;
    }

    enqueueMessage(startAddress, static_cast<quint16>(values.size()), values, serverAddress, false);
    startDispatchTimerIfNeeded();
}

void ModbusClient::handleReplyFinished(QModbusReply *reply, bool isReadOperation)
{
    if (!reply) {
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        if (isReadOperation) {
            const QModbusDataUnit unit = reply->result();
            QVector<quint16> values(unit.valueCount());
            for (uint i = 0; i < unit.valueCount(); ++i) {
                values[static_cast<int>(i)] = unit.value(i);
            }
            // qDebug() << unit.startAddress() << values;
            emit readCompleted(unit.startAddress(), values);
        } else {
            const QModbusDataUnit unit = reply->result();
            emit writeCompleted(unit.startAddress(), unit.valueCount());
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        handleError(tr("Modbus reply protocol error: %1 (exception code: 0x%2)")
                        .arg(reply->errorString())
                        .arg(reply->rawResult().exceptionCode(), 0, 16),
                    reply);
    } else if (reply->error() == QModbusDevice::ReplyAbortedError) {
        // Специальная обработка ошибки закрытия соединения
        QString operation = isReadOperation ? tr("read") : tr("write");
        handleError(tr("Modbus %1 operation aborted: connection was closed during request. "
                       "The device may have disconnected or the connection timed out. "
                       "Please check the connection and try again.")
                        .arg(operation),
                    reply);
    } else {
        handleError(tr("Modbus reply error: %1").arg(reply->errorString()), reply);
    }
}

void ModbusClient::handleError(const QString &context, QModbusReply *reply)
{
    QString detailedContext = context;
    if (reply) {
        detailedContext += tr(" [code=%1]").arg(reply->error());
    }

    qCWarning(lcModbusClient) << detailedContext;
    emit errorOccurred(detailedContext);
}

void ModbusClient::enqueueMessage(int address,
                                  quint16 numberOfEntries,
                                  const QVector<quint16> &values,
                                  int serverAddress,
                                  bool isRead)
{
    QueuedMessage message;
    message.isRead = isRead;
    message.numberOfEntries = numberOfEntries;
    message.values = values;
    message.serverAddress = serverAddress;

    if (!m_messageQueue.contains(address)) {
        m_messageOrder.append(address);
    }
    m_messageQueue.insert(address, message);
}

void ModbusClient::dispatchQueuedMessages()
{
    if (m_isDispatching || !isConnected() || m_messageQueue.isEmpty()) {
        return;
    }

    m_pendingDispatch = m_messageOrder;
    m_isDispatching = true;
    sendNextQueuedMessage();
}

void ModbusClient::sendNextQueuedMessage()
{
    if (!isConnected()) {
        m_isDispatching = false;
        return;
    }

    if (m_activeReply) {
        return;
    }

    if (m_pendingDispatch.isEmpty()) {
        m_messageQueue.clear();
        m_messageOrder.clear();
        m_isDispatching = false;
        return;
    }

    const int address = m_pendingDispatch.takeFirst();
    const QueuedMessage message = m_messageQueue.value(address);

    QModbusReply *reply = nullptr;
    if (message.isRead) {
        reply = sendReadRequest(address, message.numberOfEntries, message.serverAddress);
    } else {
        reply = sendWriteRequest(address, message.values, message.serverAddress);
    }

    if (!reply) {
        // Failed to send, move on to the next message to avoid blocking the queue.
        QTimer::singleShot(0, this, [this]() {
            sendNextQueuedMessage();
        });
        return;
    }

    m_activeReply = reply;

    connect(reply, &QModbusReply::finished, this, [this, reply, isRead = message.isRead]() {
        if (reply != m_activeReply) {
            return;
        }
        handleReplyFinished(reply, isRead);
        reply->deleteLater();
        m_activeReply.clear();
        onReplySettled();
    });

    startReplyTimeout();
}

QModbusReply *ModbusClient::sendReadRequest(int startAddress, quint16 numberOfEntries, int serverAddress)
{
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        return nullptr;
    }

    if (auto reply = m_client->sendReadRequest(
            QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries),
            serverAddress)) {
        return reply;
    } else {
        handleError(tr("Failed to send read request: %1").arg(m_client->errorString()));
        return nullptr;
    }
}

QModbusReply *ModbusClient::sendWriteRequest(int startAddress,
                                            const QVector<quint16> &values,
                                            int serverAddress)
{
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        return nullptr;
    }

    QModbusDataUnit dataUnit(QModbusDataUnit::HoldingRegisters, startAddress, values.size());
    for (int i = 0; i < values.size(); ++i) {
        dataUnit.setValue(i, values.at(i));
    }

    if (auto reply = m_client->sendWriteRequest(dataUnit, serverAddress)) {
        return reply;
    } else {
        handleError(tr("Failed to send write request: %1").arg(m_client->errorString()));
        return nullptr;
    }
}

void ModbusClient::startDispatchTimerIfNeeded()
{
    if (isConnected() && m_dispatchTimer && !m_dispatchTimer->isActive()) {
        m_dispatchTimer->start();
    }
}

void ModbusClient::stopDispatching()
{
    if (m_dispatchTimer) {
        m_dispatchTimer->stop();
    }
    m_isDispatching = false;
    m_pendingDispatch.clear();
}

void ModbusClient::onReplySettled()
{
    if (m_replyTimeout) {
        m_replyTimeout->stop();
    }
    QTimer::singleShot(0, this, [this]() {
        sendNextQueuedMessage();
    });
}

void ModbusClient::startReplyTimeout()
{
    if (!m_replyTimeout) {
        return;
    }
    const int timeout = m_timeoutMs > 0 ? m_timeoutMs : 1000;
    m_replyTimeout->start(timeout);
}

