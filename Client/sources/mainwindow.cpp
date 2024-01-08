#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTime>
#include <string>

int MainWindow::kInstanceCount = 0;

MainWindow::MainWindow(QString currentLogin,
                       QString allConnectedUsers,
                       QTcpSocket *socket,
                       QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_allConnectedUsers(allConnectedUsers),
    m_currentLogin(currentLogin)
{
    ui->setupUi(this);

    if(socket) {
        m_socket = socket;
    }
    else {
        m_socket = new QTcpSocket();
    }

    ui->currentUserLabel->setText("<font color=red>" + tr("Current user:") + "</font>");
    ui->onLineUsersComboBox->addItem("all");

    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainWindow::disconnect);

    enterUserInChat(currentLogin, allConnectedUsers);
    kInstanceCount++;

    auto * const usersComboBox = ui->onLineUsersComboBox;
    usersComboBox->setCurrentIndex(-1);
    connect(usersComboBox, &QComboBox::currentTextChanged, this, [this,usersComboBox](QString login){
        ui->messagesTextBrowser->clear();
        if(usersComboBox->currentIndex() != -1) {
            QString message = "141;" + login;
            m_socket->write(message.toUtf8());
        }
    });

    /*
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::readData);
    timer->start(1000);
    */
}

MainWindow::~MainWindow()
{
    delete m_socket;
    delete ui;
    kInstanceCount--;
    if(kInstanceCount <= 0)
        qApp->exit(0);
}

MainWindow *MainWindow::createClient(QTcpSocket *socket)
{
    StartScreen m_StartScreen(socket);
    auto result = m_StartScreen.exec();
    if(result ==QDialog::Rejected){
        return nullptr;
    }
    auto w = new MainWindow(m_StartScreen.getCurrentLogin(), m_StartScreen.getAllConnectedUsers(), m_StartScreen.getSocket());
    w->setAttribute(Qt::WA_DeleteOnClose);
    return w;
}

void MainWindow::on_messageLineEdit_returnPressed()
{
    on_sendMessageButton_clicked();
}

void MainWindow::on_sendMessageButton_clicked()
{
    if (ui->messageLineEdit->text().isEmpty()) {
        QMessageBox msgb;
        msgb.setText(tr("Can't Send Empty Message"));
        msgb.resize(60, 40);
        msgb.exec();
        return;
    }

    if(ui->onLineUsersComboBox->currentIndex() == -1) {
        QMessageBox msgb;
        msgb.setText(tr("You need to select a user!"));
        msgb.resize(60, 40);
        msgb.exec();
        return;
    }

    QString message;
    QString style = "<div style=\"margin-left:50px; color:";
    QString title, StyledText;
    if ((ui->messagesTextBrowser->toPlainText()).length()>0) {
        StyledText.append("<pre> </pre>");
    }
    if(ui->onLineUsersComboBox->currentIndex() == 0) {
        title = style + QString("black\">%1 ").arg(getCurrentLogin()) +
                tr("write to all:") + "</div>";
        StyledText.append(title);
        title = style + "grey\">%1</div>";
        StyledText.append(title.arg(ui->messageLineEdit->text()));
        auto textCursor = ui->messagesTextBrowser->textCursor();
        textCursor.movePosition(QTextCursor::End);
        ui->messagesTextBrowser->setTextCursor(textCursor);
        ui->messagesTextBrowser->insertHtml(StyledText);
        textCursor.movePosition(QTextCursor::End);
        ui->messagesTextBrowser->setTextCursor(textCursor);

        message = "130;" + ui->messageLineEdit->text();
        m_socket->write(message.toUtf8());
        m_socket->flush();
        ui->messageLineEdit->clear();
        ui->messageLineEdit->setFocus();
    }
    else {
        QString sender = getCurrentLogin();
        QString recipient = ui->onLineUsersComboBox->currentText();
        if(sender != recipient) {
            title = style + QString("green\">%1 ").arg(sender) +
                    tr("write to") +  QString(" %2:").arg(recipient) +
                    "</div>";
            StyledText.append(title);
            title = style + "grey\">%1</div>";
            StyledText.append(title.arg(ui->messageLineEdit->text()));
            auto textCursor = ui->messagesTextBrowser->textCursor();
            textCursor.movePosition(QTextCursor::End);
            ui->messagesTextBrowser->setTextCursor(textCursor);
            ui->messagesTextBrowser->insertHtml(StyledText);
            textCursor.movePosition(QTextCursor::End);
            ui->messagesTextBrowser->setTextCursor(textCursor);
        }
        message = "140;" + recipient + ";" + ui->messageLineEdit->text();
        m_socket->write(message.toUtf8());
        m_socket->flush();
        ui->messageLineEdit->clear();
        ui->messageLineEdit->setFocus();
    }
}

void MainWindow::on_actionOpenAnotherClient_triggered()
{
    auto w = createClient();
    if(w)
        w->show();
}

void MainWindow::on_actionCloseThisClient_triggered()
{
    setCurrentLogin("");

    m_socket->write("200");
    this->close();
}

