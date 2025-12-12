#include "generatorsetterform.h"
#include "ui_generatorsetterform.h"

#include "modbusclient.h"
#include "enums.h"
#include "endianutils.h"
#include "textbuttonform.h"

#include <cstring>

namespace {
QString makeAddressString(int address)
{
    return QStringLiteral("0x%1").arg(address, 0, 16).toUpper();
}
}

GeneratorSetterForm::GeneratorSetterForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GeneratorSetterForm)
{
    ui->setupUi(this);
    setupTable();
    populateTable();
    connect(ui->generatorTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, [this](int logicalIndex, int /*oldSize*/, int /*newSize*/) {
                Q_UNUSED(logicalIndex);
                ui->generatorTableWidget->resizeRowsToContents();
                ui->generatorTableWidget->viewport()->update();
            }, Qt::DirectConnection);
}

GeneratorSetterForm::~GeneratorSetterForm()
{
    delete ui;
}

void GeneratorSetterForm::setModbusClient(ModbusClient *client)
{
    if (m_modbusClient == client) {
        return;
    }

    if (m_modbusClient) {
        disconnect(m_modbusClient, &ModbusClient::readCompleted,
                   this, &GeneratorSetterForm::handleReadCompleted);
    }

    m_modbusClient = client;

    if (m_modbusClient) {
        connect(m_modbusClient, &ModbusClient::readCompleted,
                this, &GeneratorSetterForm::handleReadCompleted);
        // requestAllValues();
    }
}

void GeneratorSetterForm::handleReadCompleted(int startAddress, const QVector<quint16> &values)
{
    std::variant<quint16, float> value;

    if (startAddress == GeneratorSetterAddress::TermoStableOnOff ||
        startAddress == GeneratorSetterAddress::ImpulseOnOff)
    {
        value = toBigEndian(values.first());
    }
    else
    {
        quint32 val32 = (quint32(toBigEndian(values.last())) << 16) | toBigEndian(values.first());
        float fValue = 0.f;
        std::memcpy(&fValue, &val32, sizeof(fValue));
        value = fValue;
    }

    const auto rowIt = m_addressToRow.constFind(startAddress);
    if (rowIt == m_addressToRow.constEnd()) {
        return;
    }

    const int row = rowIt.value();

    QTableWidgetItem *valueItem = ui->generatorTableWidget->item(row, 2);
    if (!valueItem) {
        valueItem = new QTableWidgetItem;
        ui->generatorTableWidget->setItem(row, 2, valueItem);
    }

    if (auto* val16 = std::get_if<quint16>(&value)) {
        TextButtonForm *textButtonItem = qobject_cast<TextButtonForm*>(ui->generatorTableWidget->cellWidget(row, 1));
        if (textButtonItem)
            textButtonItem->setOnButton(*val16 ? true : false);

        valueItem->setText(QString::number(*val16));
        valueItem->setData(Qt::UserRole, QVariant::fromValue(*val16));
    } else if (auto* fValue = std::get_if<float>(&value)) {
        valueItem->setText(QString::number(*fValue));
        valueItem->setData(Qt::UserRole, QVariant::fromValue(*fValue));
    }
}

void GeneratorSetterForm::sendState(int address, bool value)
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
                                                              value ? toLittleEndian(quint16(States::On)) : toLittleEndian(quint16(States::Off)));
                              },
                              Qt::QueuedConnection);

    //!НЕ УСПЕВАЕТ ВКЛЮЧИТЬСЯ
    // QMetaObject::invokeMethod(m_modbusClient,
    //                           [client = m_modbusClient, address, value]() {
    //                               if (!client) {
    //                                   return;
    //                               }
    //                               client->readHoldingRegisters(address, 1);
    //                           },
    //                           Qt::QueuedConnection);
}

void GeneratorSetterForm::setupTable()
{
    ui->generatorTableWidget->setColumnCount(3);
    ui->generatorTableWidget->setHorizontalHeaderLabels(
        {tr("Адрес"), tr("Название"), tr("Значение")});
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->generatorTableWidget->verticalHeader()->setVisible(false);
    ui->generatorTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->generatorTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->generatorTableWidget->setFocusPolicy(Qt::NoFocus);
}

void GeneratorSetterForm::populateTable()
{
    m_entries = {
        { GeneratorSetterAddress::TermoStableOnOff,     tr("термостабилизацию") },
        { GeneratorSetterAddress::ImpulseOnOff,         tr("импульсы") },
        { GeneratorSetterAddress::DiodTemperature,      tr("Температура диода") },
        { GeneratorSetterAddress::CrystalTemperature,   tr("Температура кристалла") }
    };

    for (const auto &entry : m_entries) {
        insertRow(entry);
    }

    ui->generatorTableWidget->resizeColumnsToContents();
}

void GeneratorSetterForm::insertRow(const BlockEntry &entry)
{
    const int row = ui->generatorTableWidget->rowCount();
    ui->generatorTableWidget->insertRow(row);

    auto *addressItem = new QTableWidgetItem(makeAddressString(entry.address));
    addressItem->setData(Qt::UserRole, entry.address);
    addressItem->setTextAlignment(Qt::AlignCenter);
    ui->generatorTableWidget->setItem(row, 0, addressItem);

    if (entry.address == GeneratorSetterAddress::TermoStableOnOff ||
        entry.address == GeneratorSetterAddress::ImpulseOnOff)
    {
        auto *textButtonItem = new TextButtonForm;
        textButtonItem->setText(entry.name);
        ui->generatorTableWidget->setCellWidget(row, 1, textButtonItem);
        connect(textButtonItem, &TextButtonForm::sendState, this, [this, entry](bool value) {
            sendState(entry.address, value);
        });
    }
    else
    {
        auto *nameItem = new QTableWidgetItem(entry.name);
        nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap);
        ui->generatorTableWidget->setItem(row, 1, nameItem);
    }

    auto *valueItem = new QTableWidgetItem(tr("Н/Д"));
    valueItem->setTextAlignment(Qt::AlignCenter);
    ui->generatorTableWidget->setItem(row, 2, valueItem);

    m_addressToRow.insert(entry.address, row);
}

void GeneratorSetterForm::requestAllValues() const
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
                                                                   address == GeneratorSetterAddress::TermoStableOnOff ||
                                                                           address == GeneratorSetterAddress::ImpulseOnOff ? 1 : 2);
                                  },
                                  Qt::QueuedConnection);
    }
}

void GeneratorSetterForm::on_pushButton_clicked()
{
    requestAllValues();
}

