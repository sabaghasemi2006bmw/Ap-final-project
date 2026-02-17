#include "adcard.h"
#include <QBuffer>
#include <QPushButton>
AdCard::AdCard(const Ad &ad, QWidget *parent) : QFrame(parent) {
    this->setFrameShape(QFrame::StyledPanel);
    this->setStyleSheet("background-color: white; border: 1px solid #d1d1d1; border-radius: 8px;");
    this->setFixedSize(380, 160);



    // --- 1. Decode the Image ---
    QPixmap pixmap = decodeImage(ad.imageBase64);
    QLabel* imgLabel = new QLabel();
    imgLabel->setPixmap(pixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    imgLabel->setFixedSize(120, 120);
    imgLabel->setStyleSheet("border: 1px solid #eee; border-radius: 4px;");

    // --- 2. Text Details ---
    QVBoxLayout* detailsLayout = new QVBoxLayout();

    // Title
    QLabel* lblTitle = new QLabel(ad.title);
    lblTitle->setStyleSheet("font-weight: bold; font-size: 16px; color: #333; border: none;");
    lblTitle->setWordWrap(true);

    // Status (Translated to String)
    QLabel* lblStatus = new QLabel(getStatusString(ad.status));
    // Color code the status
    QString statusColor = (ad.status == AdStatus::APPROVED) ? "green" :
                              (ad.status == AdStatus::PENDING) ? "orange" : "red";
    lblStatus->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border: none;").arg(statusColor));
    QString Cate = "";
    QLabel* lblCategory = new QLabel(ad.category);
    QString CategoryColor = "gold";
    lblCategory->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border: none;").arg(statusColor));
    // Price
    QLabel* lblPrice = new QLabel(QString::number(ad.price) + " Token"); // Using Tokens as per PDF
    lblPrice->setStyleSheet("font-weight: bold; color: #1976D2; font-size: 14px; border: none;");

    // Date
    QLabel* lblDate = new QLabel(ad.date);
    lblDate->setStyleSheet("color: #9e9e9e; font-size: 11px; border: none;");
    //
    btnAdd = new QPushButton("Add to Cart");
    btnAdd->setStyleSheet("background-color: #FFA500; color: white; border-radius: 4px; padding: 5px;"); // Orange color
    btnAdd->setCursor(Qt::PointingHandCursor);

    connect(btnAdd, &QPushButton::clicked, [this, ad](){
        emit addToCartClicked(ad); // Send the whole ad to MainWindow
    });
    btnRemove = new QPushButton("Remove", this);
    btnRemove->setStyleSheet("background-color: #D32F2F; color: white;"); // Red
    btnRemove->setCursor(Qt::PointingHandCursor);
    btnRemove->setVisible(false); // Hidden by default
    connect(btnRemove, &QPushButton::clicked, [this, ad](){ emit removeFromCartClicked(ad.id); });

    // --- 3. Assemble Layout ---
    detailsLayout->addWidget(lblTitle);
    detailsLayout->addWidget(lblStatus);
    detailsLayout->addStretch();
    detailsLayout->addWidget(lblCategory);
    detailsLayout->addWidget(lblPrice);
    detailsLayout->addWidget(lblDate);
    detailsLayout->addWidget(btnAdd);
    detailsLayout->addWidget(btnRemove);



    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(imgLabel);       // Image on Left
    mainLayout->addLayout(detailsLayout);  // Text on Right

    // Set RTL for Persian UI
    this->setLayoutDirection(Qt::RightToLeft);
}

QPixmap AdCard::decodeImage(const QString &base64) {
    if (base64.isEmpty()) {
        QPixmap placeholder(120, 120);
        placeholder.fill(Qt::lightGray);
        return placeholder;
    }

    QByteArray imageData = QByteArray::fromBase64(base64.toLatin1());
    QPixmap pixmap;
    pixmap.loadFromData(imageData);
    return pixmap;
}

void AdCard::enableCartMode()
{
    btnAdd->setVisible(false);
    btnRemove->setVisible(true);
}

QString AdCard::getStatusString(AdStatus status) {
    switch (status) {
    case AdStatus::PENDING: return "Pending Review";
    case AdStatus::APPROVED: return "Available";
    case AdStatus::SOLD: return "Sold";
    case AdStatus::REJECTED: return "Rejected";
    default: return "Unknown";
    }
}

