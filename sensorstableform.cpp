#include "sensorstableform.h"
#include "ui_sensorstableform.h"

SensorsTableForm::SensorsTableForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SensorsTableForm)
{
    ui->setupUi(this);
}

SensorsTableForm::~SensorsTableForm()
{
    delete ui;
}
