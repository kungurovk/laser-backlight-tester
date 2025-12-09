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
{
    connect(m_client, &QModbusTcpClient::stateChanged, this, [this](QModbusDevice::State state) {
        if (state == QModbusDevice::ConnectedState) {
            emit connectionStateChanged(true);
        } else if (state == QModbusDevice::UnconnectedState) {
            emit connectionStateChanged(false);
        }
    });

    connect(m_client, &QModbusTcpClient::errorOccurred, this, [this](QModbusDevice::Error error) {
        if (error == QModbusDevice::NoError) {
            return;
        }
        emit errorOccurred(tr("Modbus client error: %1").arg(m_client->errorString()));
    });

    // test
    // QTimer::singleShot(1000, [this](){
    //     // emit readCompleted(0x11e, {0x0500, 0x0006}); //25-26 = 3
    //     emit readCompleted(0x11e, {0x0500, 0x0004}); //25-26 = 2
    //     emit readCompleted(0x120, {0x00ff, 0x0000});
    //     emit readCompleted(0x122, {0x0500});
    // });
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

    if (auto reply = m_client->sendReadRequest(QModbusDataUnit(QModbusDataUnit::HoldingRegisters,
                                                               startAddress, numberOfEntries),
                                               serverAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                handleReplyFinished(reply, true);
            });
        } else {
            handleReplyFinished(reply, true);
        }
    } else {
        handleError(tr("Failed to send read request: %1").arg(m_client->errorString()));
    }
}

void ModbusClient::writeSingleRegister(int address, quint16 value, int serverAddress)
{
    writeMultipleRegisters(address, QVector<quint16>{value}, serverAddress);
}

void ModbusClient::writeMultipleRegisters(int startAddress, const QVector<quint16> &values, int serverAddress)
{
    if (!m_client) {
        handleError(tr("Unable to write registers: Modbus client is unavailable."));
        return;
    }

    QModbusDataUnit dataUnit(QModbusDataUnit::HoldingRegisters, startAddress, values.size());
    for (int i = 0; i < values.size(); ++i) {
        dataUnit.setValue(i, values.at(i));
    }

    if (auto reply = m_client->sendWriteRequest(dataUnit, serverAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                handleReplyFinished(reply, false);
            });
        } else {
            handleReplyFinished(reply, false);
        }
    } else {
        handleError(tr("Failed to send write request: %1").arg(m_client->errorString()));
    }
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
    } else {
        handleError(tr("Modbus reply error: %1").arg(reply->errorString()), reply);
    }

    reply->deleteLater();
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

