#ifndef TEXTBUTTONFORM_H
#define TEXTBUTTONFORM_H

#include <QWidget>

namespace Ui {
class TextButtonForm;
}

class TextButtonForm : public QWidget
{
    Q_OBJECT

public:
    explicit TextButtonForm(QWidget *parent = nullptr);
    ~TextButtonForm();

    void setText(const QString &text);
    void setOnButton(bool flag);

signals:
    void sendState(bool value);

private:
    Ui::TextButtonForm *ui;

    bool m_isON = false;
};

#endif // TEXTBUTTONFORM_H
