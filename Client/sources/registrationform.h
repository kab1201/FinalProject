#ifndef REGISTRATIONFORM_H
#define REGISTRATIONFORM_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class RegistrationForm;
}

class RegistrationForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrationForm(QWidget *parent = nullptr);
    ~RegistrationForm();

    void setSocket(QTcpSocket *newSocket);
    void setTextInfo(const QString &text);

signals:
    void loginRequested();
    void accepted(QString userLogin, QString userPassword);
    void rejected();

private slots:
    void on_loginPushButton_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::RegistrationForm *ui;
    QTcpSocket *m_socket;
};

#endif // REGISTRATIONFORM_H
