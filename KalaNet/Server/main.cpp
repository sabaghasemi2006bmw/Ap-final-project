#include "mainwindow.h"
#include "myserver.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MyServer server;
    server.startServer();

    MainWindow w;
    w.show();
    return a.exec();
}
