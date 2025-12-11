#include "limitandtargetvaluesform.h"
#include "ui_limitandtargetvaluesform.h"

#include "modbusclient.h"
#include "enums.h"
#include "endianutils.h"

#include <cstring>

namespace {
QString makeAddressString(int address)
{
    return QStringLiteral("0x%1").arg(address, 0, 16).toUpper();
}
}

LimitAndTargetValuesForm::LimitAndTargetValuesForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LimitAndTargetValuesForm)
{
    ui->setupUi(this);
    setupTable();
    populateTable();
    connect(ui->limitAndTargetTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, [this](int logicalIndex, int /*oldSize*/, int /*newSize*/) {
                Q_UNUSED(logicalIndex);
                ui->limitAndTargetTableWidget->resizeRowsToContents();
                ui->limitAndTargetTableWidget->viewport()->update();
            }, Qt::DirectConnection);
}

LimitAndTargetValuesForm::~LimitAndTargetValuesForm()
{
    delete ui;
}

void LimitAndTargetValuesForm::setModbusClient(ModbusClient *client)
{
    if (m_modbusClient == client) {
        return;
    }

    if (m_modbusClient) {
        disconnect(m_modbusClient, &ModbusClient::readCompleted,
                   this, &LimitAndTargetValuesForm::handleReadCompleted);
    }

    m_modbusClient = client;

    if (m_modbusClient) {
        connect(m_modbusClient, &ModbusClient::readCompleted,
                this, &LimitAndTargetValuesForm::handleReadCompleted);
        requestAllValues();
    }
}

void LimitAndTargetValuesForm::handleReadCompleted(int startAddress, const QVector<quint16> &values)
{
    quint32 val32 = (quint32(toBigEndian(values.last())) << 16) | toBigEndian(values.first());
    float fValue = 0.f;
    std::memcpy(&fValue, &val32, sizeof(fValue));

    const auto rowIt = m_addressToRow.constFind(startAddress);
    if (rowIt == m_addressToRow.constEnd()) {
        return;
    }

    const int row = rowIt.value();
    QTableWidgetItem *valueItem = ui->limitAndTargetTableWidget->item(row, 2);
    if (!valueItem) {
        valueItem = new QTableWidgetItem;
        ui->limitAndTargetTableWidget->setItem(row, 2, valueItem);
    }

    valueItem->setText(QString::number(fValue));
    valueItem->setData(Qt::UserRole, QVariant::fromValue(fValue));
}

void LimitAndTargetValuesForm::setupTable()
{
    ui->limitAndTargetTableWidget->setColumnCount(3);
    ui->limitAndTargetTableWidget->setHorizontalHeaderLabels(
        {tr("Адрес"), tr("Название"), tr("Значение")});
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->limitAndTargetTableWidget->verticalHeader()->setVisible(false);
    ui->limitAndTargetTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->limitAndTargetTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->limitAndTargetTableWidget->setFocusPolicy(Qt::NoFocus);
}

