#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator myAppTranslator;
    myAppTranslator.load("C:/Qt_projects/Chat_Client/translations/my_ru.qm");
    a.installTranslator(&myAppTranslator);

    auto w = MainWindow::createClient();
    if(w)
        w->show();
    else
        return 0;

    return a.exec();
}


