#ifndef DISCOUNT_H
#define DISCOUNT_H

#include <QString>
#include <QDate>
#include <QJsonObject>

struct DiscountCode {
    QString code;
    int percentage;
    QDate expirationDate;

    QJsonObject toJson() const {
        return QJsonObject{
            {"code", code},
            {"percentage", percentage},
            {"expiration", expirationDate.toString(Qt::ISODate)}
        };
    }

    static DiscountCode fromJson(const QJsonObject &json) {
        DiscountCode d;
        d.code = json["code"].toString();
        d.percentage = json["percentage"].toInt();
        d.expirationDate = QDate::fromString(json["expiration"].toString(), Qt::ISODate);
        return d;
    }
};

#endif // DISCOUNT_H