void MainWindow::on_messagesTextBrowser_cursorPositionChanged()
{
    QList<QTextBrowser::ExtraSelection> extraSelections;

    QTextBrowser::ExtraSelection selection;

    QColor lineColor = QColor("#f0f0f0");

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    QTextCursor cursor = ui->messagesTextBrowser->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    selection.cursor = cursor;
    extraSelections.append(selection);

    ui->messagesTextBrowser->setExtraSelections(extraSelections);
}

QTcpSocket *MainWindow::getSocket() const
{
    return m_socket;
}

QString MainWindow::getCurrentLogin() const
{
    return m_currentLogin;
}

void MainWindow::setCurrentLogin(const QString &newCurrentLogin)
{
    m_currentLogin = newCurrentLogin;
}

void MainWindow::enterUserInChat(const QString &login, const QString &users)
{
    m_currentLogin = login;
    ui->currentUserLabel->setText("<font color=green>" + tr("Current user: ") + QString("%1").arg(login) + "</font>");
    ui->messageLabel->setEnabled(true);
    ui->messageLineEdit->setEnabled(true);
    ui->sendMessageButton->setEnabled(true);
     ui->messagesTextBrowser->setEnabled(true);
    QStringList lst = users.split(",");
    ui->onLineUsersComboBox->clear();
    ui->onLineUsersComboBox->setCurrentIndex(-1);
    ui->onLineUsersComboBox->addItem("all");
    for(const auto& user : lst)
        ui->onLineUsersComboBox->addItem(user);
    ui->onLineUsersComboBox->setEnabled(true);
}

void MainWindow::banUser()
{
    ui->currentUserLabel->setText("<font color=red>" + tr("Current user: ") + QString("%1").arg(getCurrentLogin()) + tr(" (in ban)") + "</font>");
    ui->messageLabel->setEnabled(false);
    ui->messageLineEdit->setEnabled(false);
    ui->sendMessageButton->setEnabled(false);
    ui->messagesTextBrowser->clear();
    ui->messagesTextBrowser->setEnabled(false);
    ui->onLineUsersComboBox->clear();
    ui->onLineUsersComboBox->setEnabled(false);
}

void MainWindow::receiveMessagesBetweenTwoUsers(QString str)
{
    if(ui->onLineUsersComboBox->currentIndex() != -1) {
        if(!str.isEmpty()) {
            QStringList messages = str.split("***");
            QString user1, user2, text;
            QString style, color, title, styledText;
            for(const auto& message : messages) {
                user1 = message.section(';',0,0);
                user2 = message.section(';',1,1);
                text = message.section(';',2,2);
                if (user1 == getCurrentLogin()) {
                    style = "<div style=\"margin-left:50px; color:";
                    color = "green";
                }
                else {
                    style = "<div style=\"color:";
                    color = "blue";
                }
                if(user2 == "all") {
                    title = style + QString("black\">%1 ").arg(user1) +
                            tr("write to all:") + "</div>";
                    styledText.append(title);
                }
                else {
                    title = style + color + QString("\">%1 ").arg(user1) +
                            tr("write to") + QString(" %1:").arg(user2) +
                            "</div>";
                    styledText.append(title);
                }
                title = style + "grey\">%1</div>";
                styledText.append(title.arg(text));
            }
            ui->messagesTextBrowser->setHtml(styledText);
            auto textCursor = ui->messagesTextBrowser->textCursor();
            textCursor.movePosition(QTextCursor::End);
            ui->messagesTextBrowser->setTextCursor(textCursor);
        }
    }
}

void MainWindow::receiveCommandMessageFromServer(QString cmd)
{
    switch(cmd.toInt()) {
    case 1:
        QMessageBox(QMessageBox::Information,
                    QObject::tr("Logout user"),
                    QObject::tr("You have logged out of the chat"),
                    QMessageBox::Ok).exec();
        break;
    case 2:
        QMessageBox(QMessageBox::Critical,
                    QObject::tr("Error"),
                    QObject::tr("Error send message"),
                    QMessageBox::Ok).exec();
        break;
    default:
        break;
    }
}

void MainWindow::disconnect()
{
    ui->sendMessageButton->setEnabled(false);

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("CAUTION"));
    msgBox.setText(tr("You are dissconnect from server!"));
    msgBox.resize(60,30);
    msgBox.exec();

    ui->messageLabel->setEnabled(false);
    ui->messageLineEdit->setEnabled(false);
    ui->messagesTextBrowser->clear();
    ui->messagesTextBrowser->setEnabled(false);
    ui->onLineUsersComboBox->clear();
    ui->onLineUsersComboBox->setEnabled(false);
    ui->sendMessageButton->setEnabled(false);
    ui->currentUserLabel->setText("<font color=red>" + tr("Current user:") + "</font>");
}

