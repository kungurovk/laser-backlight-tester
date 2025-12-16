#include "blocktableform.h"
#include "ui_blocktableform.h"

#include "enums.h"
#include "endianutils.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QBitArray>
#include <QDebug>

namespace {
QString makeAddressString(int address)
{
    return QStringLiteral("0x%1").arg(address, 0, 16).toUpper();
}
}

BlockTableForm::BlockTableForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BlockTableForm)
{
    ui->setupUi(this);
    setupTable();
    populateBlockTable();
    connect(ui->blockTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, [this](int logicalIndex, int /*oldSize*/, int /*newSize*/) {
        Q_UNUSED(logicalIndex);
        const int col = 3; // actions column with QPushButton
        const int rows = ui->blockTableWidget->rowCount();
        for (int r = 0; r < rows; ++r) {
            const QModelIndex idx = ui->blockTableWidget->model()->index(r, col);
            const QRect rect = ui->blockTableWidget->visualRect(idx);
            if (!rect.isNull()) {
                if (QWidget *w = ui->blockTableWidget->cellWidget(r, col)) {
                    w->setGeometry(rect);
                }
                ui->blockTableWidget->viewport()->update(rect);
            }
        }
        ui->blockTableWidget->resizeRowsToContents();
        ui->blockTableWidget->viewport()->update();
    }, Qt::DirectConnection);
    connect(ui->blockTableWidget->horizontalHeader(), &QHeaderView::geometriesChanged,
            this, [this]() {
        ui->blockTableWidget->viewport()->update();
    }, Qt::DirectConnection);
    m_detailTableConnection = connect(ui->detailTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, [this](int logicalIndex, int /*oldSize*/, int /*newSize*/) {
                Q_UNUSED(logicalIndex);
                ui->detailTableWidget->resizeRowsToContents();
                ui->detailTableWidget->viewport()->update();
            });
}

BlockTableForm::~BlockTableForm()
{
    delete ui;
}

void BlockTableForm::setModbusClient(ModbusClient *client)
{
    if (m_modbusClient == client) {
        return;
    }

    if (m_modbusClient) {
        disconnect(m_modbusClient, &ModbusClient::readCompleted,
                   this, &BlockTableForm::handleReadCompleted);
    }

    m_modbusClient = client;

    if (m_modbusClient) {
        connect(m_modbusClient, &ModbusClient::readCompleted,
                this, &BlockTableForm::handleReadCompleted);
        // requestAllValues();
    }
}

QList<int> BlockTableForm::getSplitterSizes()
{
    return ui->splitter->sizes();
}

void BlockTableForm::setSplitterSizes(const QList<int> &sizes)
{
    ui->splitter->setSizes(sizes);
}

void BlockTableForm::handleReadCompleted(int startAddress, const QVector<quint16> &values)
{
    std::variant<quint16, quint32> value;

    if (startAddress == BlockTableAddress::LaserControlBoardStatus ||
        startAddress == BlockTableAddress::PowerSupplyControlStatus)
    {
        value = (quint32(toBigEndian(values.last())) << 16) | toBigEndian(values.first());
    } else {
        value = toBigEndian(values.first());
    }

    const auto rowIt = m_addressToRow.constFind(startAddress);
    if (rowIt == m_addressToRow.constEnd()) {
        return;
    }

    const int row = rowIt.value();
    QTableWidgetItem *valueItem = ui->blockTableWidget->item(row, 2);
    if (!valueItem) {
        valueItem = new QTableWidgetItem;
        ui->blockTableWidget->setItem(row, 2, valueItem);
    }

    if (auto* val16 = std::get_if<quint16>(&value)) {
        valueItem->setText(QString::number(*val16));
        valueItem->setData(Qt::UserRole, QVariant::fromValue(*val16));
    } else if (auto* val32 = std::get_if<quint32>(&value)) {
        valueItem->setText(QString::number(*val32));
        valueItem->setData(Qt::UserRole, QVariant::fromValue(*val32));
    }
}

void BlockTableForm::showDetails(int address)
{
    for(int rowNumber = ui->detailTableWidget->rowCount(); rowNumber >= 0 ; rowNumber--)
        ui->detailTableWidget->removeRow(rowNumber);

    setupBlockStatusTable();

    auto value = ui->blockTableWidget->item(m_addressToRow[address], 2)->data(Qt::UserRole);
    //if addr
    if (address == BlockTableAddress::LaserControlBoardStatus)
        fillLaserControlBoardStatus();
    else if (address == BlockTableAddress::PowerSupplyControlStatus)
        fillGeneratorSetterStatus();
    else
        fillPowerSupplyQuantumtronsStatus();

    disconnect(m_detailTableConnection);

    populateBlockStatusTable(value);

    ui->detailTableWidget->resizeRowsToContents();
    m_detailTableConnection = connect(ui->detailTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
                                      this, [this](int logicalIndex, int /*oldSize*/, int /*newSize*/) {
        Q_UNUSED(logicalIndex);
        ui->detailTableWidget->resizeRowsToContents();
        ui->detailTableWidget->viewport()->update();
    });

    // qDebug() << ui->blockTableWidget->item(m_addressToRow[address], 2)->data(Qt::UserRole);
}

