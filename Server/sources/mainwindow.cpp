#include "mainwindow.h"
#include "./ui_mainwindow.h"
//#include "checkboxdelegate.h"

#include <QMessageBox>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Chat Server");
    ui->portLineEdit->setText("5555");

    m_server = new QTcpServer();
    m_server->setMaxPendingConnections(10);

    ui->banUserButton->setEnabled(false);
    ui->disconnectUserButton->setEnabled(false);
    ui->userFromComboBox->setEnabled(false);
    ui->userToComboBox->setEnabled(false);
    ui->showMessagesButton->setEnabled(false);
    ui->filterUsersComboBox->setEnabled(false);

    /* Создаем объект, который будет использоваться для работы с БД
     * и инициализируем подключение к БД
     * */
    m_dbPtr = std::make_shared<DataBase>();
    bool result = m_dbPtr->openDatabase();
    QString logMessage;

    if(result)
    {
        logMessage = QString("<font color=darkGreen>%1 ").arg(QTime::currentTime().toString()) +
                     tr("The Connection to the database was successful!") + "</font>";
        ui->logsTextEdit->append(logMessage);
        if(m_dbPtr->createTables()) {
            logMessage = QString("<font color=darkGreen>%1 ").arg(QTime::currentTime().toString()) +
                         tr("Database Tables successfully found or created.") + "</font>";
            ui->logsTextEdit->append(logMessage);
        }
        else {
            ui->connectButton->setEnabled(false);
            logMessage = QString("<font color=darkRed>%1 ").arg(QTime::currentTime().toString()) +
                         tr("The search or creation of database tables failed.") + "</font>";
            ui->logsTextEdit->append(logMessage);
        }
    }
    else {
        QMessageBox(QMessageBox::Information,
                    QObject::tr("Error"),
                    QObject::tr("Connection with database failed!"),
                    QMessageBox::Ok).exec();
        ui->connectButton->setEnabled(false);
        logMessage = QString("<font color=darkRed>%1 ").arg(QTime::currentTime().toString()) +
                     tr("The Connection to the database failed!") + "</font>";
        ui->logsTextEdit->append(logMessage);
    }

    connect(m_server,&QTcpServer::newConnection,this,&MainWindow::newConnect);
    connect(ui->shutDownButton, &QPushButton::clicked, this, &MainWindow::close);

    connect(ui->usersListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        if(ui->filterUsersComboBox->currentIndex() == 1) {
            ui->banUserButton->setEnabled(true);
            ui->disconnectUserButton->setEnabled(true);
            for(Client* user : m_allClients) {
                if((user->getName() == item->text()) && user->isInBan()) {
                    ui->banUserButton->setText(tr("Unban user"));
                    break;
                }
                else {
                    ui->banUserButton->setText(tr("Ban user"));
                }
            }
        }
        else if(ui->filterUsersComboBox->currentIndex() == 2) {
            ui->banUserButton->setEnabled(true);
            ui->disconnectUserButton->setEnabled(false);
            ui->banUserButton->setText(tr("Unban user"));
        }
        else {
            ui->banUserButton->setEnabled(false);
            ui->disconnectUserButton->setEnabled(false);
        }
    });
}

MainWindow::~MainWindow()
{
    m_server->close();
    m_server->deleteLater();
    delete ui;
}

Client *MainWindow::getClientBySocket(QTcpSocket *socket)
{
    for(Client* client : m_allClients) {
        QTcpSocket* clientSocket = client->getSocket();
        if(socket == clientSocket) {
            return client;
        }
    }
    return nullptr;
}

void MainWindow::removeClient(QTcpSocket *socket)
{
    m_allClients.remove(m_allClients.indexOf(getClientBySocket(socket)));
}

void MainWindow::sendMessageToAllButOne(QByteArray message, QTcpSocket *socket)
{
    for (int i = 0; i < m_allClients.size(); ++i) {
        QTcpSocket* clientSocket = m_allClients.at(i)->getSocket();
        if(socket != clientSocket && m_allClients.at(i)->isOnline()) {
            clientSocket->write(message);
        }
    }
}

