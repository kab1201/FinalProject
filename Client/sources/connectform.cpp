#include "connectform.h"
#include "ui_connectform.h"

#include <QMessageBox>

ConnectForm::ConnectForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectForm)
{
    ui->setupUi(this);
    ui->ipLineEdit->setText("127.0.0.1");
    ui->portLineEdit->setText("5555");
}

ConnectForm::~ConnectForm()
{
    delete ui;
}

void ConnectForm::on_connectButton_clicked()
{
    QString ipAddress = ui->ipLineEdit->text();
    qint16 port = ui->portLineEdit->text().toInt();

    if (ui->connectButton->text() == QString(tr("Connect")))
    {
        m_socket->abort();
        m_socket->connectToHost(ipAddress, port);

        if (!m_socket->waitForConnected())
        {

            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Client"));
            msgBox.setText(tr("Runtime Out"));
            msgBox.resize(50,30);
            msgBox.exec();
            return;
        }

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Client"));
        msgBox.setText(tr("Successful Connection"));
        msgBox.resize(50,40);
        msgBox.exec();

        ui->connectButton->setText(tr("Disconnect"));
        ui->exitButton->setEnabled(false);
        ui->ipLineEdit->setEnabled(false);
        ui->portLineEdit->setEnabled(false);
    }
    else
    {
        m_socket->disconnectFromHost();

        ui->connectButton->setText(tr("Connect"));
        ui->exitButton->setEnabled(false);
        ui->ipLineEdit->setEnabled(true);
        ui->portLineEdit->setEnabled(true);
    }

    emit acceptedConnect(ipAddress, port);
}

void ConnectForm::on_exitButton_clicked()
{
    emit rejected();
}

void ConnectForm::setSocket(QTcpSocket *newSocket)
{
    m_socket = newSocket;
}

