#include "modecontrolform.h"
#include "ui_modecontrolform.h"

ModeControlForm::ModeControlForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModeControlForm)
{
    ui->setupUi(this);
}

ModeControlForm::~ModeControlForm()
{
    delete ui;
}
