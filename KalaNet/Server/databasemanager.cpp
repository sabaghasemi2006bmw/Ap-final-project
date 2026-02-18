#include "databasemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QUuid>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {
    loadData();
    loadAds();
    loadDiscounts();

}


void DatabaseManager::loadData() {
    QMutexLocker locker(&mutex); // Lock so no one edits while we load

    QFile file(USERS_FILE);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No users file found. Creating new database.";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();

    users.clear();
    for (const QJsonValue &val : array) {
        users.append(User::fromJson(val.toObject()));
    }
    file.close();
    qDebug() << "Loaded" << users.size() << "users.";
}

void DatabaseManager::saveData() {
    QMutexLocker locker(&mutex);

    QJsonArray array;
    for (const User &u : users) {
        array.append(u.toJson());
    }

    QJsonDocument doc(array);
    QFile file(USERS_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

bool DatabaseManager::registerUser(const User &newUser) {
    QMutexLocker locker(&mutex);

    // Check if username already exists
    for (const User &u : users) {
        if (u.username == newUser.username) return false;
    }

    users.append(newUser);
    QJsonArray array;
    for (const User &u : users) array.append(u.toJson());
    QJsonDocument doc(array);
    QFile file(USERS_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }

    return true;
}

User* DatabaseManager::loginUser(QString username, QString passwordHash) {
    QMutexLocker locker(&mutex);


    for (int i = 0; i < users.size(); ++i) {
        if (users[i].username == username && users[i].passwordHash == passwordHash) {
            return &users[i];
        }
    }
    return nullptr;
}

void DatabaseManager::loadAds() {
    QMutexLocker locker(&mutex);
    QFile file(ADS_FILE);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
    ads.clear();
    for (const QJsonValue &val : array) {
        ads.append(Ad::fromJson(val.toObject()));
    }
    file.close();
}

void DatabaseManager::saveAds() {

    QMutexLocker locker(&mutex);

    QJsonArray array;
    for (const Ad &a :ads) {
        array.append(a.toJson());
    }

    QJsonDocument doc(array);
    QFile file(ADS_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }

}

void DatabaseManager::saveGlobalChat()
{
    QJsonArray chatArray;

    for (const Message &msg : globalMessages) {
        chatArray.append(msg.toJson());
    }

    QJsonDocument doc(chatArray);

    QFile file("global_chat.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open global_chat.json for writing!";
        return;
    }

    // 5. Write the data
    file.write(doc.toJson());
    file.close();
}

void DatabaseManager::saveDiscounts()
{
    {
        QJsonArray arr;
        for (const DiscountCode &dc : discountCodes) {
            arr.append(dc.toJson());
        }

        QJsonDocument doc(arr);

        QFile file("discounts.json");
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Could not open discounts.json for writing!";
            return;
        }

        file.write(doc.toJson());
        file.close();

        qDebug() << "Discounts saved successfully.";
    }
}



void DatabaseManager::loadDiscounts()
{
    QFile file("discounts.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No discounts file found. Starting fresh.";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "Failed to parse discounts.json or invalid format.";
        return;
    }

    discountCodes.clear();

    QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        if (val.isObject()) {
            discountCodes.append(DiscountCode::fromJson(val.toObject()));
        }
    }

    qDebug() << "Loaded" << discountCodes.size() << "discount codes.";
}

bool DatabaseManager::createAd(Ad &newAd) {
    QMutexLocker locker(&mutex);

    for (const Ad &existingAd : ads) {
        if (existingAd.sellerUsername == newAd.sellerUsername &&
            existingAd.title == newAd.title &&
            existingAd.description == newAd.description &&
            existingAd.status != AdStatus::SOLD)
        {

            return false;
        }
    }

    newAd.id = QUuid::createUuid().toString(QUuid::Id128);
    newAd.status = AdStatus::PENDING;

    ads.append(newAd);
    mutex.unlock();
    saveAds();
    mutex.lock();
    return true;
}

QVector<Ad> DatabaseManager::getAllAds() {
    QMutexLocker locker(&mutex);
    return ads;
}