void MainWindow::sendMessageToOne(QByteArray message, QTcpSocket *socket)
{
    for (int i = 0; i < m_allClients.size(); ++i) {
        QTcpSocket* clientSocket = m_allClients.at(i)->getSocket();
        if(socket == clientSocket) {
            clientSocket->write(message);
        }
    }
}

void MainWindow::registration(QTcpSocket* socket, QString name, QString login, QString password)
{

    QString message;
    int res = m_dbPtr->addUser(name, login, password);
    switch (res) {
    case 0:
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' not added in database! Registration failed...");
        ui->logsTextEdit->append(message);
        socket->write("111");
        break;
    case 1:
    {
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' add in database! Registration success...");
        getClientBySocket(socket)->setState(true);
        getClientBySocket(socket)->setName(login);
        m_onlineUsersCount++;
        ui->logsTextEdit->append(message);

        message = "112;" + login + ";";
        for(Client* user : m_allClients) {
            if(user->isOnline())
                message += user->getName() + ",";
        }
        message.resize(message.size()-1);
        socket->write(message.toUtf8());

        m_modelUserComboBox->setQuery("SELECT login FROM users ORDER BY CASE WHEN id=0 THEN -100 ELSE 0 END, login", m_dbPtr->getDb());
        ui->userFromComboBox->setModel(m_modelUserComboBox);
        ui->userToComboBox->setModel(m_modelUserComboBox);

        message = "100;" + login + ";";
        sendMessageToAllButOne(message.toUtf8(), socket);

        if(ui->filterUsersComboBox->currentIndex() != 2) {
            on_filterUsersComboBox_currentIndexChanged(ui->filterUsersComboBox->currentIndex());
        }
        break;
    }
    case 2:
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' is already exists");
        ui->logsTextEdit->append(message);
        message = "113;" + login + ";";
        socket->write(message.toUtf8());
        break;
    case 3:
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' not added in database! The login must contain letters and numbers only!");
        ui->logsTextEdit->append(message);
        message = "114;" + login + ";";
        socket->write(message.toUtf8());
        break;
    case 4:
        message = QTime::currentTime().toString() + tr(" Login 'all' is reserved!");
        ui->logsTextEdit->append(message);
        socket->write("115");
        break;
    case 5:
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' not added in database! Password must be equal to or more than 5 characters long!");
        ui->logsTextEdit->append(message);
        message = "116;" + login + ";";
        socket->write(message.toUtf8());
        break;
    case 6:
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' not added in database! Password must not exceed 20 characters long!");
        ui->logsTextEdit->append(message);
        message = "117;" + login + ";";
        socket->write(message.toUtf8());
        break;
    case 7:
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' not added in database! Password contains invalid characters!");
        ui->logsTextEdit->append(message);
        message = "118;" + login + ";";
        socket->write(message.toUtf8());
        break;  
    default:
        break;
    }
}

void MainWindow::login(QTcpSocket *socket, QString login, QString password, size_t cntAttemptsLogin)
{
    QString message;
    int res = 0;

    // User sign in attempt
    if (cntAttemptsLogin > cntAttempts) {
        message = QTime::currentTime().toString() + tr(" User with login '") + login + tr("' have made ") + QString::number(cntAttempts) + tr(" attempts to login!");
        ui->logsTextEdit->append(message);
        message = "125;" + login + ";";
        socket->write(message.toUtf8());
        return;
    }

    res = m_dbPtr->checkUserByLoginAndPassword(login, password);
    switch (res) {
    case 1:
        message = QTime::currentTime().toString() + tr(" Invalid login!");
        ui->logsTextEdit->append(message);
        socket->write("123");
        return;
    case 2:
        message = QTime::currentTime().toString() + tr(" Invalid password for User with login '") + login + "'!";
        ui->logsTextEdit->append(message);
        message = "124;" + login + ";";
        socket->write(message.toUtf8());
        return;
    default:
        break;
    }

    bool isAlreadyOnline = false;
    for(Client* user : m_allClients) {
        if((user->getName() == login) && user->isOnline()) {
            isAlreadyOnline = true;
        }
    }
    if(res == 0 && !isAlreadyOnline) {
        m_dbPtr->setLoginTime(login);
        message = QTime::currentTime().toString() + tr(" The user with login '") + login + tr("' was authenticated");
        getClientBySocket(socket)->setState(true);
        getClientBySocket(socket)->setName(login);
        m_onlineUsersCount++;
        ui->logsTextEdit->append(message);
        if(ui->filterUsersComboBox->currentIndex() == 1)
            ui->usersListWidget->addItem(login);

        message = "121;" + login + ";";
        for(Client* user : m_allClients) {
            if(user->isOnline())
                message += user->getName() + ",";
        }
        message.resize(message.size()-1);
        socket->write(message.toUtf8());

        message = "100;" + login  + ";";
        sendMessageToAllButOne(message.toUtf8(), socket);
    }
    else if(isAlreadyOnline) {
        message = QTime::currentTime().toString() + tr(" The user with login '") + login + tr("' is already online");
        ui->logsTextEdit->append(message);
        message = "126;" + login + ";";
        socket->write(message.toUtf8());
    }
    else {
        message = QTime::currentTime().toString() + tr(" The user with login '") + login + tr("' was not authenticated");
        ui->logsTextEdit->append(message);
        message = "122;" + login + ";";
        socket->write(message.toUtf8());
    }
}

