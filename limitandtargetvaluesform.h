#ifndef LIMITANDTARGETVALUESFORM_H
#define LIMITANDTARGETVALUESFORM_H

#include <QWidget>

namespace Ui {
class LimitAndTargetValuesForm;
}

class LimitAndTargetValuesForm : public QWidget
{
    Q_OBJECT

public:
    explicit LimitAndTargetValuesForm(QWidget *parent = nullptr);
    ~LimitAndTargetValuesForm();

private:
    Ui::LimitAndTargetValuesForm *ui;
};

#endif // LIMITANDTARGETVALUESFORM_H
