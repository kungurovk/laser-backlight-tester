#include "blocktableform.h"
#include "ui_blocktableform.h"

#include "enums.h"
#include "modbusclient.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

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
    populateTable();
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

        const quint16 value = values.at(i);
        valueItem->setText(QString::number(value));
        valueItem->setData(Qt::UserRole, QVariant::fromValue(value));
    }
}

void BlockTableForm::showDetails(int address)
{
    const int row = m_addressToRow.value(address, -1);
    if (row < 0) {
        return;
    }

    const QString name = ui->blockTableWidget->item(row, 1)
            ? ui->blockTableWidget->item(row, 1)->text()
            : tr("Неизвестный блок");
    const QString value = ui->blockTableWidget->item(row, 2)
            ? ui->blockTableWidget->item(row, 2)->text()
            : tr("Н/Д");

    QMessageBox::information(this,
                             tr("Информация о блоке"),
                             tr("%1 (%2)\nЗначение: %3")
                                 .arg(name)
                                 .arg(makeAddressString(address))
                                 .arg(value));

    emit detailsRequested(address);
}

void BlockTableForm::setupTable()
{
    ui->blockTableWidget->setColumnCount(4);
    ui->blockTableWidget->setHorizontalHeaderLabels(
        {tr("Адрес"), tr("Название"), tr("Значение"), tr("Действия")});
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->blockTableWidget->verticalHeader()->setVisible(false);
    ui->blockTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->blockTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->blockTableWidget->setFocusPolicy(Qt::NoFocus);
}

void BlockTableForm::populateTable()
{
    m_entries = {
        { BlockTableAddress::LaserControlBoardStatus,        tr("Статус платы управления лазером") },
        { BlockTableAddress::PowerSupplyControlStatus,       tr("Статус блока питания управления") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus1, tr("Статус блока питания кванторов #1") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus2, tr("Статус блока питания кванторов #2") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus3, tr("Статус блока питания кванторов #3") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus4, tr("Статус блока питания кванторов #4") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus5, tr("Статус блока питания кванторов #5") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus6, tr("Статус блока питания кванторов #6") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus7, tr("Статус блока питания кванторов #7") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus8, tr("Статус блока питания кванторов #8") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus9, tr("Статус блока питания кванторов #9") },
        { BlockTableAddress::PowerSupplyQuantumtronsStatus10,tr("Статус блока питания кванторов #10")}
    };

    for (const auto &entry : m_entries) {
        insertRow(entry);
    }

    ui->blockTableWidget->resizeColumnsToContents();
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
    nameItem->setTextAlignment(Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap);
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