void BlockTableForm::setupTable()
{
    ui->blockTableWidget->setColumnCount(4);
    ui->blockTableWidget->setHorizontalHeaderLabels(
        {tr("Адрес"), tr("Название"), tr("Значение"), tr("Действия")});
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->blockTableWidget->verticalHeader()->setVisible(false);
    ui->blockTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // ui->blockTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->blockTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->blockTableWidget->horizontalHeader()->setHighlightSections(false);
}

void BlockTableForm::setupBlockStatusTable()
{
    ui->detailTableWidget->setColumnCount(3);
    ui->detailTableWidget->setHorizontalHeaderLabels(
        {tr("Номер бита"), tr("Название"), tr("Значение")});
    ui->detailTableWidget->verticalHeader()->setVisible(false);
    ui->detailTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->detailTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->detailTableWidget->setFocusPolicy(Qt::NoFocus);
}

void BlockTableForm::populateBlockTable()
{
    m_entries = {
        { BlockTableAddress::LaserControlBoardStatus,         tr("Статус платы управления лазером") },
        { BlockTableAddress::PowerSupplyControlStatus,        tr("Статус блока питания управления") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_1, tr("Статус блока питания кванторов #1") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_2, tr("Статус блока питания кванторов #2") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_3, tr("Статус блока питания кванторов #3") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_4, tr("Статус блока питания кванторов #4") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_5, tr("Статус блока питания кванторов #5") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_6, tr("Статус блока питания кванторов #6") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_7, tr("Статус блока питания кванторов #7") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_8, tr("Статус блока питания кванторов #8") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_9, tr("Статус блока питания кванторов #9") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus_10,tr("Статус блока питания кванторов #10")}
    };

    for (const auto &entry : m_entries) {
        insertRow(entry);
    }

    ui->blockTableWidget->resizeColumnsToContents();
}

