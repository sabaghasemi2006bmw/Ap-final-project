#ifndef USER_H
#define USER_H

#include <QString>
#include <QJsonObject>
#include <QDataStream>

class User {
public:
    User();

    // Core Data
    QString username;
    QString passwordHash; // We store the HASH, not the plain password
    QString fullName;
    QString phoneNumber;
    QString email;
    QString address;

    // Wallet & Role
    long long walletBalance;
    bool isAdmin;

    // Serialization (Convert to/from JSON for saving to file)
    QJsonObject toJson() const;
    static User fromJson(const QJsonObject &json);

    // Stream Operators (For sending over Network Socket)
    friend QDataStream &operator<<(QDataStream &out, const User &user);
    friend QDataStream &operator>>(QDataStream &in, User &user);
};
#endif // USER_H
