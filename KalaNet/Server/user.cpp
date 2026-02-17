#include "user.h"

User::User() {
    walletBalance = 0;
    isAdmin = false;
}

QJsonObject User::toJson() const {
    QJsonObject json;
    json["username"] = username;
    json["passwordHash"] = passwordHash;
    json["fullName"] = fullName;
    json["phoneNumber"] = phoneNumber;
    json["email"] = email;
    json["address"] = address;
    json["walletBalance"] = (qint64)walletBalance;
    json["isAdmin"] = isAdmin;
    return json;
}

User User::fromJson(const QJsonObject &json) {
    User u;
    u.username = json["username"].toString();
    u.passwordHash = json["passwordHash"].toString();
    u.fullName = json["fullName"].toString();
    u.phoneNumber = json["phoneNumber"].toString();
    u.email = json["email"].toString();
    u.address = json["address"].toString();
    u.walletBalance = json["walletBalance"].toInteger();
    u.isAdmin = json["isAdmin"].toBool();
    return u;
}

QDataStream &operator<<(QDataStream &out, const User &user) {
    out << user.username << user.passwordHash << user.fullName
        << user.phoneNumber << user.email << user.address
        << user.walletBalance << user.isAdmin;
    return out;
}

QDataStream &operator>>(QDataStream &in, User &user) {
    in >> user.username >> user.passwordHash >> user.fullName
        >> user.phoneNumber >> user.email >> user.address
        >> user.walletBalance >> user.isAdmin;
    return in;
}