void BlockTableForm::fillLaserControlBoardStatus()
{
    m_blockStatusEntries = {
        { LaserControlBoardStatusBits::HeaterStatus,                            tr("Состояние нагревателя") },
        { LaserControlBoardStatusBits::TempOfCase_1BelowMinLimit,               tr("Температура корпуса №1 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::TempOfCase_1AboveMaxLimit,               tr("Температура корпуса №1 выше максимальной предельной") },
        { LaserControlBoardStatusBits::TempOfCase_2BelowMinLimit,               tr("Температура корпуса №2 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::TempOfCase_2AboveMaxLimit,               tr("Температура корпуса №2 выше максимальной предельной") },
        { LaserControlBoardStatusBits::TempOfCool_1BelowMinLimit,               tr("Температура охладителя №1 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::TempOfCool_1AboveMaxLimit,               tr("Температура охладителя №1 выше максимальной предельной") },
        { LaserControlBoardStatusBits::TempOfCool_2BelowMinLimit,               tr("Температура охладителя №2 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::TempOfCool_2AboveMaxLimit,               tr("Температура охладителя №2 выше максимальной предельной") },
        { LaserControlBoardStatusBits::CoolFlowRate_1BelowMinLimit,             tr("Расход охладителя №1 ниже предельного значения") },
        { LaserControlBoardStatusBits::CoolFlowRate_1AboveMaxLimit,             tr("Расход охладителя №1 выше предельного значения") },
        { LaserControlBoardStatusBits::CoolFlowRate_2BelowMinLimit,             tr("Расход охладителя №2 ниже предельного значения") },
        { LaserControlBoardStatusBits::CoolFlowRate_2AboveMaxLimit,             tr("Расход охладителя №2 выше предельного значения") },
        { LaserControlBoardStatusBits::CoolFlowRate_3BelowMinLimit,             tr("Расход охладителя №3 ниже предельного значения") },
        { LaserControlBoardStatusBits::CoolFlowRate_3AboveMaxLimit,             tr("Расход охладителя №3 выше предельного значения") },
        { LaserControlBoardStatusBits::AirHumidity_1BelowMinLimit,              tr("Влажность воздуха №1 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::AirHumidity_1AboveMaxLimit,              tr("Влажность воздуха №1 выше максимальной предельной") },
        { LaserControlBoardStatusBits::AirTemperature_1BelowMinLimit,           tr("Температура воздуха №1 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::AirTemperature_1AboveMaxLimit,           tr("Температура воздуха №1 выше максимальной предельной") },
        { LaserControlBoardStatusBits::AirHumidity_2BelowMinLimit,              tr("Влажность воздуха №2 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::AirHumidity_2AboveMaxLimit,              tr("Влажность воздуха №2 выше максимальной предельной") },
        { LaserControlBoardStatusBits::AirTemperature_2BelowMinLimit,           tr("Температура воздуха №2 ниже минимальной предельной") },
        { LaserControlBoardStatusBits::AirTemperature_2AboveMaxLimit,           tr("Температура воздуха №2 выше максимальной предельной") },
        { LaserControlBoardStatusBits::PowerLaser_2BelowMinLimit,               tr("Мощность лазера ниже установленной") },
        { LaserControlBoardStatusBits::PowerLaser_2AboveMaxLimit,               tr("Мощность лазера выше установленной") },
        { LaserControlBoardStatusBits::LaserWorkMode_1Bit,                      tr("Режим работы лазера") },
        { LaserControlBoardStatusBits::SignalIsGood,                            tr("Сигнал «Исправен»") },
        { LaserControlBoardStatusBits::SignalIsReady,                           tr("Сигнал «Готов»") },
        { LaserControlBoardStatusBits::StateOfMasterOscillatorPumpSyncPulse,    tr("Состояние синхроимпульса накачки задающего генератора") },
        { LaserControlBoardStatusBits::StateOfAmplifierPumpSyncPulse,           tr("Состояние синхроимпульса накачки усилителей") },
        { LaserControlBoardStatusBits::StateOfRadiationGenerationSyncPulse,     tr("Состояние синхроимпульса генерации излучения") },
    };
}

void BlockTableForm::fillGeneratorSetterStatus()
{
    m_blockStatusEntries = {
        { GeneratorSetterStatusBits::ThermalStabilizationEnabled,                   tr("Термостабилизации включена") },
        { GeneratorSetterStatusBits::LaserDiodeTemperatureNormal,                   tr("Температура лазерного диода в норме") },
        { GeneratorSetterStatusBits::LaserDiodeOverheated,                          tr("Перегрев лазерного диода") },
        { GeneratorSetterStatusBits::DoublerCrystalTemperatureNormal,               tr("Температура кристалла удвоителя в норме") },
        { GeneratorSetterStatusBits::DoublerCrystalOverheated,                      tr("Перегрев кристалла удвоителя") },
        { GeneratorSetterStatusBits::TwelveVPowerSourceEnabled,                     tr("Силовой источник 12 В включен") },
        { GeneratorSetterStatusBits::LaserModuleOperational,                        tr("Лазерный модуль в рабочем состоянии") },
        { GeneratorSetterStatusBits::LaserDiodeVoltageRegulatorEnabled,             tr("Регулятор напряжения лазерного диода включен") },
        { GeneratorSetterStatusBits::LaserDiodeCurrentPulsesEnabled,                tr("Импульсы тока лазерного диода включены") },
        { GeneratorSetterStatusBits::CoolerOverheated,                              tr("Перегрев охладителя") },
        { GeneratorSetterStatusBits::PowerTransistorOverheated,                     tr("Перегрев силового транзистора") },
        { GeneratorSetterStatusBits::LaserDiodeOvercurrent,                         tr("Превышение тока лазерного диода") },
        { GeneratorSetterStatusBits::PowerTransistorOvervoltage,                    tr("Превышение напряжения на силовом транзисторе") },
        { GeneratorSetterStatusBits::AcoustoOpticShutterDriverFailure,              tr("Сбой драйвера акустооптического затвора") },
        { GeneratorSetterStatusBits::ExternalLock,                                  tr("Внешняя блокировка") },
        { GeneratorSetterStatusBits::SystemConfigurationLoaded,                     tr("Системная конфигурация загружена") },
        { GeneratorSetterStatusBits::DriverConfigurationLoaded,                     tr("Конфигурация драйвера загружена") },
        { GeneratorSetterStatusBits::TimingConfigurationLoaded,                     tr("Конфигурация тайминга загружена") },
        { GeneratorSetterStatusBits::DiodeThermalStabilizationConfigurationLoaded,  tr("Конфигурация термостабилизации диода загружена") },
        { GeneratorSetterStatusBits::CrystalThermalStabilizationConfigurationLoaded,tr("Конфигурация термостабилизации кристалла загружена") }
    };
}

void BlockTableForm::fillPowerSupplyQuantumtronsStatus()
{
    m_blockStatusEntries = {
        { PowerSupplyQuantumtronsStatusBits::PowerSupply,           tr("Источник питания включен/выключен") },
        { PowerSupplyQuantumtronsStatusBits::FrequencyModeControl,  tr("Управление частотным режимом") },
        { PowerSupplyQuantumtronsStatusBits::Synchronization,       tr("Синхронизация внутренняя/внешняя") },
        { PowerSupplyQuantumtronsStatusBits::PowerSupplyReadySignal,tr("Сигнал готовности источника питания")}
    };
}

void BlockTableForm::populateBlockStatusTable(QVariant value)
{
    for (auto &entry : m_blockStatusEntries) {
        entry.value = value;
        insertRowBlockStatus(entry);
    }

    ui->detailTableWidget->resizeColumnsToContents();
    // ui->detailTableWidget->columnWidth()
}

void BlockTableForm::insertRow(const BlockEntry &entry)
{
    const int row = ui->blockTableWidget->rowCount();
    ui->blockTableWidget->insertRow(row);

    auto *addressItem = new QTableWidgetItem(makeAddressString(entry.address));
    addressItem->setData(Qt::UserRole, entry.address);
    addressItem->setTextAlignment(Qt::AlignCenter);
    ui->blockTableWidget->setItem(row, 0, addressItem);

    auto *nameItem = new QTableWidgetItem(entry.name);
    nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap);
    ui->blockTableWidget->setItem(row, 1, nameItem);

    auto *valueItem = new QTableWidgetItem(tr("Н/Д"));
    valueItem->setTextAlignment(Qt::AlignCenter);
    ui->blockTableWidget->setItem(row, 2, valueItem);

    auto *button = new QPushButton(tr("Подробнее"), ui->blockTableWidget);
    ui->blockTableWidget->setCellWidget(row, 3, button);
    connect(button, &QPushButton::clicked, this, [this, entry] {
        showDetails(entry.address);
    });

    m_addressToRow.insert(entry.address, row);
}

void BlockTableForm::insertRowBlockStatus(const BlockStatusEntry &entry)
{
    const int row = ui->detailTableWidget->rowCount();
    ui->detailTableWidget->insertRow(row);

    quint32 value = (entry.value.toUInt() >> entry.address) & 1u;
    QString numOfBit = QString::number(entry.address);
    if (m_addressToRow.key(ui->blockTableWidget->currentRow()) == BlockTableAddress::LaserControlBoardStatus)
    {
        if (entry.address == LaserControlBoardStatusBits::LaserWorkMode_1Bit)
        {
            numOfBit += "-" + QString::number(LaserControlBoardStatusBits::LaserWorkMode_2Bit);
            const quint32 b1 = ((entry.value.toUInt() >> LaserControlBoardStatusBits::LaserWorkMode_1Bit) & 1u);
            const quint32 b2 = ((entry.value.toUInt() >> LaserControlBoardStatusBits::LaserWorkMode_2Bit) & 1u);
            value = (b2 << 1) | b1;
        }
    }

    auto *addressItem = new QTableWidgetItem(numOfBit);
    addressItem->setData(Qt::UserRole, numOfBit);
    addressItem->setTextAlignment(Qt::AlignCenter);
    ui->detailTableWidget->setItem(row, 0, addressItem);

    auto *nameItem = new QTableWidgetItem(entry.name);
    nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap);
    ui->detailTableWidget->setItem(row, 1, nameItem);

    // qDebug() << toBigEndian(quint16(entry.value.toUInt()));
    // qDebug() << ((toBigEndian(quint16(entry.value.toUInt())) >> entry.address) & 1u);
    // std::variant<quint16, quint32> value;
    // auto startAddress = m_addressToRow.key(ui->blockTableWidget->currentRow());

    // qDebug() << "m_addressToRow" << m_addressToRow.key(ui->blockTableWidget->currentRow());
    // qDebug() << "row" << ui->blockTableWidget->currentRow();
    // if (startAddress == BlockTableAddress::LaserControlBoardStatus ||
    //     startAddress == BlockTableAddress::PowerSupplyControlStatus) {
    //     value = (quint32(entry.value.) << 16) | values.last();
    // } else {
    //     value = quint16(values.first());
    // }
    auto *valueItem = new QTableWidgetItem(entry.value.isNull() ?
                                               tr("Н/Д") :
                                               QString::number(value));
    valueItem->setTextAlignment(Qt::AlignCenter);
    ui->detailTableWidget->setItem(row, 2, valueItem);
}

void BlockTableForm::requestAllValues() const
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
                                      client->readHoldingRegisters(address,
                                                                   address == BlockTableAddress::LaserControlBoardStatus ||
                                                                    address == BlockTableAddress::PowerSupplyControlStatus ? 2 : 1);
                                  },
                                  Qt::QueuedConnection);
    }
}

void BlockTableForm::on_pushButton_clicked()
{
    requestAllValues();
}

