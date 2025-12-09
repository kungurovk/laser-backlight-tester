#ifndef SENSORSTABLEFORM_H
#define SENSORSTABLEFORM_H

#include <QWidget>

#include "modbusclient.h"

class ModbusClient;

namespace Ui {
class SensorsTableForm;
}

class SensorsTableForm : public QWidget, public ModbusBase
{
    Q_OBJECT

public:
    explicit SensorsTableForm(QWidget *parent = nullptr);
    ~SensorsTableForm();

    void setModbusClient(ModbusClient *client);

private slots:
    void handleReadCompleted(int startAddress, const QVector<quint16> &values);

private:
    struct BlockEntry
    {
        int address;
        QString name;
    };

    void setupTable();
    void populateTable();
    void insertRow(const BlockEntry &entry);
    void requestAllValues() const;

    Ui::SensorsTableForm *ui;
    ModbusClient *m_modbusClient = nullptr;
    QVector<BlockEntry> m_entries;
    QHash<int, int> m_addressToRow;
};

#endif // SENSORSTABLEFORM_H
