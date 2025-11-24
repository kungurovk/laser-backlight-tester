#include "sensorstableform.h"
#include "ui_sensorstableform.h"

#include "modbusclient.h"
#include "enums.h"

namespace {
QString makeAddressString(int address)
{
    return QStringLiteral("0x%1").arg(address, 0, 16).toUpper();
}
}

SensorsTableForm::SensorsTableForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SensorsTableForm)
{
    ui->setupUi(this);
    setupTable();
    populateTable();
    connect(ui->sensorsTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, [this](int logicalIndex, int /*oldSize*/, int /*newSize*/) {
                Q_UNUSED(logicalIndex);
                ui->sensorsTableWidget->resizeRowsToContents();
                ui->sensorsTableWidget->viewport()->update();
            }, Qt::DirectConnection);
}

SensorsTableForm::~SensorsTableForm()
{
    delete ui;
}

void SensorsTableForm::setModbusClient(ModbusClient *client)
{
    if (m_modbusClient == client) {
        return;
    }

    if (m_modbusClient) {
        disconnect(m_modbusClient, &ModbusClient::readCompleted,
                   this, &SensorsTableForm::handleReadCompleted);
    }

    m_modbusClient = client;

    if (m_modbusClient) {
        connect(m_modbusClient, &ModbusClient::readCompleted,
                this, &SensorsTableForm::handleReadCompleted);
        requestAllValues();
    }
}

void SensorsTableForm::handleReadCompleted(int startAddress, const QVector<quint16> &values)
{
    for (int i = 0; i < values.size(); ++i) {
        const int address = startAddress + i;
        const auto rowIt = m_addressToRow.constFind(address);
        if (rowIt == m_addressToRow.constEnd()) {
            continue;
        }

        const int row = rowIt.value();
        QTableWidgetItem *valueItem = ui->sensorsTableWidget->item(row, 2);
        if (!valueItem) {
            valueItem = new QTableWidgetItem;
            ui->sensorsTableWidget->setItem(row, 2, valueItem);
        }

        const quint16 value = values.at(i);
        valueItem->setText(QString::number(value));
        valueItem->setData(Qt::UserRole, QVariant::fromValue(value));
    }
}

void SensorsTableForm::setupTable()
{
    ui->sensorsTableWidget->setColumnCount(3);
    ui->sensorsTableWidget->setHorizontalHeaderLabels(
        {tr("Адрес"), tr("Название"), tr("Значение")});
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->sensorsTableWidget->verticalHeader()->setVisible(false);
    ui->sensorsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->sensorsTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->sensorsTableWidget->setFocusPolicy(Qt::NoFocus);
}

void SensorsTableForm::populateTable()
{
    m_entries = {
        { SensorsTableAddress::BoardOperatingMode,      tr("Режим работы платы управления лазером") },
        { SensorsTableAddress::LaserOperatingMode,      tr("Режим работы лазера") },
        { SensorsTableAddress::CaseTemperature_1,       tr("Температура корпуса #1") },
        { SensorsTableAddress::CaseTemperature_2,       tr("Температура корпуса #2") },
        { SensorsTableAddress::CoolantTemperature_1,    tr("Температура охладителя #1") },
        { SensorsTableAddress::CoolantTemperature_2,    tr("Температура охладителя #2") },
        { SensorsTableAddress::CoolantFlowRate_1,       tr("Расход охладителя #1") },
        { SensorsTableAddress::CoolantFlowRate_2,       tr("Расход охладителя #2") },
        { SensorsTableAddress::CoolantFlowRate_3,       tr("Расход охладителя #3") },
        { SensorsTableAddress::AirHumidity_1,           tr("Влажность воздуха #1") },
        { SensorsTableAddress::AitTemperature_1,        tr("Температура воздуха #1") },
        { SensorsTableAddress::AirHumidity_2,           tr("Влажность воздуха #2") },
        { SensorsTableAddress::AitTemperature_2,        tr("Температура воздуха #2") },
        { SensorsTableAddress::LaserPower,              tr("Мощность лазера") },
        { SensorsTableAddress::CrystalTemperature_1,    tr("Температура кристалла LBO #1") },
        { SensorsTableAddress::CrystalTemperature_2,    tr("Температура кристалла LBO #2") },
        { SensorsTableAddress::LaserWorkTime,           tr("Время наработки лазера (кол-во импульсов)") }
    };

    for (const auto &entry : m_entries) {
        insertRow(entry);
    }

    ui->sensorsTableWidget->resizeColumnsToContents();
}

void SensorsTableForm::insertRow(const BlockEntry &entry)
{
    const int row = ui->sensorsTableWidget->rowCount();
    ui->sensorsTableWidget->insertRow(row);

    auto *addressItem = new QTableWidgetItem(makeAddressString(entry.address));
    addressItem->setData(Qt::UserRole, entry.address);
    addressItem->setTextAlignment(Qt::AlignCenter);
    ui->sensorsTableWidget->setItem(row, 0, addressItem);

    auto *nameItem = new QTableWidgetItem(entry.name);
    nameItem->setTextAlignment(Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap);
    ui->sensorsTableWidget->setItem(row, 1, nameItem);

    auto *valueItem = new QTableWidgetItem(tr("Н/Д"));
    valueItem->setTextAlignment(Qt::AlignCenter);
    ui->sensorsTableWidget->setItem(row, 2, valueItem);

    m_addressToRow.insert(entry.address, row);
}

void SensorsTableForm::requestAllValues() const
{
    if (!m_modbusClient) {
        return;
    }

    for (const auto &entry : m_entries) {
        const int address = entry.address;
        QMetaObject::invokeMethod(m_modbusClient,
                                  [client = m_modbusClient, address]() {
                                      if (!client) {
                                          return;
                                      }
                                      client->readHoldingRegisters(address, 1);
                                  },
                                  Qt::QueuedConnection);
    }
}
