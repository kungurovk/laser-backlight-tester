#include "blocktableform.h"
#include "ui_blocktableform.h"

#include "enums.h"
#include "modbusclient.h"
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
    connect(ui->detailTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, [this](int logicalIndex, int /*oldSize*/, int /*newSize*/) {
                Q_UNUSED(logicalIndex);
                ui->detailTableWidget->resizeRowsToContents();
                ui->detailTableWidget->viewport()->update();
            }, Qt::DirectConnection);
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
        requestAllValues();
    }
}

void BlockTableForm::handleReadCompleted(int startAddress, const QVector<quint16> &values)
{
    for (int i = 0; i < values.size(); ++i) {
        const int address = startAddress + i;
        const auto rowIt = m_addressToRow.constFind(address);
        if (rowIt == m_addressToRow.constEnd()) {
            continue;
        }

        const int row = rowIt.value();
        QTableWidgetItem *valueItem = ui->blockTableWidget->item(row, 2);
        if (!valueItem) {
            valueItem = new QTableWidgetItem;
            ui->blockTableWidget->setItem(row, 2, valueItem);
        }

        //todo type from address
        const quint16 value = values.at(i);
        valueItem->setText(QString::number(toBigEndian(value)));
        valueItem->setData(Qt::UserRole, QVariant::fromValue(value));
    }
}

void BlockTableForm::showDetails(int address)
{
    for(int rowNumber = ui->detailTableWidget->rowCount(); rowNumber >= 0 ; rowNumber--)
        ui->detailTableWidget->removeRow(rowNumber);

    setupBlockStatusTable();

    auto value = ui->blockTableWidget->item(m_addressToRow[address], 2)->data(Qt::UserRole);
    //if addr
    populateBlockStatusTable(value);
    //else

    qDebug() << ui->blockTableWidget->item(m_addressToRow[address], 2)->data(Qt::UserRole);
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

void BlockTableForm::populateBlockStatusTable(QVariant value)
{
    m_blockStatusEntries = {
        { BlockStatusBitNumbers::PowerSupply,           tr("Источник питания включен/выключен") },
        { BlockStatusBitNumbers::FrequencyModeControl,  tr("Управление частотным режимом") },
        { BlockStatusBitNumbers::Synchronization,       tr("Синхронизация внутренняя/внешняя") },
        { BlockStatusBitNumbers::PowerSupplyReadySignal,tr("Сигнал готовности источника питания")}
    };

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

    auto *addressItem = new QTableWidgetItem(QString::number(entry.address));
    addressItem->setData(Qt::UserRole, entry.address);
    addressItem->setTextAlignment(Qt::AlignCenter);
    ui->detailTableWidget->setItem(row, 0, addressItem);

    auto *nameItem = new QTableWidgetItem(entry.name);
    nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap);
    ui->detailTableWidget->setItem(row, 1, nameItem);

    // qDebug() << toBigEndian(quint16(entry.value.toUInt()));
    // qDebug() << ((toBigEndian(quint16(entry.value.toUInt())) >> entry.address) & 1u);
    auto *valueItem = new QTableWidgetItem(entry.value.isNull() ?
                                               tr("Н/Д") :
                                               QString::number((entry.value.toUInt() >> entry.address) & 1u));
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
                                      client->readHoldingRegisters(address, 1);
                                  },
                                  Qt::QueuedConnection);
    }
}
