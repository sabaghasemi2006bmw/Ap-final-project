#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonObject>
#include "user.h"
#include "ad.h"
#include "message.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    static NetworkManager& instance(); // Singleton Access
    void connectToServer();

    void sendLogin(QString username, QString passwordHash);
    void sendRegister(User user);
    void sendCreateAd(const Ad &ad);
    void sendGetAds();
    void sendUpdateProfile(const User &user);
    void sendBuyItem(QString adId, QString username, long long price);
    void sendGetCaptcha();
    void sendChargeWallet(QString username, QString cardNum, long long amount, QString captcha);
    void sendGlobalMessage(QString sender, QString text);
    void getGlobalChat();

signals:
    // Signals to update UI when Server replies
    void loginResponse(bool success, QString message, User userData);
    void registerResponse(bool success, QString message, User user);
    void connected();
    void disconnected();

    void createAdResponse(bool success, QString message);
    void getAdsResponse(bool success, QString message, QVector<Ad> ads);
    void updateProfileResponse(bool success, QString message);

    void captchaReceived(QString code); // Signal to update UI label
    void walletChargedResponse(bool success, QString message, long long newBalance);
    void buyItemResponse(bool success, QString message);

    void globalChatReceived(QVector<Message> messages);

private slots:
    void onReadyRead();

private:
    explicit NetworkManager(QObject *parent = nullptr);
    QTcpSocket *socket;
    qint32 blockSize = 0;
    void sendPacket(RequestType type, const QByteArray &additionalData = QByteArray());
};

#endif // NETWORKMANAGER_H
