#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QJsonObject>
#include <QDataStream>
#include <QDateTime>

class Message {
public:
    QString sender;
    QString text;
    QString timestamp;

    Message() {}
    Message(QString s, QString t) : sender(s), text(t) {
        timestamp = QDateTime::currentDateTime().toString("HH:mm"); // Just show time
    }

    // Serialization for Network
    friend QDataStream &operator<<(QDataStream &out, const Message &m) {
        out << m.sender << m.text << m.timestamp;
        return out;
    }
    friend QDataStream &operator>>(QDataStream &in, Message &m) {
        in >> m.sender >> m.text >> m.timestamp;
        return in;
    }

    // JSON for Saving to File
    QJsonObject toJson() const {
        return QJsonObject{ {"sender", sender}, {"text", text}, {"time", timestamp} };
    }
    static Message fromJson(const QJsonObject &json) {
        Message m;
        m.sender = json["sender"].toString();
        m.text = json["text"].toString();
        m.timestamp = json["time"].toString();
        return m;
    }
};
#endif // MESSAGE_H
