#include "modecontrolform.h"
#include "ui_modecontrolform.h"

ModeControlForm::ModeControlForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModeControlForm)
{
    ui->setupUi(this);

    const auto wireModeButton = [this](QPushButton *button, Mode mode) {
        connect(button, &QPushButton::clicked, this, [this, mode] {
            emit modeRequested(mode);
        });
    };

    wireModeButton(ui->pushButtonAutoMode, Mode::Auto);
    wireModeButton(ui->pushButtonManualMode, Mode::Manual);
    wireModeButton(ui->pushButtonDutyMode, Mode::Duty);
    wireModeButton(ui->pushButtonPrepareMode, Mode::Prepare);
    wireModeButton(ui->pushButtonWorkMode, Mode::Work);
}

ModeControlForm::~ModeControlForm()
{
    delete ui;
}
