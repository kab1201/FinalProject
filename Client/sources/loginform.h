#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);
    ~LoginForm();

    void setTextInfo(const QString &text);

signals:
    void registerRequested();
    void accepted(QString userLogin, QString userPassword);
    void rejected();

private slots:
    void on_registrationPushButton_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::LoginForm *ui;
};

#endif // LOGINFORM_H
