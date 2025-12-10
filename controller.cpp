#include "controller.h"
#include "enums.h"
#include "endianutils.h"

#include <QDebug>

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    m_modebusClient = new ModbusClient;
    m_modebusClientThread = new QThread(this);
    m_modebusClient->moveToThread(m_modebusClientThread);
    // Ensure the controller lives in the worker thread and is deleted there
    m_modebusClient->connect(m_modebusClientThread, &QThread::finished, m_modebusClient, &QObject::deleteLater);
    m_modebusClientThread->start();
    connect(this, &Controller::connectToTcpPort, m_modebusClient, &ModbusClient::connectDevice, Qt::QueuedConnection);
    connect(this, &Controller::disconnectFromTcp, m_modebusClient, &ModbusClient::disconnectDevice, Qt::QueuedConnection);
    connect(m_modebusClient,  &ModbusClient::connectionStateChanged, this, [this](bool connected){
    //     if (m_tcpConnected == connected) return;
    //     m_tcpConnected = connected;
    //     emit tcpConnectedChanged(m_tcpConnected);

    //     if (connected) sendOnConnection();
    //     else doOnDisconnected();
    });
    connect(this, &Controller::sendMessage, m_modebusClient, &ModbusClient::writeSingleRegister);

    // qDebug() << toLittleEndian(Mode::Work);
    // qDebug() << toBigEndian((short)0x0400);
    emit connectToTcpPort("172.16.5.101", 502);
}

Controller::~Controller()
{
    if (m_modebusClientThread) {
        m_modebusClientThread->quit();
        m_modebusClientThread->wait();
    }
}

ModbusClient *Controller::modbusClient() const
{
    return m_modebusClient;
}

void Controller::sendMessageForMode(Mode mode)
{
    emit sendMessage(ModeAddress::Address, toLittleEndian(mode));
}
