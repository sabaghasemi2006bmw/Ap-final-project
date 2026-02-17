#include "myserver.h"
#include "clienthandler.h"
#include <QDebug>

MyServer::MyServer(QObject *parent) : QTcpServer(parent) {
    db = new DatabaseManager(this);
}

void MyServer::startServer() {
    // Listen on all network interfaces, Port 1234
    if(!this->listen(QHostAddress::Any, 1234)) {
        qDebug() << "Could not start server";
    } else {
        qDebug() << "Listening on port 1234...";
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor) {
    qDebug() << "New Client Connected! ID:" << socketDescriptor;

    ClientHandler *thread = new ClientHandler(socketDescriptor, db, this);

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}
