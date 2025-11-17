#ifndef MODECONTROLFORM_H
#define MODECONTROLFORM_H

#include <QWidget>

#include "enums.h"

namespace Ui {
class ModeControlForm;
}

class ModeControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit ModeControlForm(QWidget *parent = nullptr);
    ~ModeControlForm();

signals:
    void modeRequested(Mode mode);

private:
    Ui::ModeControlForm *ui;
};

#endif // MODECONTROLFORM_H
