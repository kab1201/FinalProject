#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <memory>
#include "startscreen.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString currentLogin,
                        QString allConnectedUsers,
                        QTcpSocket *socket= nullptr,
                        QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow* createClient(QTcpSocket *socket = nullptr);
    static int kInstanceCount;

    QTcpSocket *getSocket() const;
    QString getCurrentLogin() const;
    void setCurrentLogin(const QString& newCurrentLogin);

    void enterUserInChat(const QString& login, const QString& users);
    void banUser();
    void receiveMessagesBetweenTwoUsers(QString str);
    void receiveCommandMessageFromServer(QString cmd);

private slots:
    void disconnect();
    void readData();
    void on_messageLineEdit_returnPressed();
    void on_sendMessageButton_clicked();
    void on_actionOpenAnotherClient_triggered();
    void on_actionCloseThisClient_triggered();
    void on_messagesTextBrowser_cursorPositionChanged();

private:
    Ui::MainWindow *ui;
    QTcpSocket *m_socket;

    QString m_currentLogin;
    QString m_allConnectedUsers;
};

#endif // MAINWINDOW_H
