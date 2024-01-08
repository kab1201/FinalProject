#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class StartScreen;
}

class StartScreen : public QDialog
{
    Q_OBJECT

public:
    explicit StartScreen(QTcpSocket *socket = nullptr,
                         QWidget *parent = nullptr);
    ~StartScreen();
    void setConnectForm();
    void setLoginForm();
    void setRegistrationForm();
    void loginResult();

    QTcpSocket *getSocket() const;
    QString getCurrentLogin() const;
    QString getAllConnectedUsers() const;

public slots:
    void onLoggedIn(QString userLogin, QString userPassword);
    void onRejectRequested();

private:
    Ui::StartScreen *ui;
    QTcpSocket *m_socket;

    QString m_currentLogin;
    QString m_allConnectedUsers;

    size_t cntAttemptsLogin {0};
};

#endif // STARTSCREEN_H
