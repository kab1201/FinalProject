#pragma once
#include "database.h"
#include "client.h"

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QMap>
#include <QSqlQueryModel>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Client* getClientBySocket(QTcpSocket* socket);
    void removeClient(QTcpSocket* socket);
    void userIsOnline(QTcpSocket* socket, QString login);
    void sendMessageToAllButOne(QByteArray message, QTcpSocket* socket);
    void sendMessageToOne(QByteArray message, QTcpSocket* socket);
    void registration(QTcpSocket* socket, QString name, QString login, QString password);
    void login(QTcpSocket* socket, QString login, QString password, size_t cntAttemptsLogin);
    void logOut(QTcpSocket* socket);
    void sendMessageToAll(QTcpSocket* socket, QString text);
    void sendPrivateMessage(QTcpSocket* socket, QString reciever, QString text);
    void getChatBetweenTwoUsers(QTcpSocket* socket, QString user2);

private slots:
    void newConnect();
    void disconnect();
    void readData();
    void on_connectButton_clicked();
    void on_showMessagesButton_clicked();
    void on_disconnectUserButton_clicked();
    void on_banUserButton_clicked();
    void on_filterUsersComboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow  *ui;
    QTcpServer      *m_server;
    std::shared_ptr<DataBase> m_dbPtr = nullptr;
    Client          *m_client;
    QVector<Client*> m_allClients{};
    int m_clientsCount {0};
    int m_onlineUsersCount {0};
    int m_allUsersCount {0};

    QSqlQueryModel *m_modelUserComboBox;

    const size_t cntAttempts = 3; // count of attempts to login
};
