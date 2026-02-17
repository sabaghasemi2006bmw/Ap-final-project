#include "clienthandler.h"
#include "qjsonarray.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRandomGenerator>

ClientHandler::ClientHandler(qintptr socketDescriptor, DatabaseManager *db, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor), db(db)
{
}

void ClientHandler::run() {
    socket = new QTcpSocket();
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        emit error(socket->error());
        return;
    }

    connect(socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);

    exec();
}

void ClientHandler::onReadyRead() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);

    while (true) {
        if (blockSize == 0) {
            if (socket->bytesAvailable() < (qint64)sizeof(qint32))
                return;

            in >> blockSize;
        }

        if (socket->bytesAvailable() < blockSize)
            return;

        qint32 typeInt;
        in >> typeInt;
        RequestType type = static_cast<RequestType>(typeInt);

        switch (type) {
        case RequestType::LOGIN:    handleLogin(in); break;
        case RequestType::REGISTER: handleRegister(in); break;
        case RequestType::CREATE_AD: handleCreateAd(in); break;
        case RequestType::GET_ALL_ADS: handleGetAds(in); break;
        case RequestType::UPDATE_PROFILE: handleUpdateProfile(in); break;
        case RequestType::BUY_ITEM:handleBuyItem(in);break;
        case RequestType::GET_CAPTCHA:handleGetCaptcha(in); break;
        case RequestType::CHARGE_WALLET: handleChargeWallet(in); break;
        case RequestType::SEND_GLOBAL_MESSAGE: {
            Message msg;
            in >> msg;
            db->addGlobalMessage(msg);
            break;
        }
        case RequestType::GET_GLOBAL_CHAT: {
            // 1. Get all messages
            QVector<Message> history = db->getGlobalMessages();

            // 2. Convert to JSON Array
            QJsonArray arr;
            for(const Message &m : history) arr.append(m.toJson());

            QJsonObject data;
            data["messages"] = arr;

            sendResponse(RequestType::GET_GLOBAL_CHAT, true, "OK", data);
            break;
        }
        default: break;
        }

        blockSize = 0;
    }
}


void ClientHandler::handleCreateAd(QDataStream &in) {

    Ad newAd;
    in >> newAd;

    qDebug() << "Received Create Ad Request from:" << newAd.sellerUsername;


    bool success = db->createAd(newAd);

    if (success) {
        sendResponse(RequestType::CREATE_AD, true, "Ad created successfully! It is now waiting for Admin approval.");
    } else {
        sendResponse(RequestType::CREATE_AD, false, "Error: You have already posted an identical ad.");
    }
}

void ClientHandler::handleGetAds(QDataStream &in) {

    db->loadAds();
    QVector<Ad> ads = db->getAllAds();
    //QVector<Ad> ads = db->getApprovedAds();

    QJsonArray adArray;
    for(const Ad &a : ads) adArray.append(a.toJson());

    sendResponse(RequestType::GET_ALL_ADS, true, "OK", QJsonObject{{"ads", adArray}});
}

void ClientHandler::handleRegister(QDataStream &in) {
    User newUser;
    in >> newUser;


    if (db->registerUser(newUser)) {

        QJsonObject userData = newUser.toJson();

        sendResponse(RequestType::REGISTER, true, "Registration Successful!", userData);
    } else {
        sendResponse(RequestType::REGISTER, false, "Username already exists.");
    }

}

void ClientHandler::handleLogin(QDataStream &in) {
    QString username, passwordHash;
    in >> username >> passwordHash;

    User* user = db->loginUser(username, passwordHash);

    if (user) {
        // Send back success + the full user profile (so Client knows wallet balance/Address)
        sendResponse(RequestType::LOGIN, true, "Welcome back!", user->toJson());
    } else {
        sendResponse(RequestType::LOGIN, false, "Invalid username or password.");
    }
}

void ClientHandler::handleUpdateProfile(QDataStream &in) {
    User updatedUser;
    in >> updatedUser;

    bool success = db->updateUser(updatedUser);

    if (success) {
        sendResponse(RequestType::UPDATE_PROFILE, true, "Profile updated!");
    } else {
        sendResponse(RequestType::UPDATE_PROFILE, false, "Could not update profile.");
    }
}

void ClientHandler::handleBuyItem(QDataStream &in)
{
    QString adId, buyerUsername;
    long long paidAmount;

    // 1. Read the data sent by the Client
    in >> adId >> buyerUsername >> paidAmount;

    // 2. Process the transaction
    int result = db->processPurchase(buyerUsername, adId, paidAmount);

    // 3. Send appropriate response based on the result code
    if (result == 0) {
        sendResponse(RequestType::BUY_ITEM, true, QString("Purchase Successful! paid :%1").arg(paidAmount));
    }
    else if (result == 2) {
        sendResponse(RequestType::BUY_ITEM, false, "Insufficient Funds in Wallet.");
    }
    else if (result == 3) {
        sendResponse(RequestType::BUY_ITEM, false, "You cannot buy your own item.");
    }
    else {
        sendResponse(RequestType::BUY_ITEM, false, "Item is no longer available.");
    }
}
void ClientHandler::handleGetCaptcha(QDataStream &in) {
    int code = QRandomGenerator::global()->bounded(1000, 9999);
    sessionCaptcha = QString::number(code);
    qDebug() << code;

    QJsonObject data;
    data["captcha"] = sessionCaptcha;

    sendResponse(RequestType::GET_CAPTCHA, true, "New Code", data);
}


void ClientHandler::handleChargeWallet(QDataStream &in) {
    QString username, cardNumber, userCaptcha;
    long long amount;

    // Read Data
    in >> username >> cardNumber >> amount >> userCaptcha;

    // 1. Check Captcha
    if (userCaptcha != sessionCaptcha) {
        sendResponse(RequestType::CHARGE_WALLET, false, "Wrong Captcha!");
        return;
    }

    // 2. Check Card
    if (cardNumber.length() != 16) {
        sendResponse(RequestType::CHARGE_WALLET, false, "Invalid Card Number (Must be 16 digits).");
        return;
    }

    // 3. Add Money
    long long newBalance = db->chargeWallet(username, amount); // Uses your existing DB function

    QJsonObject data;
    data["newBalance"] = (qint64)newBalance;

    sendResponse(RequestType::CHARGE_WALLET, true, "Wallet Charged Successfully!", data);

    // Clear captcha so it can't be reused
    sessionCaptcha.clear();
}

void ClientHandler::sendResponse(RequestType type, bool success, const QString &message, const QJsonObject &data) {
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    // 1. Reserve 4 bytes for the Size Header
    out << (qint32)0;

    // 2. Write the Standard Payload
    out << (qint32)type;
    out << success;
    out << message;

    // Write JSON Data (as a byte array to be safe)
    out << QJsonDocument(data).toJson(QJsonDocument::Compact);

    // 3. Jump back to the start and write the actual size
    out.device()->seek(0);
    out << (qint32)(packet.size() - sizeof(qint32));

    // 4. Send
    socket->write(packet);
    socket->flush();
}



void ClientHandler::onDisconnected() {
    socket->deleteLater();
    exit(0);
}
