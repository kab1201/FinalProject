#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << QCoreApplication::applicationDirPath() + "/plugins";
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");

    QTranslator myappTranslator;
    myappTranslator.load("C:/Qt_projects/Chat_Server/translations/my_ru.qm");
    a.installTranslator(&myappTranslator);

    auto w = MainWindow();
    w.show();

    return a.exec();
}