void MainWindow::logOut(QTcpSocket *socket)
{
    QString logMessage = QString("<font color=darkRed>%1 ").arg(QTime::currentTime().toString()) +
                         tr("The user with login ") +
                         QString("%1 ").arg(getClientBySocket(socket)->getName()) +
                         "logged out the chat</font>";
    ui->logsTextEdit->append(logMessage);
    auto userName = getClientBySocket(socket)->getName();
    if(ui->filterUsersComboBox->currentIndex() == 1) {
        QListWidgetItem* currentItem = ui->usersListWidget->findItems(userName, Qt::MatchExactly)[0];
        delete ui->usersListWidget->takeItem(ui->usersListWidget->row(currentItem));
    }
    QString message = "200;" + userName + ";";
    sendMessageToAllButOne(message.toUtf8(), socket);
    socket->write("500;1");
    getClientBySocket(socket)->setName("");
    getClientBySocket(socket)->setState(false);
}

void MainWindow::sendMessageToAll(QTcpSocket *socket, QString text)
{
    QString message;
    bool res = m_dbPtr->addMessageToAll(getClientBySocket(socket)->getName(), text);
    if(res) {
        for (int i = 0; i < m_allClients.size(); ++i) {
            QTcpSocket* onlineClientSocket = m_allClients.at(i)->getSocket();
            if(socket != onlineClientSocket && m_allClients.at(i)->isOnline()) {
                message = "131;" + getClientBySocket(socket)->getName() + ";" + text;
                onlineClientSocket->write(message.toUtf8());
            }
        }
    }
    else {
        socket->write("500;2");
    }
}

void MainWindow::sendPrivateMessage(QTcpSocket *socket, QString recipient, QString text)
{
    QString message;
    bool res = m_dbPtr->addPrivateMessage(getClientBySocket(socket)->getName(), recipient, text);
    if(res) {
        for (Client* client : m_allClients) {
            if(client->getName() == recipient) {
                message = "141;" + getClientBySocket(socket)->getName() + ";" + text;
                client->getSocket()->write(message.toUtf8());
            }
        }
    }
    else {
        socket->write("500;2");
    }
}

void MainWindow::getChatBetweenTwoUsers(QTcpSocket *socket, QString user2)
{
    QVector<QString> messages = m_dbPtr->getMessagesBetweenTwoUsers(getClientBySocket(socket)->getName(), user2);
    QString str;
    if(messages.size()>0) {
        for(const auto& mes : messages) {
            str += mes + "***";
        }
        str.resize(str.size()-3);
        QString message = "142;" + str;
        socket->write(message.toUtf8());
        socket->flush();
    }
    else
        socket->write("142;");
}

void MainWindow::newConnect()
{
    QTcpSocket* clientSocket = m_server->nextPendingConnection();
    m_client = new Client(clientSocket);
    int port = m_client->getPort();

    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::readData);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MainWindow::disconnect);

    m_allClients.push_back(m_client);
    QString logMessage = QString("<font color=darkGreen>%1 ").arg(QTime::currentTime().toString()) +
                         tr("New client connect from port number ") +
                         QString("%1 ").arg(QString::number(port)) +
                         "</font>";
    ui->logsTextEdit->append(logMessage);
}

