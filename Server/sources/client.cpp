#include "client.h"

Client::Client(QTcpSocket* socket, QObject *parent)
    : QObject{parent}
{
    m_socket = socket;
    m_isOnline = false;
    m_isBan = false;
}

Client::~Client()
{
    m_socket->deleteLater();
}

QTcpSocket* Client::getSocket() const
{
    return m_socket;
}

int Client::getPort() const
{
    return m_socket->peerPort();
}

QString Client::getLogin() const
{
    return m_clientLogin;
}

QString Client::getName() const
{
    return m_clientName;
}

void Client::setLogin(const QString &login)
{
    m_clientLogin = login;
}

void Client::setName(const QString &name)
{
    m_clientName = name;
}

void Client::setState(bool state)
{
    m_isOnline = state;
}

void Client::setBan(bool ban)
{
    m_isBan = ban;
}

bool Client::isOnline()
{
    return m_isOnline;
}

bool Client::isInBan()
{
    return m_isBan;
}