/* COMMANDS FROM SERVER
100 - new user join to chat
111 - registration (user not addet in database)
112 - registration (user add in database)
113 - registration (user is already exists)
121 - login (Authentication success)
122 - login (Authentication failed)
123 - login (user already online)
131 - send message to all online clients
141 - private message
142 - receive messages between two users
200 - user left chat
300 - current user in ban
301 - user in ban
302 - current user unban
303 - user unban
500 - command messages*/

void MainWindow::readData()
{
    QByteArray message;
    message = m_socket->readAll();
    QString str = QString(message);
    QString cmd = str.section(';',0,0);
    QString currentLogin;
    int ind;
    QString style = "<div style=\"margin-left:0px; color:";
    QString title, StyledText;
    QString logMessage;

    switch (cmd.toInt()) {
    case 100:
        currentLogin = str.section(';',1,1);
        logMessage = "<font color=green>" +
                     QString("%1 ").arg(QTime::currentTime().toString()) + QString("User %1 ").arg(currentLogin) +
                     tr("join to Chat!") +
                     "</font>";
        ui->informationTextBrowser->append(logMessage);
        ind = ui->onLineUsersComboBox->findText(currentLogin, Qt::MatchExactly);
        if(ind == -1) {
            ui->onLineUsersComboBox->addItem(currentLogin);
        }
        break;
    case 131:
    {
        if ((ui->messagesTextBrowser->toPlainText()).length()>0) {
            StyledText.append("<pre> </pre>");
        }
        title = style + QString("black\">%1 ").arg(str.section(';',1,1)) +
                tr("write to all:") +
                "</div>";
        StyledText.append(title);
        title = style + "grey\">%1</div>";
        StyledText.append(title.arg(str.section(';',2,2)));
        auto textCursor = ui->messagesTextBrowser->textCursor();
        textCursor.movePosition(QTextCursor::End);
        ui->messagesTextBrowser->setTextCursor(textCursor);
        ui->messagesTextBrowser->insertHtml(StyledText);
        textCursor.movePosition(QTextCursor::End);
        ui->messagesTextBrowser->setTextCursor(textCursor);
        break;
    }
    case 141:
    {
        QString recepient = str.section(';',1,1);
        currentLogin = getCurrentLogin();
        if(ui->onLineUsersComboBox->currentText() == recepient) {
            if ((ui->messagesTextBrowser->toPlainText()).length()>0) {
                StyledText.append("<pre> </pre>");
            }
            title = style + QString("blue\">%1 ").arg(recepient) +
                    tr("write to") +  QString(" %1:").arg(currentLogin) +
                    "</div>";
            StyledText.append(title);
            title = style + "grey\">%1</div>";
            StyledText.append(title.arg(str.section(';',2,2)));
            auto textCursor = ui->messagesTextBrowser->textCursor();
            textCursor.movePosition(QTextCursor::End);
            ui->messagesTextBrowser->setTextCursor(textCursor);
            ui->messagesTextBrowser->insertHtml(StyledText);
            textCursor.movePosition(QTextCursor::End);
            ui->messagesTextBrowser->setTextCursor(textCursor);
        }
        break;
    }
    case 142:
        receiveMessagesBetweenTwoUsers(str.section(';',1));
        break;
    case 200:
    {
        currentLogin = str.section(';',1,1);
        logMessage = "<font color=red>" +
                     QString("%1 ").arg(QTime::currentTime().toString()) + tr("User with login ") + QString("%1 ").arg(currentLogin) +
                     tr("left the Chat!") +
                     "</font>";
        ui->informationTextBrowser->append(logMessage);

        ind = ui->onLineUsersComboBox->findText(currentLogin, Qt::MatchExactly);
        if(ind != -1) {
            ui->onLineUsersComboBox->removeItem(ind);
        }
        break;
    }
    case 300:
        logMessage = "<font color=red>" +
                     QString("%1 ").arg(QTime::currentTime().toString()) +
                     tr("You are in BAN!") +
                     "</font>";
        ui->informationTextBrowser->append(logMessage);
        banUser();
        break;
    case 301:
        logMessage = "<font color=red>" +
                     QString("%1 ").arg(QTime::currentTime().toString()) +
                     tr("User with login ") +
                     QString("%1 ").arg(str.section(';',1)) +
                     tr("sent to ban!") +
                     "</font>";
        ui->informationTextBrowser->append(logMessage);
        break;
    case 302:
        logMessage = "<font color=green>" +
                     QString("%1 ").arg(QTime::currentTime().toString()) +
                     tr("You are UNBAN!") +
                     "</font>";
        ui->informationTextBrowser->append(logMessage);
        enterUserInChat(str.section(';',1,1), str.section(';',2,2));
        break;
    case 303:
        logMessage = "<font color=green>" +
                     QString("%1 ").arg(QTime::currentTime().toString()) +
                     tr("User with login ") +
                     QString("%1 ").arg(str.section(';',1)) +
                     tr("Unban!") +
                     "</font>";
        ui->informationTextBrowser->append(logMessage);
        break;
    case 500:
        receiveCommandMessageFromServer(str.section(';',1,1));
        break;
    default:
        break;
    }
}


