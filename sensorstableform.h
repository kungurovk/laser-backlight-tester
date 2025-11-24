#ifndef SENSORSTABLEFORM_H
#define SENSORSTABLEFORM_H

#include <QWidget>

namespace Ui {
class SensorsTableForm;
}

class SensorsTableForm : public QWidget
{
    Q_OBJECT

public:
    explicit SensorsTableForm(QWidget *parent = nullptr);
    ~SensorsTableForm();

private:
    Ui::SensorsTableForm *ui;
};

#endif // SENSORSTABLEFORM_H
