#ifndef GENERATORSETTERFORM_H
#define GENERATORSETTERFORM_H

#include <QWidget>

#include "modbusclient.h"

class ModbusClient;

namespace Ui {
class GeneratorSetterForm;
}

class GeneratorSetterForm : public QWidget, public ModbusBase
{
    Q_OBJECT

public:
    explicit GeneratorSetterForm(QWidget *parent = nullptr);
    ~GeneratorSetterForm();

    void setModbusClient(ModbusClient *client);

private slots:
    void handleReadCompleted(int startAddress, const QVector<quint16> &values);
    void sendState(int address, bool value);

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
    void requestAllValues() const;

    Ui::GeneratorSetterForm *ui;
    ModbusClient *m_modbusClient = nullptr;
    QVector<BlockEntry> m_entries;
    QHash<int, int> m_addressToRow;
};

#endif // GENERATORSETTERFORM_H
