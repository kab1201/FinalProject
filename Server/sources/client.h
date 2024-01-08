#pragma once

#include "database.h"
#include <QObject>
#include <QTcpSocket>
#include <QString>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QTcpSocket* socket, QObject *parent = nullptr);
    ~Client();

    QTcpSocket* getSocket() const;
    int getPort() const;
    QString getLogin() const;
    QString getName() const;

    void setLogin(const QString &login);
    void setName(const QString &name);
    void setState(bool state);
    void setBan(bool ban);

    bool isOnline();
    bool isInBan();

private:
    QTcpSocket* m_socket;
    QString m_clientLogin {};
    QString m_clientName {};
    bool m_isOnline {};
    bool m_isBan {};

signals:

};

