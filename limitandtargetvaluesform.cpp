#include "limitandtargetvaluesform.h"
#include "ui_limitandtargetvaluesform.h"

LimitAndTargetValuesForm::LimitAndTargetValuesForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LimitAndTargetValuesForm)
{
    ui->setupUi(this);
}

LimitAndTargetValuesForm::~LimitAndTargetValuesForm()
{
    delete ui;
}
