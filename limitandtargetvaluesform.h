#ifndef LIMITANDTARGETVALUESFORM_H
#define LIMITANDTARGETVALUESFORM_H

#include <QWidget>

#include "modbusclient.h"

namespace Ui {
class LimitAndTargetValuesForm;
}

class LimitAndTargetValuesForm : public QWidget, public ModbusBase
{
    Q_OBJECT

public:
    explicit LimitAndTargetValuesForm(QWidget *parent = nullptr);
    ~LimitAndTargetValuesForm();

    void setModbusClient(ModbusClient *client);

private slots:
    void handleReadCompleted(int startAddress, const QVector<quint16> &values);

    void on_pushButton_clicked();

private:
    struct BlockEntry
    {
        int address;
        QString name;
    };

    void setupTable();
    void populateTable();
    void insertRow(const BlockEntry &entry);
    void requestValueByValue() const;
    void requestAllValues() const;

    Ui::LimitAndTargetValuesForm *ui;
    ModbusClient *m_modbusClient = nullptr;
    QVector<BlockEntry> m_entries;
    QHash<int, int> m_addressToRow;
};

#endif // LIMITANDTARGETVALUESFORM_H