QVector<Ad> DatabaseManager::getApprovedAds() {
    QMutexLocker locker(&mutex);
    QVector<Ad> result;
    for(const Ad &ad : ads) {
        if(ad.status == AdStatus::APPROVED) result.append(ad);
    }
    return result;
}

void DatabaseManager::updateAdStatus(QString adId, AdStatus newStatus) {
    QMutexLocker locker(&mutex);
    for(Ad &ad : ads) {
        if(ad.id == adId) {
            ad.status = newStatus;
            break;
        }
    }
    mutex.unlock();
    saveAds();
    mutex.lock();

}

bool DatabaseManager::updateUser(const User &updatedUser) {
    QMutexLocker locker(&mutex);

    for (int i = 0; i < users.size(); ++i) {
        if (users[i].username == updatedUser.username) {
            // Update editable fields
            users[i].fullName = updatedUser.fullName;
            users[i].phoneNumber = updatedUser.phoneNumber;
            users[i].email = updatedUser.email;
            users[i].address = updatedUser.address;
            users[i].passwordHash = updatedUser.passwordHash;
            users[i].walletBalance = updatedUser.walletBalance;
            locker.unlock();
            saveData();

            return true;
        }
    }

    return false;
}

int DatabaseManager::processPurchase(QString buyerUsername, QString adId, long long paidAmount)
{
    QMutexLocker locker(&mutex);

    Ad *targetAd = nullptr;
    for (int i = 0; i < ads.size(); ++i) {
        if (ads[i].id == adId) {
            targetAd = &ads[i];
            break;
        }
    }

    if (!targetAd || targetAd->status != AdStatus::APPROVED) {
        return 1;
    }
    if (targetAd->sellerUsername == buyerUsername) {
        return 3;
    }

    User *buyer = nullptr;
    User *seller = nullptr;

    for (int i = 0; i < users.size(); ++i) {
        if (users[i].username == buyerUsername) buyer = &users[i];
        if (users[i].username == targetAd->sellerUsername) seller = &users[i];
    }

    if (!buyer || !seller) {
        return 1;
    }

    if (buyer->walletBalance < paidAmount) {
        return 2;
    }


    buyer->walletBalance -= paidAmount;
    seller->walletBalance += paidAmount;

    // Update Ad Status
    targetAd->status = AdStatus::SOLD;
    targetAd->buyerUsername = buyerUsername; // Record the buyer for history

    mutex.unlock();
    saveData();
    saveAds();
    mutex.lock();

    qDebug() << "Transaction Complete:" << buyerUsername << "bought" << targetAd->title << "for" << paidAmount;

    return 0; // Success
}

void DatabaseManager::addGlobalMessage(const Message &msg) {
    QMutexLocker locker(&mutex);
    globalMessages.append(msg);
    saveGlobalChat(); // Save to json file
}

QVector<Message> DatabaseManager::getGlobalMessages() {
    QMutexLocker locker(&mutex);
    return globalMessages;
}

long long DatabaseManager::chargeWallet(QString username, long long amount)
{
    QMutexLocker locker(&mutex);

    for (int i = 0; i < users.size(); ++i) {
        if (users[i].username == username) {

            users[i].walletBalance += amount;

            mutex.unlock();
            saveData();
            mutex.lock();

            qDebug() << "Wallet Charged:" << username << "+" << amount << "New Balance:" << users[i].walletBalance;

            return users[i].walletBalance;
        }
    }

    // User not found
    return -1;
}

void DatabaseManager::addDiscount(const DiscountCode &dc) {
    QMutexLocker locker(&mutex);
    discountCodes.append(dc);
    saveDiscounts();
}

int DatabaseManager::getDiscountPercentage(QString code) {
    QMutexLocker locker(&mutex);
    QDate today = QDate::currentDate();

    for (const DiscountCode &dc : discountCodes) {
        if (dc.code == code) {
            if (dc.expirationDate >= today) {
                return dc.percentage;
            } else {
                return -1;
            }
        }
    }
    mutex.unlock();
    saveDiscounts();
    mutex.lock();
    return 0;
}

QVector<DiscountCode> DatabaseManager::getAllDiscounts()
{
    QMutexLocker locker(&mutex);
    return discountCodes;
}
