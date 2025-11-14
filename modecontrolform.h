#ifndef MODECONTROLFORM_H
#define MODECONTROLFORM_H

#include <QWidget>

namespace Ui {
class ModeControlForm;
}

class ModeControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit ModeControlForm(QWidget *parent = nullptr);
    ~ModeControlForm();

private:
    Ui::ModeControlForm *ui;
};

#endif // MODECONTROLFORM_H
