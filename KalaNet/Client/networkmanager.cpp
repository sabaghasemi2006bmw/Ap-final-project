#include "networkmanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QMessageBox>

NetworkManager& NetworkManager::instance() {
    static NetworkManager _instance;
    return _instance;
}

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &NetworkManager::connected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::disconnected);
}

void NetworkManager::sendPacket(RequestType type, const QByteArray &payloadData)
{
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    // 1. Reserve Size Header
    out << (qint32)0;

    // 2. Write Request Type
    out << (qint32)type;

    // 3. Write Specific Data (Login info, Ad object, etc.)
    // We write the raw bytes directly to the stream
    if (!payloadData.isEmpty()) {
        out.writeRawData(payloadData.constData(), payloadData.size());
    }

    // 4. Update Size Header
    out.device()->seek(0);
    out << (qint32)(packet.size() - sizeof(qint32));

    // 5. Send
    socket->write(packet);
    socket->flush();
}

void NetworkManager::connectToServer() {
    // Connect to Localhost (127.0.0.1) on Port 1234
    socket->connectToHost("127.0.0.1", 1234);
}

void NetworkManager::sendLogin(QString username, QString passwordHash) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << username << passwordHash; // Serialize arguments

    // Send using our safe helper
    sendPacket(RequestType::LOGIN, payload);
}

void NetworkManager::sendRegister(User user) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << user; // Uses operator<< from user.cpp

    sendPacket(RequestType::REGISTER, payload);
}

void NetworkManager::sendCreateAd(const Ad &ad) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << ad;

    sendPacket(RequestType::CREATE_AD, payload);
}

void NetworkManager::sendGetAds() {
    sendPacket(RequestType::GET_ALL_ADS);
}

void NetworkManager::onReadyRead() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);

    while (true) {
        // 1. Read Size Header
        if (blockSize == 0) {
            if (socket->bytesAvailable() < (qint64)sizeof(qint32))
                return;
            in >> blockSize;
        }

        // 2. Wait for Full Body
        if (socket->bytesAvailable() < blockSize)
            return;

        // 3. Process Packet
        qint32 typeInt;
        bool success;
        QString message;
        QByteArray jsonData;

        in >> typeInt >> success >> message >> jsonData;
        RequestType type = static_cast<RequestType>(typeInt);

        // Convert JSON bytes to Object
        QJsonObject data = QJsonDocument::fromJson(jsonData).object();

        // 4. Route to Signals
        switch (type) {
        case RequestType::LOGIN:
            emit loginResponse(success, message, success ? User::fromJson(data) : User());
            break;
        case RequestType::REGISTER:{
            User registeredUser;

            if (success && !jsonData.isEmpty()) {
                QJsonObject dataObj = QJsonDocument::fromJson(jsonData).object();
                registeredUser = User::fromJson(dataObj);
            }

            emit registerResponse(success, message, registeredUser);
            break;
        }
        case RequestType::CREATE_AD:
            emit createAdResponse(success, message);
            break;
        case RequestType::GET_ALL_ADS: {
            QVector<Ad> resultAds;
            if (success && data.contains("ads")) {
                QJsonArray arr = data["ads"].toArray();
                for(const auto &val : arr) resultAds.append(Ad::fromJson(val.toObject()));
            }
            emit getAdsResponse(success, message, resultAds);
            break;
        }
        case RequestType::UPDATE_PROFILE:
            emit updateProfileResponse(success, message);
            break;
        case RequestType::GET_CAPTCHA: {
            QString code = data["captcha"].toString();
            emit captchaReceived(code);
            break;
        }

        case RequestType::CHARGE_WALLET: {
            long long bal = data["newBalance"].toInteger();
            emit walletChargedResponse(success, message, bal);
            break;
        }
        case RequestType::BUY_ITEM:
            emit buyItemResponse(success, message);
            break;
        case RequestType::GET_GLOBAL_CHAT: {
            QVector<Message> msgs;
            if (success) {
                QJsonArray arr = data["messages"].toArray();
                for(auto v : arr) {
                    msgs.append(Message::fromJson(v.toObject()));
                }
            }
            emit globalChatReceived(msgs);
            break;
        }
        default: break;
        }

        // 5. Reset
        blockSize = 0;
    }
}

void NetworkManager::sendUpdateProfile(const User &user) {
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << (qint32)RequestType::UPDATE_PROFILE;
    out << user; // Sends the whole user object (including the new info)

    socket->write(packet);
}

void NetworkManager::sendBuyItem(QString adId, QString username, long long price)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << adId << username << price; // Send 3 items

    sendPacket(RequestType::BUY_ITEM, payload);

}

void NetworkManager::sendGetCaptcha() {

    sendPacket(RequestType::GET_CAPTCHA);
}

void NetworkManager::sendChargeWallet(QString username, QString cardNum, long long amount, QString captcha) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << username << cardNum << amount << captcha;

    sendPacket(RequestType::CHARGE_WALLET, payload);
}

void NetworkManager::sendGlobalMessage(QString sender, QString text) {
    Message msg(sender, text);

    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << msg;

    sendPacket(RequestType::SEND_GLOBAL_MESSAGE, payload);
}

void NetworkManager::getGlobalChat() {
    sendPacket(RequestType::GET_GLOBAL_CHAT);
}