void LimitAndTargetValuesForm::populateTable()
{
    m_entries = {
        { ValuesTableAddress::CaseTemperatureMinValue_1,    tr("Минимальное значение температуры корпуса #1") },
        { ValuesTableAddress::CaseTemperatureMaxValue_1,    tr("Максимальное значение температуры корпуса #1") },
        { ValuesTableAddress::CaseTemperatureMinValue_2,    tr("Минимальное значение температуры корпуса #2") },
        { ValuesTableAddress::CaseTemperatureMaxValue_2,    tr("Максимальное значение температуры корпуса #2") },
        { ValuesTableAddress::CoolantTemperatureMinValue_1, tr("Минимальное значение температуры охладителя №1") },
        { ValuesTableAddress::CoolantTemperatureMaxValue_1, tr("Максимальное значение температуры охладителя №1") },
        { ValuesTableAddress::CoolantTemperatureMinValue_2, tr("Минимальное значение температуры охладителя №2") },
        { ValuesTableAddress::CoolantTemperatureMaxValue_2, tr("Максимальное значение температуры охладителя №2") },
        { ValuesTableAddress::FlowRateMinValue_1,           tr("Минимальное значение расхода охладителя №1") },
        { ValuesTableAddress::FlowRateMaxValue_1,           tr("Максимальное значение расхода охладителя №1") },
        { ValuesTableAddress::FlowRateMinValue_2,           tr("Минимальное значение расхода охладителя №2") },
        { ValuesTableAddress::FlowRateMaxValue_2,           tr("Максимальное значение расхода охладителя №2") },
        { ValuesTableAddress::FlowRateMinValue_3,           tr("Минимальное значение расхода охладителя №3") },
        { ValuesTableAddress::FlowRateMaxValue_3,           tr("Максимальное значение расхода охладителя №3") },
        { ValuesTableAddress::AirHumidityMinValue_1,        tr("Минимальное значение влажности воздуха №1") },
        { ValuesTableAddress::AirHumidityMaxValue_1,        tr("Максимальное значение влажности воздуха №1") },
        { ValuesTableAddress::AirTemperatureMinValue_1,     tr("Минимальное значение температуры воздуха №1") },
        { ValuesTableAddress::AirTemperatureMaxValue_1,     tr("Максимальное значение температуры воздуха №1") },
        { ValuesTableAddress::AirHumidityMinValue_2,        tr("Минимальное значение влажности воздуха №2") },
        { ValuesTableAddress::AirHumidityMaxValue_2,        tr("Максимальное значение влажности воздуха №2") },
        { ValuesTableAddress::AirTemperatureMinValue_2,     tr("Минимальное значение температуры воздуха №2") },
        { ValuesTableAddress::AirTemperatureMaxValue_2,     tr("Максимальное значение температуры воздуха №2") },
        { ValuesTableAddress::PowerLaserMinValue,           tr("Минимальное значение мощности лазера") },
        { ValuesTableAddress::PowerLaserMaxValue,           tr("Максимальное значение мощности лазера") },
        { ValuesTableAddress::CrystalTemperatureTarget_1,   tr("Целевая температура кристалла LBO №1") },
        { ValuesTableAddress::CrystalTemperatureTarget_2,   tr("Целевая температура кристалла LBO №2") },
        { ValuesTableAddress::KP_PID_LBO,                   tr("кП ПИД регулятора нагревателя кристалла LBO") },
        { ValuesTableAddress::KI_PID_LBO,                   tr("кИ ПИД регулятора нагревателя кристалла LBO") },
        { ValuesTableAddress::KD_PID_LBO,                   tr("кД ПИД регулятора нагревателя кристалла LBO") }
    };

    for (const auto &entry : m_entries) {
        insertRow(entry);
    }

    ui->limitAndTargetTableWidget->resizeColumnsToContents();
}

void LimitAndTargetValuesForm::insertRow(const BlockEntry &entry)
{
    const int row = ui->limitAndTargetTableWidget->rowCount();
    ui->limitAndTargetTableWidget->insertRow(row);

    auto *addressItem = new QTableWidgetItem(makeAddressString(entry.address));
    addressItem->setData(Qt::UserRole, entry.address);
    addressItem->setTextAlignment(Qt::AlignCenter);
    ui->limitAndTargetTableWidget->setItem(row, 0, addressItem);

    auto *nameItem = new QTableWidgetItem(entry.name);
    nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap);
    ui->limitAndTargetTableWidget->setItem(row, 1, nameItem);

    auto *valueItem = new QTableWidgetItem(tr("Н/Д"));
    valueItem->setTextAlignment(Qt::AlignCenter);
    ui->limitAndTargetTableWidget->setItem(row, 2, valueItem);

    m_addressToRow.insert(entry.address, row);
}

void LimitAndTargetValuesForm::requestAllValues() const
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
                                      client->readHoldingRegisters(address, 2);
                                  },
                                  Qt::QueuedConnection);
    }
}