void MainWindow::disconnect()
{
    QTcpSocket* currentSocket = qobject_cast<QTcpSocket*>(sender());
    for(Client* client : m_allClients) {
        QTcpSocket* clientSocket = client->getSocket();
        if(currentSocket == clientSocket) {
            if(!client->getName().isEmpty()) {
                QListWidgetItem* currentItem = ui->usersListWidget->findItems(client->getName(), Qt::MatchExactly)[0];
                if((client->isOnline() && ui->filterUsersComboBox->currentIndex() == 1) or
                    (client->isInBan() && ui->filterUsersComboBox->currentIndex() == 2)) {
                    delete ui->usersListWidget->takeItem(ui->usersListWidget->row(currentItem));
                }
                QString message = "200;" + getClientBySocket(currentSocket)->getName();
                sendMessageToAllButOne(message.toUtf8(), currentSocket);
            }
            removeClient(currentSocket);
            currentSocket->deleteLater();
            QString logMessage = QString("<font color=darkRed>%1 ").arg(QTime::currentTime().toString()) +
                                 tr("Client from port number ") +
                                 QString("%1 ").arg(client->getPort()) +
                                 "disconnect.</font>";
            ui->logsTextEdit->append(logMessage);
            break;
        }
    }
}

/* COMMANDS FROM CLIENTS
110 - registration
120 - login
130 - message to all online clients
140 - send private message
141 - get chat between two users
200 - logout user*/

void MainWindow::readData()
{
    QByteArray message;
    QTcpSocket* currentSocket = qobject_cast<QTcpSocket*>(sender());
    message =  currentSocket->readAll();
    QString str = QString(message);
    QString cmd = str.section(';',0,0);
    QString logMessage;

    switch (cmd.toInt()) {
    case 110:
        logMessage = QString("%1 ").arg(QTime::currentTime().toString()) + tr("Requested command from client: Registration.");
        ui->logsTextEdit->append(logMessage);
        registration(currentSocket, str.section(';',1,1), str.section(';',2,2), str.section(';',3,3));
        break;
    case 120:
        logMessage = QString("%1 ").arg(QTime::currentTime().toString()) + tr("Requested command from client: Login.");
        ui->logsTextEdit->append(logMessage);
        login(currentSocket, str.section(';',1,1), str.section(';',2,2), str.section(';',3,3).toUInt());
        break;
    case 130:
        logMessage = QString("%1 ").arg(QTime::currentTime().toString()) + tr("Requested command from client: Send message to all.");
        ui->logsTextEdit->append(logMessage);
        logMessage = QString("%1 %2 ").arg(QTime::currentTime().toString()).arg(getClientBySocket(currentSocket)->getName()) + tr("write:");
        ui->logsTextEdit->append(logMessage);
        ui->logsTextEdit->append(QString("<font color=dimGray>%1</font>").arg(str.section(';',1)));
        sendMessageToAll(currentSocket, str.section(';',1));
        break;
    case 140:
        logMessage = QString("%1 ").arg(QTime::currentTime().toString()) + tr("Requested command from client: Send private message.");
        ui->logsTextEdit->append(logMessage);
        sendPrivateMessage(currentSocket, str.section(';',1,1), str.section(';',2));
        break;
    case 141:
        logMessage = QString("%1 ").arg(QTime::currentTime().toString()) + tr("Requested command from client: Get chat between two users.");
        ui->logsTextEdit->append(logMessage);
        getChatBetweenTwoUsers(currentSocket,str.section(';',1));
        break;
    case 200:
        logMessage = QString("%1 ").arg(QTime::currentTime().toString()) + tr("Requested command from client: Logout.");
        ui->logsTextEdit->append(logMessage);
        logOut(currentSocket);
        break;
    default:
        break;
    }
}

