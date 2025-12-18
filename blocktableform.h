#ifndef BLOCKTABLEFORM_H
#define BLOCKTABLEFORM_H

#include <QHash>
#include <QVector>
#include <QWidget>
#include <QString>
#include <QVariant>

#include "modbusclient.h"

class ModbusClient;

namespace Ui {
class BlockTableForm;
}

class BlockTableForm : public QWidget, public ModbusBase
{
    Q_OBJECT

public:
    explicit BlockTableForm(QWidget *parent = nullptr);
    ~BlockTableForm();

    void setModbusClient(ModbusClient *client) override;

    QList<int> getSplitterSizes();
    void setSplitterSizes(const QList<int> &);

    void requestValueByValue() const;
    void requestAllValues() const override;

private slots:
    void handleReadCompleted(int startAddress, const QVector<quint16> &values);
    void showDetails(int address);

    void on_pushButton_clicked();

private:
    struct BlockEntry
    {
        int address;
        QString name;
    };

    struct BlockStatusEntry
    {
        int address;
        QString name;
        QVariant value;
    };

    void setupTable();
    void setupBlockStatusTable();
    void populateBlockTable();
    void fillLaserControlBoardStatus();
    void fillGeneratorSetterStatus();
    void fillPowerSupplyQuantumtronsStatus();
    void populateBlockStatusTable(QVariant value);
    void insertRow(const BlockEntry &entry);
    void insertRowBlockStatus(const BlockStatusEntry &entry);

    Ui::BlockTableForm *ui;
    ModbusClient *m_modbusClient = nullptr;
    QVector<BlockEntry> m_entries;
    QHash<int, int> m_addressToRow;

    QVector<BlockStatusEntry> m_blockStatusEntries;
    QMetaObject::Connection m_detailTableConnection;
};

#endif // BLOCKTABLEFORM_H
