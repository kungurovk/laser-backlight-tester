#include "blocktableform.h"
#include "ui_blocktableform.h"

BlockTableForm::BlockTableForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BlockTableForm)
{
    ui->setupUi(this);
}

BlockTableForm::~BlockTableForm()
{
    delete ui;
}
