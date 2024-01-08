#pragma once
#include <QObject>
#include <QtSql>
#include <QVector>
#include <QString>
#include <QMessageBox>
#include <string>
#include "sha1.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    explicit DataBase(QObject *parent = nullptr);
    ~DataBase();

    bool openDatabase();
    bool createTables();

    QSqlDatabase getDb() const;
    QVector<QString> getAllUsers();

    int addUser(QString name, QString login, QString password);
    int checkUserByLoginAndPassword(QString login, QString password);

    bool addMessageToAll(QString sender, QString text);
    bool addPrivateMessage(QString sender, QString recipient, QString text);
    QVector<QString> getMessagesBetweenTwoUsers(QString user1, QString user2, QString placeOfUsed = "Client");  

public slots:
    int setBan(const QString &login, bool value);
    int setLoginTime(const QString &login);

private:
    int getUserIdByLogin(QString login);
    int getUserIdByName(QString name);
    QString getUserById(QString id);
    int getHashPassword(const int userID) const;
    string getSalt(const int userID) const;

    bool isAlphaNumeric(const string &str);
    uint createHashPassword(const string& psw);
    string createSalt();

    int checkNewLogin(const QString& login);
    int checkNewPassword(const QString& psw);

private:
    const size_t minPswLen = 5; // minimum password length
    const size_t maxPswLen = 20; // maximum password length

    // Сам объект базы данных, с которым будет производиться работа
    QSqlDatabase m_db;
};