void MainWindow::on_connectButton_clicked()
{
    if(ui->connectButton->text() == QString(tr("Connect"))) {
        qint16 port = ui->portLineEdit->text().toInt();

        if(!m_server->listen(QHostAddress::Any, port))
        {
            QMessageBox(QMessageBox::Critical,
                        QObject::tr("Error"),
                        m_server->errorString(),
                        QMessageBox::Ok).exec();
            return;
        }
        ui->connectButton->setText(tr("Disconnect"));
        ui->portLineEdit->setEnabled(false);
        ui->userFromComboBox->setEnabled(true);
        ui->userToComboBox->setEnabled(true);
        ui->showMessagesButton->setEnabled(true);
        ui->filterUsersComboBox->setEnabled(true);

        // Заполняем ComboBox-ы
        // Добавление вариантов в filterUsersComboBox
        ui->filterUsersComboBox->addItem("All");
        ui->filterUsersComboBox->addItem("OnLine");
        ui->filterUsersComboBox->addItem("Baned");
        // Установка начального выбранного варианта и заполнение usersListWidget данными о пользователях, кроме 'all'
        ui->filterUsersComboBox->setCurrentIndex(0);

        // Заполняем ComboBox-ы данными о пользователях
        m_modelUserComboBox = new QSqlQueryModel(this);
        m_modelUserComboBox->setQuery("SELECT login FROM users ORDER BY CASE WHEN id=0 THEN -100 ELSE 0 END, login", m_dbPtr->getDb());
        ui->userFromComboBox->setModel(m_modelUserComboBox);
        ui->userToComboBox->setModel(m_modelUserComboBox);

        on_showMessagesButton_clicked();
    }
    else {
        for(Client* client : m_allClients) {
            if(client->getSocket()->state() == QAbstractSocket::ConnectedState)
            {
                client->getSocket()->disconnectFromHost();
            }
        }
        m_server->close();
        ui->connectButton->setText(tr("Connect"));
        ui->portLineEdit->setEnabled(true);
        ui->userFromComboBox->setEnabled(false);
        ui->userToComboBox->setEnabled(false);
        ui->showMessagesButton->setEnabled(false);
        ui->filterUsersComboBox->setEnabled(false);
        ui->filterUsersComboBox->clear();
        ui->usersListWidget->clear();
        ui->userFromComboBox->clear();
        ui->userToComboBox->clear();
        ui->messagesTextEdit->clear();
        ui->logsTextEdit->clear();
    }
}

void MainWindow::on_showMessagesButton_clicked()
{
    ui->messagesTextEdit->clear();
    QString sender, recipient, message, styledText;
    QString user1 = ui->userFromComboBox->currentText();
    QString user2 = ui->userToComboBox->currentText();
    QVector<QString> messages = m_dbPtr->getMessagesBetweenTwoUsers(user1,user2,"Server");
    if(messages.isEmpty()) {
        ui->messagesTextEdit->append(tr("No messages between users!"));
    }
    else {
        QString style, color, title;
        style = "<div style=\"color:";
        color = "blue";
        if (user1 == "all" or user2 == "all") {
            for(const auto& msg : messages) {
                sender = msg.section(';',0,0);
                recipient = msg.section(';',1,1);
                message = msg.section(';',2);

                if(recipient != "all")
                    color = "blue";
                else
                    color = "black";
                title = style + color + QString("\">%1 ").arg(sender) +
                        tr("write to") + QString(" %1:").arg(recipient) +
                        "</div>";
                styledText.append(title);
                title = style + "grey\">%1</div>";
                styledText.append(title.arg(message));
            }
        }
        else {
            for(const auto& msg : messages) {
                sender = msg.section(';',0,0);
                recipient = msg.section(';',1,1);
                message = msg.section(';',2);

                if (user1 == sender) {
                    style = "<div style=\"margin-left:50px; color:";
                    color = "green";
                }
                else {
                    style = "<div style=\"color:";
                    color = "blue";
                }

                if(user2 == "all") {
                    title = style + QString("black\">%1 ").arg(sender) +
                            tr("write to") + " all:</div>";
                    styledText.append(title);
                }
                else {
                    title = style + color + QString("\">%1 ").arg(sender) +
                            tr("write to") + QString(" %1:").arg(recipient) +
                            "</div>";
                    styledText.append(title);
                }
                title = style + "grey\">%1</div>";
                styledText.append(title.arg(message));
            }
        }
        ui->messagesTextEdit->setHtml(styledText);
    }
}

