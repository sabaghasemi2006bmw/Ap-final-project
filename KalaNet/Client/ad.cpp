#include "ad.h"
#include <QUuid> // To generate random IDs if needed (or do it in server)
#include <QDateTime>

Ad::Ad() {
    price = 0;
    status = AdStatus::PENDING; // Default status
    date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
}

QJsonObject Ad::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["title"] = title;
    json["description"] = description;
    json["category"] = category;
    json["price"] = (qint64)price;
    json["imageBase64"] = imageBase64;
    json["buyerUsername"] = buyerUsername;
    json["sellerUsername"] = sellerUsername;
    json["status"] = (int)status;
    json["date"] = date;
    return json;
}

Ad Ad::fromJson(const QJsonObject &json) {
    Ad ad;
    ad.id = json["id"].toString();
    ad.title = json["title"].toString();
    ad.description = json["description"].toString();
    ad.category = json["category"].toString();
    ad.price = json["price"].toInteger();
    ad.imageBase64 = json["imageBase64"].toString();
    ad.buyerUsername = json["buyerUsername"].toString();
    ad.sellerUsername = json["sellerUsername"].toString();
    ad.status = static_cast<AdStatus>(json["status"].toInt());
    ad.date = json["date"].toString();
    return ad;
}

QDataStream &operator<<(QDataStream &out, const Ad &ad) {
    out << ad.id << ad.title << ad.description << ad.category
        << (qint64)ad.price << ad.imageBase64 << ad.sellerUsername
        << ad.buyerUsername<< (qint32)ad.status << ad.date;
    return out;
}

QDataStream &operator>>(QDataStream &in, Ad &ad) {
    qint64 p; qint32 s;
    in >> ad.id >> ad.title >> ad.description >> ad.category
        >> p >> ad.imageBase64 >> ad.sellerUsername >> ad.buyerUsername >> s >> ad.date;
    ad.price = p;
    ad.status = static_cast<AdStatus>(s);
    return in;
}
