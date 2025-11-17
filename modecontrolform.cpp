#include "modecontrolform.h"
#include "ui_modecontrolform.h"

ModeControlForm::ModeControlForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModeControlForm)
{
    ui->setupUi(this);

    connect(ui->pushButtonAutoMode, &QPushButton::clicked, this, &ModeControlForm::setAutoMode);
    connect(ui->pushButtonManualMode, &QPushButton::clicked, this, &ModeControlForm::setManualMode);
}

ModeControlForm::~ModeControlForm()
{
    delete ui;
}
