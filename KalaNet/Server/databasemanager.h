#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QMutex>
#include "user.h" // From Common
#include "message.h"
#include <ad.h>
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    void loadData();
    void saveData();

    void loadAds();
    void saveAds();

    void saveGlobalChat();

    bool registerUser(const User &newUser); // Returns false if username exists
    User* loginUser(QString username, QString passwordHash);

    int getUserCount() const { return users.size(); }
    QVector<QString> getChatContacts(QString username);


    // Ad Management
    bool createAd(Ad &newAd); // Pass by ref to update ID
    QVector<Ad> getAllAds();  // Returns all ads (for Admin)
    QVector<Ad> getApprovedAds(); // Returns only approved (for Clients)

    // Admin Actions
    void updateAdStatus(QString adId, AdStatus newStatus);
    bool updateUser(const User &updatedUser);
    int processPurchase(QString buyerUsername, QString adId, long long paidAmount);
    long long chargeWallet(QString username, long long amount);
    void addGlobalMessage(const Message &msg);
    QVector<Message> getGlobalMessages();



private:
    QVector<User> users;
    QVector<Ad> ads;
    QVector<Message> globalMessages;

    QMutex mutex;

    const QString USERS_FILE = "users.json";
    const QString ADS_FILE = "ads.json";

};

#endif // DATABASEMANAGER_H
