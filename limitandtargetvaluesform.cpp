#include "limitandtargetvaluesform.h"
#include "ui_limitandtargetvaluesform.h"

LimitAndTargetValuesForm::LimitAndTargetValuesForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LimitAndTargetValuesForm)
{
    ui->setupUi(this);
}

LimitAndTargetValuesForm::~LimitAndTargetValuesForm()
{
    delete ui;
}

void LimitAndTargetValuesForm::setModbusClient(ModbusClient *client)
{
    // if (m_modbusClient == client) {
    //     return;
    // }

    // if (m_modbusClient) {
    //     disconnect(m_modbusClient, &ModbusClient::readCompleted,
    //                this, &SensorsTableForm::handleReadCompleted);
    // }

    // m_modbusClient = client;

    // if (m_modbusClient) {
    //     connect(m_modbusClient, &ModbusClient::readCompleted,
    //             this, &SensorsTableForm::handleReadCompleted);
    //     requestAllValues();
    // }
}

void LimitAndTargetValuesForm::handleReadCompleted(int startAddress, const QVector<quint16> &values)
{
    // for (int i = 0; i < values.size(); ++i) {
    //     const int address = startAddress + i;
    //     const auto rowIt = m_addressToRow.constFind(address);
    //     if (rowIt == m_addressToRow.constEnd()) {
    //         continue;
    //     }

    //     const int row = rowIt.value();
    //     QTableWidgetItem *valueItem = ui->sensorsTableWidget->item(row, 2);
    //     if (!valueItem) {
    //         valueItem = new QTableWidgetItem;
    //         ui->sensorsTableWidget->setItem(row, 2, valueItem);
    //     }

    //     const quint16 value = values.at(i);
    //     valueItem->setText(QString::number(value));
    //     valueItem->setData(Qt::UserRole, QVariant::fromValue(value));
    // }
}
