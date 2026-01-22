#include "modecontrolform.h"
#include "ui_modecontrolform.h"
#include "endianutils.h"

#include <QDebug>
#include <QtEndian>
#include <QMap>

ModeControlForm::ModeControlForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModeControlForm)
{
    ui->setupUi(this);
    ui->radioButtonAuto->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->radioButtonManual->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->radioButtonDuty->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->radioButtonPrepare->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->radioButtonWork->setAttribute(Qt::WA_TransparentForMouseEvents);

    const auto wireModeButton = [this](QPushButton *button, ModeAddress address) {
        connect(button, &QPushButton::clicked, this, [this, address] {
            QMap<int, bool> states;
            states[address] = true;  // Set current address to true
            switch (address) {
            case ModeAddress::AutoAddress:
                states[ModeAddress::ManualAddress] = false;
                break;
            case ModeAddress::ManualAddress:
                states[ModeAddress::AutoAddress] = false;
                break;
            case ModeAddress::DutyAddress:
                states[ModeAddress::PrepareAddress] = false;
                states[ModeAddress::WorkAddress] = false;
                break;
            case ModeAddress::PrepareAddress:
                states[ModeAddress::DutyAddress] = false;
                states[ModeAddress::WorkAddress] = false;
                break;
            case ModeAddress::WorkAddress:
                states[ModeAddress::DutyAddress] = false;
                states[ModeAddress::PrepareAddress] = false;
                break;
            default:
                break;
            }
            sendState(states);
        });
    };

    wireModeButton(ui->pushButtonAutoMode, ModeAddress::AutoAddress);
    wireModeButton(ui->pushButtonManualMode, ModeAddress::ManualAddress);
    wireModeButton(ui->pushButtonDutyMode, ModeAddress::DutyAddress);
    wireModeButton(ui->pushButtonPrepareMode, ModeAddress::PrepareAddress);
    wireModeButton(ui->pushButtonWorkMode, ModeAddress::WorkAddress);
}

ModeControlForm::~ModeControlForm()
{
    delete ui;
}

void ModeControlForm::setModbusClient(ModbusClient *client)
{
    if (m_modbusClient == client) {
        return;
    }

    if (m_modbusClient) {
        disconnect(m_modbusClient, &ModbusClient::readCompleted,
                   this, &ModeControlForm::handleReadCompleted);
        disconnect(m_modbusClient, &ModbusClient::writeCompleted,
                   this, &ModeControlForm::handleWriteCompleted);
    }

    m_modbusClient = client;

    if (m_modbusClient) {
        connect(m_modbusClient, &ModbusClient::readCompleted,
                this, &ModeControlForm::handleReadCompleted);
        connect(m_modbusClient, &ModbusClient::writeCompleted,
                this, &ModeControlForm::handleWriteCompleted);
        // requestAllValues();
    }
}

void ModeControlForm::handleWriteCompleted(int startAddress, quint16 numberOfEntries)
{
    Q_UNUSED(numberOfEntries);
    // Check if this write was for one of our addresses
    if (startAddress >= ModeAddress::ManualAddress &&
        startAddress <= ModeAddress::WorkAddress)
    {
        // Refresh all values after a successful write
        requestAllValues();
    }
}

void ModeControlForm::handleReadCompleted(int startAddress, const QVector<quint16> &values)
{
    if (startAddress == SensorsTableAddress::BoardOperatingMode) //test ModeAddress::ManualAddress
    {
        qDebug() << "Received" << values.size() << "registers starting from" << startAddress;
        int currentAddress = startAddress;
        int valueIndex = 0;

        while (valueIndex < values.size()) {
            quint16 value;
            value = toBigEndian(values[valueIndex]);
            // qDebug() << "value" << value;

            if (currentAddress == SensorsTableAddress::BoardOperatingMode)
            {
                switch (value) {
                case Mode::Manual:
                    ui->radioButtonManual->click();
                    break;
                case Mode::Auto:
                    ui->radioButtonAuto->click();
                    break;
                default:
                    break;
                }
            }
            else if (currentAddress == SensorsTableAddress::LaserOperatingMode)
            {
                switch (value) {
                case Mode::Duty:
                    ui->radioButtonDuty->click();
                    break;
                case Mode::Prepare:
                    ui->radioButtonPrepare->click();
                    break;
                case Mode::Work:
                    ui->radioButtonWork->click();
                    break;
                default:
                    break;
                }
            }
            valueIndex++;
            currentAddress++;
        }
    }
}

void ModeControlForm::sendState(int address, bool value)
{
    if (!m_modbusClient) {
        return;
    }

    QMetaObject::invokeMethod(m_modbusClient,
                              [client = m_modbusClient, address, value]() {
                                  if (!client) {
                                      return;
                                  }
                                  client->writeSingleRegister(address,
                                                              value ? toLittleEndian(quint16(States::On)) :
                                                                  toLittleEndian(quint16(States::Off)));
                              },
                              Qt::QueuedConnection);
}

void ModeControlForm::sendState(const QMap<int, bool> &states)
{
    if (!m_modbusClient || states.isEmpty()) {
        return;
    }

    // Find min and max addresses for the range
    auto addresses = states.keys();
    std::sort(addresses.begin(), addresses.end());
    int startAddress = addresses.first();
    int endAddress = addresses.last();

    // Create vector with values for the entire range
    QVector<quint16> values(endAddress - startAddress + 1, toLittleEndian(quint16(States::Off)));

    // Set the specified states
    for (auto it = states.begin(); it != states.end(); ++it) {
        int index = it.key() - startAddress;
        values[index] = it.value() ? toLittleEndian(quint16(States::On)) : toLittleEndian(quint16(States::Off));
    }

    QMetaObject::invokeMethod(m_modbusClient,
                              [client = m_modbusClient, startAddress, values]() {
                                  if (!client) {
                                      return;
                                  }
                                  client->writeMultipleRegisters(startAddress, values);
                              },
                              Qt::QueuedConnection);
}

void ModeControlForm::requestAllValues() const
{
    if (!m_modbusClient) {
        return;
    }

    // Read all registers in one call
    const int startAddress = SensorsTableAddress::BoardOperatingMode;
    const int registerCount = SensorsTableAddress::CaseTemperature_1 - startAddress;

    //test
    // const int startAddress = ModeAddress::ManualAddress;
    // const int registerCount = 1;

    QMetaObject::invokeMethod(m_modbusClient,
                              [client = m_modbusClient, startAddress, registerCount]() {
                                  if (!client) {
                                      return;
                                  }
                                  client->readHoldingRegisters(startAddress, registerCount);
                              },
                              Qt::QueuedConnection);
}

void ModeControlForm::on_pushButton_clicked()
{
    requestAllValues();
}

