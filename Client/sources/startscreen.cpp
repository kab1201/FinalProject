#include "startscreen.h"
#include "ui_startscreen.h"

#include <QMessageBox>

StartScreen::StartScreen(QTcpSocket *socket,
                         QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartScreen)
{
    ui->setupUi(this);

    if(socket)
        m_socket = socket;
    else
        m_socket = new QTcpSocket();

    ui->connectWidget->setSocket(m_socket);
    ui->registrationWidget->setSocket(m_socket);

    connect(m_socket, &QTcpSocket::readyRead, this, &StartScreen::loginResult);
    connect(ui->connectWidget, &ConnectForm::acceptedConnect, this, &StartScreen::setLoginForm);
    connect(ui->connectWidget, &ConnectForm::rejected, this, &StartScreen::onRejectRequested);
    connect(ui->loginWidget, &LoginForm::registerRequested, this, &StartScreen::setRegistrationForm);
    connect(ui->loginWidget, &LoginForm::accepted, this, &StartScreen::onLoggedIn);
    connect(ui->loginWidget, &LoginForm::rejected, this, &StartScreen::onRejectRequested);
    connect(ui->registrationWidget, &RegistrationForm::loginRequested, this, &StartScreen::setLoginForm);
    connect(ui->registrationWidget, &RegistrationForm::accepted, this, &StartScreen::onLoggedIn);
    connect(ui->registrationWidget, &RegistrationForm::rejected, this, &StartScreen::onRejectRequested);
    this->setWindowTitle(tr("Connect to Server"));
}

StartScreen::~StartScreen()
{
    delete ui;
}

void StartScreen::setConnectForm()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void StartScreen::setLoginForm()
{
    ui->stackedWidget->setCurrentIndex(1);
    this->setWindowTitle(tr("Login to Server"));
}

void StartScreen::setRegistrationForm()
{
    ui->stackedWidget->setCurrentIndex(2);
    this->setWindowTitle(tr("New Registration"));
}

/* COMMANDS FROM SERVER
100 - new user join to chat
111 - registration (User don't add to database)
112 - registration (User add to database)
113 - registration (User is already exists)
114 - registration (User don't add to database: The login must contain letters and numbers only)
115 - registration (User don't add to database: Login 'all' is reserved)
116 - registration (User don't add to database: Password must be equal to or more than 5 characters long)
117 - registration (User don't add to database: Password must not exceed 20 characters long)
118 - registration (User don't add to database: Password contains invalid characters)
121 - login (Authentication success)
122 - login (Authentication failed)
123 - login (Authentication failed: Invalid login)
124 - login (Authentication failed: Invalid password)
125 - login (Authentication failed: User used all attempts)
126 - login (user already online)

*/
void StartScreen::loginResult()
{
    QByteArray message;
    message = m_socket->readAll();
    QString str = QString(message);
    int command = str.section(';',0,0).toInt();
    QString userLogin = str.section(';',1,1);
    QString allConnectedUsers = str.section(';',2);
    QString infoMessage;

    switch (command) {
    case 111:
        if (userLogin.size() > 0) {
            infoMessage = "<font color=red>" + tr("User with login ") + userLogin +
                          tr(" not added in database! Registration failed.") + "</font>";
            ui->registrationWidget->setTextInfo(infoMessage);
        }
        break;
    case 112:
        if (userLogin.size() > 0) {
            infoMessage = "<font color=green>" + tr("User with login ") + userLogin +
                          tr(" add in database! Registration success") + ".</font>";
            ui->registrationWidget->setTextInfo(infoMessage);
            QMessageBox(QMessageBox::Information,
                        QObject::tr("Registration"),
                        QObject::tr("Registration of a new user was successful!"),
                        QMessageBox::Ok).exec();
            m_currentLogin = userLogin;
            m_allConnectedUsers = allConnectedUsers;
            accept();
            this->close();
        }
        break;
    case 113:
        if (userLogin.size() > 0) {
            infoMessage = "<font color=red>" + tr("User with login ") + userLogin + tr(" is already exists! Try again.") + "</font>";
            ui->registrationWidget->setTextInfo(infoMessage);
        }
        break;
    case 114:
        ui->registrationWidget->setTextInfo("<font color=red>" + tr("The login must contain letters and numbers only!") + "</font>");
        break;
    case 115:
        ui->registrationWidget->setTextInfo("<font color=red>" + tr("Login 'all' is reserved!") + "</font>");
        break;
    case 116:
        ui->registrationWidget->setTextInfo("<font color=red>" + tr("Password must be equal to or more than 5 characters long!") + "</font>");
        break;
    case 117:
        ui->registrationWidget->setTextInfo("<font color=red>" + tr("Password must not exceed 20 characters long!") + "</font>");
        break;
    case 118:
        ui->registrationWidget->setTextInfo("<font color=red>" + tr("Password contains invalid characters!") + "</font>");
        break;
    case 121:
        ui->loginWidget->setTextInfo("<font color=green>" + tr("Authentication was successful.") + "</font>");
        QMessageBox(QMessageBox::Information,
                    QObject::tr("Authentication"),
                    QObject::tr("Authentication user was successful!"),
                    QMessageBox::Ok).exec();
        m_currentLogin = userLogin;
        m_allConnectedUsers = allConnectedUsers;
        accept();
        this->close();
        break;
    case 122:
        ui->loginWidget->setTextInfo("<font color=red>" + tr("Authentication was failed.") + "</font>");
        break;
    case 123:
        ui->loginWidget->setTextInfo("<font color=red>" + tr("Invalid login! Try again, please.") + "</font>");
        break;
    case 124:
        ui->loginWidget->setTextInfo("<font color=red>" + tr("Invalid password! Try again, please.") + "</font>");
        break;
    case 125:
        ui->loginWidget->setTextInfo("<font color=red>" + tr("Authentication failed! You've used all attempts!") + "</font>");
        break;
    case 126:
        ui->loginWidget->setTextInfo("<font color=red>" + tr("You are already online.") + "</font>");
        break;
    default:
        break;
    }
}

void StartScreen::onLoggedIn(QString userLogin, QString userPassword)
{
    QString message;
    if(!userLogin.isEmpty() && !userPassword.isEmpty()) {
        ++cntAttemptsLogin;
        message = "120;" +  userLogin + ";" + userPassword + ";" + QString::number(cntAttemptsLogin) + ";";
        m_socket->write(message.toUtf8());
    }
    else {
        ui->loginWidget->setTextInfo("<font color=red>" + tr("All fields must be filled in.") + "</font>");
        return;
    }
}

void StartScreen::onRejectRequested()
{
    reject();
}

QTcpSocket *StartScreen::getSocket() const
{
    return m_socket;
}

QString StartScreen::getCurrentLogin() const
{
    return m_currentLogin;
}

QString StartScreen::getAllConnectedUsers() const
{
    return m_allConnectedUsers;
}

