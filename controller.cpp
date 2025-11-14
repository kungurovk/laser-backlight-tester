#include "controller.h"

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
}

Controller::~Controller()
{
    if (m_modebusClientThread) {
        m_modebusClientThread->quit();
        m_modebusClientThread->wait();
    }
}
