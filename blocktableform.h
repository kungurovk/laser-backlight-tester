#ifndef BLOCKTABLEFORM_H
#define BLOCKTABLEFORM_H

#include <QHash>
#include <QVector>
#include <QWidget>
#include <QString>

class ModbusClient;

namespace Ui {
class BlockTableForm;
}

class BlockTableForm : public QWidget
{
    Q_OBJECT

public:
    explicit BlockTableForm(QWidget *parent = nullptr);
    ~BlockTableForm();

    void setModbusClient(ModbusClient *client);

signals:
    void detailsRequested(int address);

private slots:
    void handleReadCompleted(int startAddress, const QVector<quint16> &values);
    void showDetails(int address);

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

    Ui::BlockTableForm *ui;
    ModbusClient *m_modbusClient = nullptr;
    QVector<BlockEntry> m_entries;
    QHash<int, int> m_addressToRow;
};

#endif // BLOCKTABLEFORM_H
