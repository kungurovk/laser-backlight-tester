#ifndef BLOCKTABLEFORM_H
#define BLOCKTABLEFORM_H

#include <QWidget>

namespace Ui {
class BlockTableForm;
}

class BlockTableForm : public QWidget
{
    Q_OBJECT

public:
    explicit BlockTableForm(QWidget *parent = nullptr);
    ~BlockTableForm();

private:
    Ui::BlockTableForm *ui;
};

#endif // BLOCKTABLEFORM_H
