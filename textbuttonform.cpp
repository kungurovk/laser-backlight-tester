#include "textbuttonform.h"
#include "ui_textbuttonform.h"

TextButtonForm::TextButtonForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextButtonForm)
{
    ui->setupUi(this);

    connect(ui->onOffPushButton, &QPushButton::clicked, [this](){
        setOnButton(!m_isON);
    });
}

TextButtonForm::~TextButtonForm()
{
    delete ui;
}

void TextButtonForm::setText(const QString &text)
{
    ui->textLabel->setText(text);
}

void TextButtonForm::setOnButton(bool flag)
{
    m_isON = flag;
    m_isON ? ui->onOffPushButton->setText("Выключить") : ui->onOffPushButton->setText("Включить");
    sendState(m_isON);
}