void MainWindow::on_filterUsersComboBox_currentIndexChanged(int index)
{
    ui->usersListWidget->clear();

    switch (index)
    {
    case 0:
    {
        QVector<QString> users = m_dbPtr->getAllUsers();
        for(auto& user : users) {
            ui->usersListWidget->addItem(user);
        }
        ui->banUserButton->setEnabled(false);
        ui->disconnectUserButton->setEnabled(false);
        ui->banUserButton->setText(tr("Ban user"));
        break;
    }
    case 1:
    {
        for(Client* user : m_allClients) {
            if(user->isOnline())
                ui->usersListWidget->addItem(user->getName());
        }
        break;
    }
    case 2:
    {
        for(Client* user : m_allClients) {
            if(user->isInBan())
                ui->usersListWidget->addItem(user->getName());
        }
        ui->banUserButton->setEnabled(false);
        ui->disconnectUserButton->setEnabled(false);
        break;
    }
    default:
        break;
    }
}

void MainWindow::on_banUserButton_clicked()
{
    QListWidgetItem* item = ui->usersListWidget->currentItem();
    QString user = item->text();
    QString logMessage;
    QString message;

    for(Client* client : m_allClients) {
        if(client->getName() == user) {
            if(ui->banUserButton->text() == QString(tr("Ban user"))) {
                ui->banUserButton->setText(tr("Unban user"));
                client->setBan(true);
                message = "300;";
                sendMessageToOne(message.toUtf8(), client->getSocket());

                message = "301;" + client->getName();
                sendMessageToAllButOne(message.toUtf8(), client->getSocket());
                logMessage = QString("<font color=darkRed>%1 ").arg(QTime::currentTime().toString()) +
                             tr("Client with login ") +
                             QString("%1 ").arg(client->getName()) +
                             tr("sent to ban.") +
                             "</font>";
                ui->logsTextEdit->append(logMessage);
            }
            else {
                ui->banUserButton->setText(tr("Ban user"));
                client->setBan(false);

                logMessage = QString("<font color=darkGreen>%1 ").arg(QTime::currentTime().toString()) +
                             tr("Client with login ") +
                             QString("%1 ").arg(client->getName()) +
                             tr("unban.") + "</font>";
                ui->logsTextEdit->append(logMessage);

                message = "302;" + client->getName() + ";";
                for(Client* user1 : m_allClients) {
                    if(user1->isOnline())
                        message += user1->getName() + ",";
                }
                message.resize(message.size()-1);
                client->getSocket()->write(message.toUtf8());

                message = "303;" + client->getName();
                sendMessageToAllButOne(message.toUtf8(), client->getSocket());
                if(ui->filterUsersComboBox->currentIndex() == 2) {
                    QListWidgetItem* currentItem = ui->usersListWidget->findItems(user, Qt::MatchExactly)[0];
                    delete ui->usersListWidget->takeItem(ui->usersListWidget->row(currentItem));
                }
            }
            break;
        }
    }
}

void MainWindow::on_disconnectUserButton_clicked()
{
    QListWidgetItem* item = ui->usersListWidget->currentItem();
    QString user = item->text();
    for(Client* client : m_allClients) {
        if((client->getName() == user) && client->isOnline()) {
            QString message = "200;" + client->getName();
            sendMessageToAllButOne(message.toUtf8(), client->getSocket());
            client->getSocket()->deleteLater();
            removeClient(client->getSocket());
            QListWidgetItem* currentItem = ui->usersListWidget->findItems(user, Qt::MatchExactly)[0];
            delete ui->usersListWidget->takeItem(ui->usersListWidget->row(currentItem));

            QString logMessage = QString("<font color=darkRed>%1 ").arg(QTime::currentTime().toString()) +
                         tr("Client from port number ") +
                         QString("%1 ").arg(client->getPort()) +
                         tr("disconnect.") + "</font>";
            ui->logsTextEdit->append(logMessage);
            break;
        }
    }
}


