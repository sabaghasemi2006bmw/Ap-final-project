#ifndef ADCARD_H
#define ADCARD_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ad.h" // Include our Common Ad class

class AdCard : public QFrame
{
    Q_OBJECT
public:
    // We now accept the full Ad object directly
    explicit AdCard(const Ad &ad, QWidget *parent = nullptr);
    void enableCartMode();


private:
    // Helper to decode Base64 to QPixmap
    QPixmap decodeImage(const QString &base64);
    // Helper to get string from Status Enum
    QString getStatusString(AdStatus status);
    QPushButton* btnAdd;
    QPushButton* btnRemove;
signals:
    void addToCartClicked(Ad ad);
    void removeFromCartClicked(QString adId);
};

#endif // ADCARD_H
