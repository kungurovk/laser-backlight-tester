#ifndef MODECONTROLFORM_H
#define MODECONTROLFORM_H

#include <QWidget>

#include "enums.h"
#include "modbusclient.h"

class ModbusClient;

namespace Ui {
class ModeControlForm;
}

class ModeControlForm : public QWidget, public ModbusBase
{
    Q_OBJECT

public:
    explicit ModeControlForm(QWidget *parent = nullptr);
    ~ModeControlForm();

    void setModbusClient(ModbusClient *client);

private slots:
    void handleReadCompleted(int startAddress, const QVector<quint16> &values);
    void sendState(int address, bool value);
    void requestAllValues() const;

    void on_pushButton_clicked();

signals:
    void modeRequested(Mode mode);

private:

    Ui::ModeControlForm *ui;
    ModbusClient *m_modbusClient = nullptr;
};

#endif // MODECONTROLFORM_H
