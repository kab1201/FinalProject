#ifndef CONNECTFORM_H
#define CONNECTFORM_H

#include <QWidget>
#include <QTcpSocket>

namespace Ui {
class ConnectForm;
}

class ConnectForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectForm(QWidget *parent = nullptr);
    ~ConnectForm();

    void setSocket(QTcpSocket *newSocket);

signals:
    void acceptedConnect(QString ip, int port);
    void rejected();

private slots:
    void on_connectButton_clicked();
    void on_exitButton_clicked();

private:
    Ui::ConnectForm *ui;
    QTcpSocket *m_socket;
};

#endif // CONNECTFORM_H
