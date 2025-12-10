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

private:
    Ui::LimitAndTargetValuesForm *ui;
};

#endif // LIMITANDTARGETVALUESFORM_H
