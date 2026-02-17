#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include "databasemanager.h"

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = nullptr);
    void startServer();

protected:
    // This is called automatically when a new client connects
    void incomingConnection(qintptr socketDescriptor) override;

private:
    DatabaseManager *db;
};

#endif // MYSERVER_H
