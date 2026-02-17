#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QThread>
#include <QTcpSocket>
#include "databasemanager.h"
#include "enums.h"

class ClientHandler : public QThread
{
    Q_OBJECT
public:
    explicit ClientHandler(qintptr socketDescriptor, DatabaseManager *db, QObject *parent = nullptr);
    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);

public slots:
    void onReadyRead();
    void onDisconnected();

private:
    qintptr socketDescriptor;
    QTcpSocket *socket;
    DatabaseManager *db;

    void sendJson(const QJsonObject &json);

    void handleCreateAd(QDataStream &in);
    void handleGetAds(QDataStream &in);
    void sendResponse(RequestType type, bool success, const QString &message = "", const QJsonObject &data = QJsonObject());


    void handleLogin(QDataStream &in);
    void handleRegister(QDataStream &in);
    void handleUpdateProfile(QDataStream &in);
    void handleBuyItem(QDataStream &in);
    void handleGetCaptcha(QDataStream &in);
    void handleChargeWallet(QDataStream &in);
    QString sessionCaptcha;

    qint32 blockSize = 0;
};

#endif
