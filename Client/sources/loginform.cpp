#include "loginform.h"
#include "ui_loginform.h"
//#include <QMessageBox>

LoginForm::LoginForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);
}

LoginForm::~LoginForm()
{
    delete ui;
}

void LoginForm::on_registrationPushButton_clicked()
{
    emit registerRequested();
}

void LoginForm::on_buttonBox_accepted()
{
    emit accepted(ui->loginEdit->text(), ui->passwordEdit->text());
}

void LoginForm::on_buttonBox_rejected()
{
    emit rejected();
}

void LoginForm::setTextInfo(const QString &text)
{
    ui->loginResultLabel->setText(text);
}
