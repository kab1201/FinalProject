#include "registrationform.h"
#include "ui_registrationform.h"
#include <QMessageBox>

RegistrationForm::RegistrationForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegistrationForm)
{
    ui->setupUi(this);
}

RegistrationForm::~RegistrationForm()
{
    delete ui;
}

void RegistrationForm::on_loginPushButton_clicked()
{
    emit loginRequested();
}

void RegistrationForm::on_buttonBox_accepted()
{
    QString message;
    QString infoMessage;
    if(!ui->nameEdit->text().isEmpty() && !ui->loginEdit->text().isEmpty() &&
        !ui->passwordEdit->text().isEmpty() && !ui->confirmPasswordEdit->text().isEmpty())
    {
        if(ui->passwordEdit->text() == ui->confirmPasswordEdit->text())
        {
            auto userName = ui->nameEdit->text();
            auto userLogin = ui->loginEdit->text();
            auto userPassword = ui->passwordEdit->text();
            message = "110;" + userName + ";" + userLogin + ";" + userPassword + ";";
            m_socket->write(message.toUtf8());
        }
        else
        {
            infoMessage = "<font color=red>" + tr("Passwords don't match. Try again.") + "</font>";
            ui->registrationResultLabel->setText(infoMessage);
            return;
        }
    }
    else
    {
        infoMessage = "<font color=red>" + tr("All fields must be filled in!") + "</font>";
        ui->registrationResultLabel->setText(infoMessage);
        return;
    }

    emit accepted(ui->loginEdit->text(), ui->passwordEdit->text());
}

void RegistrationForm::on_buttonBox_rejected()
{
    emit rejected();
}

void RegistrationForm::setSocket(QTcpSocket *newSocket)
{
    m_socket = newSocket;
}

void RegistrationForm::setTextInfo(const QString &text)
{
    ui->registrationResultLabel->setText(text);
}
