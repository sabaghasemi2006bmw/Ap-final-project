#include "mainwindow.h"
#include <QCryptographicHash>
#include <QMessageBox>
#include <QFileDialog>
#include <QBuffer>
#include <QMessageBox>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "networkmanager.h"
#include "ui_mainwindow.h"
#include "ad.h"
#include "adcard.h"

QString hashPassword(QString rawPassword) {
    return QString(QCryptographicHash::hash(rawPassword.toUtf8(), QCryptographicHash::Sha256).toHex());
}
QPixmap decodeBase64(const QString &base64) {
    if (base64.isEmpty()) return QPixmap();
    QPixmap img;
    img.loadFromData(QByteArray::fromBase64(base64.toLatin1()));
    return img;
}

QWidget* createMessageWidget(const Message &msg, bool isMe) {
    QWidget *widget = new QWidget();

    QLabel *lblSender = new QLabel(msg.sender);
    lblSender->setStyleSheet("font-size: 10px; color: gray; font-weight: bold; margin-bottom: 2px;");

    QLabel *lblText = new QLabel(msg.text);
    lblText->setWordWrap(true);
    lblText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblText->setStyleSheet("font-size: 20px; padding: 5px 15px 15px 15px; background: transparent;");
    lblText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QLabel *lblTime = new QLabel(msg.timestamp);
    lblTime->setStyleSheet("font-size: 9px; color: #555; margin-top: 2px;");

    QString bubbleStyle;
    if (isMe) {
        bubbleStyle = "background-color: #DCF8C6; border-radius: 10px; border: 1px solid #C0E8A8;";
        lblSender->setAlignment(Qt::AlignRight);
        lblTime->setAlignment(Qt::AlignRight);
    } else {
        bubbleStyle = "background-color: #FFFFFF; border-radius: 10px; border: 1px solid #E0E0E0;";
        lblSender->setAlignment(Qt::AlignLeft);
        lblTime->setAlignment(Qt::AlignLeft);
    }

    QWidget *textContainer = new QWidget();
    textContainer->setStyleSheet(bubbleStyle);
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(8, 8, 8, 8);
    textLayout->addWidget(lblText);

    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->addWidget(lblSender);

    mainLayout->addWidget(textContainer);

    mainLayout->addWidget(lblTime);

    if (isMe) {
        mainLayout->setAlignment(Qt::AlignRight);
        textContainer->setMaximumWidth(300);
    } else {
        mainLayout->setAlignment(Qt::AlignLeft);
        textContainer->setMaximumWidth(300);
    }

    return widget;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    NetworkManager::instance().connectToServer();

    connect(&NetworkManager::instance(), &NetworkManager::loginResponse,
            this, &::MainWindow::handleLoginResponse);
    connect(&NetworkManager::instance(), &NetworkManager::registerResponse,
            this, &MainWindow::handleRegisterResponse);
    connect(&NetworkManager::instance(), &NetworkManager::updateProfileResponse,
            this, &MainWindow::onUpdateProfileResponse);
    connect(&NetworkManager::instance(), &NetworkManager::createAdResponse,
            this, &MainWindow::onCreateAdResponse);
    connect(&NetworkManager::instance(), &NetworkManager::getAdsResponse,
            this, &MainWindow::onGetAdsResponse);
    connect(&NetworkManager::instance(), &NetworkManager::captchaReceived,
            this, &MainWindow::onCaptchaReceived);
    connect(&NetworkManager::instance(), &NetworkManager::buyItemResponse,
            this, &MainWindow::onBuyItemResponse);
    connect(&NetworkManager::instance(), &NetworkManager::walletChargedResponse,
            this, &MainWindow::onChargeResponse);
    connect(&NetworkManager::instance(), &NetworkManager::globalChatReceived,
            this, &MainWindow::onGlobalChatReceived);
    connect(&NetworkManager::instance(), &NetworkManager::discountChecked,
            this, &MainWindow::onDiscountChecked);


    this->setFixedSize(1200,800);
    this->move(400,0);
    ui->App->setCurrentIndex(1);
    ////////////////////////////////////////////////////////////////////////
    QPixmap server_page(":/Images/First_BackGround.png");
    QPalette palette1;
    palette1.setBrush(QPalette::Window, QBrush(server_page));
    ui->Login->setPalette(palette1);
    ui->Login->setAutoFillBackground(true);

    QPixmap store_page(":/Images/store_background.jpg");
    QPalette palette2;
    palette2.setBrush(QPalette::Window, QBrush(store_page));
    ui->Store->setPalette(palette2);
    ui->Store->setAutoFillBackground(true);

    QPixmap Ad_Reg_page(":/Images/Ad_registration.png");
    QPalette palette3;
    palette3.setBrush(QPalette::Window, QBrush(Ad_Reg_page));
    ui->Register_Ad->setPalette(palette3);
    ui->Register_Ad->setAutoFillBackground(true);

    QPixmap His_page(":/Images/History_bg.png");
    QPalette palette4;
    palette4.setBrush(QPalette::Window, QBrush(His_page));
    ui->History->setPalette(palette4);
    ui->History->setAutoFillBackground(true);

    QPixmap Wallet_page(":/Images/WalletCharg_bg.png");
    QPalette palette5;
    palette5.setBrush(QPalette::Window, QBrush(Wallet_page));
    ui->Walletcharge->setPalette(palette3);
    ui->Walletcharge->setAutoFillBackground(true);

    QPixmap Basket_page(":/Images/Basket_bg.png");
    QPalette palette6;
    palette6.setBrush(QPalette::Window, QBrush(Basket_page));
    ui->Shopping_bask->setPalette(palette3);
    ui->Shopping_bask->setAutoFillBackground(true);

    QPixmap Chat_page(":/Images/Chat_bg.png");
    QPalette palette7;
    palette7.setBrush(QPalette::Window, QBrush(Chat_page));
    ui->Chat->setPalette(palette7);
    ui->Chat->setAutoFillBackground(true);
    //////////////////////////////////////////////////////////////////////////


    QVBoxLayout *scrollLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);

}

void MainWindow::updateAdDisplay()
{
    QWidget *container = ui->scrollAreaWidgetContents;
    if (!container->layout()) container->setLayout(new QVBoxLayout());
    QLayout *layout = container->layout();

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }


    QString searchText = ui->search_le->text().trimmed();
    for (const Ad &ad : adList) {
        if (ad.status != AdStatus::APPROVED) continue;

        bool catMatch = false;
        if(ui->All->isChecked()){
            catMatch = true;
        }
        if(ui->House->isChecked() && ad.category=="House"){
            catMatch = true;
        }
        if(ui->Kitchen->isChecked() && ad.category=="Kitchen"){
            catMatch = true;
        }
        if(ui->DigitalDevices->isChecked() && ad.category=="DigitalDevices"){
            catMatch = true;
        }
        if(ui->Personalthings->isChecked() && ad.category=="Personalthings"){
            catMatch = true;
        }
        if(ui->Vehicles->isChecked() && ad.category=="Vehicles"){
            catMatch = true;
        }

        bool searchMatch = searchText.isEmpty() ||
                           ad.title.contains(searchText, Qt::CaseInsensitive) ||
                           ad.description.contains(searchText, Qt::CaseInsensitive);

        if (catMatch && searchMatch) {
            AdCard *card = new AdCard(ad);
            connect(card, &AdCard::addToCartClicked, this, &MainWindow::onAdAddedToCart);
            layout->addWidget(card);
        }
    }

    ((QVBoxLayout*)layout)->addStretch(); // Push items to the top
}

void MainWindow::refreshCartDisplay()
{
    ui->label_25->setText(QString("%1")
                              .arg(currentUser.walletBalance));
    QWidget *container = ui->scrollArea_Basket->widget();
    if (!container->layout()) container->setLayout(new QVBoxLayout());
    QLayout *layout = container->layout();

    // Clear old items
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget(); delete item;
    }

    long long total = 0;

    for(const Ad &ad : shoppingCart) {
        AdCard *card = new AdCard(ad);
        card->enableCartMode(); // Switch to Remove button
        connect(card, &AdCard::removeFromCartClicked, this, &MainWindow::onRemoveFromCart);
        layout->addWidget(card);

        total += ad.price;
    }

    // Apply Discount
    long long finalPrice = total;
    if (discountFactor > 0.0f) {
        long long discountAmount = total * discountFactor;
        finalPrice = total - discountAmount;
        ui->Dis_Cost->setText(QString("Total: %1\nDiscount: -%2\nFinal: %3 Tokens")
                                         .arg(total).arg(discountAmount).arg(finalPrice));
    } else {
        ui->Dis_Cost->setText("Total: " + QString::number(total) + " Tokens");
    }

    ((QVBoxLayout*)layout)->addStretch();
}

void MainWindow::refreshHistory()
{
    if (ui->table_history->columnCount() == 0) {
        QStringList headers = {"Image", "Title", "Seller", "Price", "Buyer"};
        ui->table_history->setColumnCount(headers.size());
        ui->table_history->setHorizontalHeaderLabels(headers);

        // Styling
        ui->table_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->table_history->verticalHeader()->setDefaultSectionSize(80); // Height for images
        ui->table_history->setIconSize(QSize(70, 70));
        ui->table_history->setEditTriggers(QAbstractItemView::NoEditTriggers); // Read-only
        ui->table_history->setSelectionBehavior(QAbstractItemView::SelectRows);
    }

    // 2. Clear previous data
    ui->table_history->setRowCount(0);

    // 3. Loop through ALL Ads to find my history
    // (Ensure you have called sendGetAds() so adList is up to date)
    for (const Ad &ad : adList) {

        // Filter: Only show SOLD items
        if (ad.status != AdStatus::SOLD) continue;

        // Filter: Am I involved? (Buyer OR Seller)
        bool isMySale = (ad.sellerUsername == currentUser.username);
        bool isMyPurchase = (ad.buyerUsername == currentUser.username);

        if (!isMySale && !isMyPurchase) continue;

        // 4. Add Row
        int row = ui->table_history->rowCount();
        ui->table_history->insertRow(row);

        // --- Col 0: Image ---
        QTableWidgetItem *imgItem = new QTableWidgetItem();
        QPixmap pix = decodeBase64(ad.imageBase64); // Uses your helper
        if (!pix.isNull()) {
            imgItem->setIcon(QIcon(pix.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        } else {
            imgItem->setText("No Image");
        }
        ui->table_history->setItem(row, 0, imgItem);

        // --- Col 1: Title ---
        ui->table_history->setItem(row, 1, new QTableWidgetItem(ad.title));

        // --- Col 2: Seller ---
        QTableWidgetItem *sellerItem = new QTableWidgetItem(ad.sellerUsername);
        if (isMySale) {
            sellerItem->setForeground(QBrush(QColor("blue"))); // Highlight "Me"
            sellerItem->setFont(QFont("Arial", 9, QFont::Bold));
        }
        ui->table_history->setItem(row, 2, sellerItem);

        // --- Col 3: Price ---
        // Visual Trick: Add "-" if I spent money, "+" if I earned money
        QString priceText = QString::number(ad.price);
        QTableWidgetItem *priceItem = new QTableWidgetItem();

        if (isMyPurchase) {
            priceItem->setText("- " + priceText);
            priceItem->setForeground(QBrush(QColor("red"))); // Money Out
        } else {
            priceItem->setText("+ " + priceText);
            priceItem->setForeground(QBrush(QColor("green"))); // Money In
        }
        ui->table_history->setItem(row, 3, priceItem);

        // --- Col 4: Buyer ---
        QTableWidgetItem *buyerItem = new QTableWidgetItem(ad.buyerUsername);
        if (isMyPurchase) {
            buyerItem->setForeground(QBrush(QColor("blue"))); // Highlight "Me"
            buyerItem->setFont(QFont("Arial", 9, QFont::Bold));
        }
        ui->table_history->setItem(row, 4, buyerItem);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshAds() {
    NetworkManager::instance().sendGetAds();
}


void MainWindow::on_Ok_clicked()
{
    QString user = ui->lineEdit->text();

    QString password = ui->lineEdit_2->text();
    QByteArray hashedData = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedSafePassword = hashedData.toHex();

    if(user.isEmpty() || ui->lineEdit_2->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    NetworkManager::instance().sendLogin(user, hashedSafePassword);
}
int clickcount =0;
void MainWindow::on_eye_login_clicked()
{
    if(clickcount==0){
        QPixmap OEyepic(":/Images/open-eye.png");
        ui->eye_login->setIcon(QIcon(OEyepic));
        ui->lineEdit_2->setEchoMode(QLineEdit::Normal);
        clickcount=1;
        return;
    }
    if(clickcount==1){
        QPixmap CEyepic(":/Images/close-eye.png");
        ui->eye_login->setIcon(QIcon(CEyepic));
        ui->lineEdit_2->setEchoMode(QLineEdit::Password);
        clickcount=0;
        return;
    }
}
void MainWindow::on_forgot_pass_clicked()
{
    /*
    QPixmap ResetPass(":/Images/SignUpBack.jpg");

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(ResetPass));
    ui->RestorePass->setPalette(palette);
    ui->RestorePass->setAutoFillBackground(true);*/
    /*QPixmap ResetPass(":/Images/PasswordReset.png");

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(ResetPass));
    ui->RestorePass->setPalette(palette);
    ui->RestorePass->setAutoFillBackground(true);*/
    ui->App->setCurrentIndex(2);
}
void MainWindow::on_SignUp_clicked()
{
    QPixmap ResetPass(":/Images/SignUpBack.jpg");

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(ResetPass));
    ui->SignUpPage->setPalette(palette);
    ui->SignUpPage->setAutoFillBackground(true);
    ui->App->setCurrentIndex(3);
}
void MainWindow::on_Back_clicked()
{
    ui->App->setCurrentIndex(1);
    ui->lineEdit_9->setText("");
    ui->lineEdit_10->setText("");
    ui->lineEdit_11->setText("");
    ui->lineEdit_12->setText("");
    ui->lineEdit_13->setText("");
    ui->lineEdit_14->setText("");
    ui->lineEdit_15->setText("");

}
int clickcount2=0;
void MainWindow::on_eye_signup_clicked()
{
    if(clickcount2==0){
        QPixmap OEyepic(":/Images/open-eye.png");
        ui->eye_signup->setIcon(QIcon(OEyepic));
        ui->lineEdit_13->setEchoMode(QLineEdit::Normal);
        clickcount2=1;
        return;
    }
    if(clickcount2==1){
        QPixmap CEyepic(":/Images/close-eye.png");
        ui->eye_signup->setIcon(QIcon(CEyepic));
        ui->lineEdit_13->setEchoMode(QLineEdit::Password);
        clickcount2=0;
        return;
    }
}
void MainWindow::on_SignupCheck_clicked()
{
    if(ui->lineEdit_9->text().isEmpty()||ui->lineEdit_10->text().isEmpty()||ui->lineEdit_11->text().isEmpty()
        ||ui->lineEdit_12->text().isEmpty()||ui->lineEdit_13->text().isEmpty()||ui->lineEdit_15->text().isEmpty()){
        QMessageBox::warning(this,"Error", "pleas fill in all the blanks\n");
        return;
    }
    QRegularExpression phonenumRegex(R"(\b09\d{9}\b)");
    if (!phonenumRegex.match(ui->lineEdit_11->text()).hasMatch()) {
        QMessageBox::warning(this,"Error", "Invalid phonenumber format\n");
        return;
    }
    QRegularExpression emailRegex(R"((^[^\s@]+@[^\s@]+\.[^\s@]+$))");
    if (!emailRegex.match(ui->lineEdit_12->text()).hasMatch()) {
        QMessageBox::warning(this,"Error", "Invalid email format\n");
        return;
    }
    if(ui->lineEdit_13->text().length()<8){
        QMessageBox::warning(this,"Error", "Password is weak!!\n");
        return;
    }
    if(ui->lineEdit_13->text()!=ui->lineEdit_14->text()){
        QMessageBox::warning(this,"Error", "Password confirmation failed\n");
        return;
    }

    User u;
    u.username = ui->lineEdit_10->text();
    u.passwordHash = hashPassword(ui->lineEdit_13->text()); //
    u.fullName = ui->lineEdit_9->text();
    u.phoneNumber = ui->lineEdit_11->text();
    u.email = ui->lineEdit_12->text();
    u.address = ui->lineEdit_15->text();
    u.walletBalance = 0;
    u.isAdmin = false;


    NetworkManager::instance().sendRegister(u);
}

void MainWindow::handleLoginResponse(bool success, QString message, User user)
{
    if (success) {
        currentUser = user;

        QMessageBox::information(this, "Welcome", "Login Successful!");
        refreshAds();
        ui->App->setCurrentIndex(4);

    } else {
        QMessageBox::critical(this, "Login Failed", message);
    }
}
void MainWindow::handleRegisterResponse(bool success, QString message, User user)
{
    if (success) {
        currentUser = user;

        QMessageBox::information(this, "Success", message);

        refreshAds();
        ui->App->setCurrentIndex(4);
    } else {
        QMessageBox::warning(this, "Registration Failed", message);
    }
}
void MainWindow::onGetAdsResponse(bool success, QString message, QVector<Ad> ads) {
    if (success) {
        this->adList = ads;
        updateAdDisplay();
    }
}

void MainWindow::onUpdateProfileResponse(bool success, QString message)
{
    if (success) {
        QMessageBox::information(this, "Success", "Profile updated successfully!");
        currentUser.fullName = ui->lineEdit_51->text();
        currentUser.phoneNumber = ui->lineEdit_52->text();
        currentUser.email = ui->lineEdit_53->text();
        currentUser.address = ui->lineEdit_55->text();
        ui->App->setCurrentIndex(4);
    } else {
        QMessageBox::critical(this, "Error", "Update failed: " + message);
    }
}

void MainWindow::onCreateAdResponse(bool success, QString message)
{
    ui->NewInform_Ok_2->setEnabled(true); // Re-enable button

    if (success) {
        QMessageBox::information(this, "Success", "Ad submitted! Waiting for Admin approval.");

        // Clear inputs
        ui->Ad_title->clear();
        ui->Ad_price->clear();
        ui->Ad_desc->clear();
        ui->comboBox->clear();


        QPixmap pix(":/Images/image-upload.png");
        pix = pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->pushButton_3->setIcon(QIcon(pix));

        selectedAdImageBase64.clear();
    } else {
        QMessageBox::warning(this, "Failed", "Could not register ad: " + message);
    }
}

void MainWindow::onBuyItemResponse(bool success, QString message)
{
    if (success) {
        QMessageBox::information(this, "Purchase Successful", message);

        shoppingCart.clear();
        refreshCartDisplay();
        currentUser.walletBalance-=to_Pay;
        ui->lineEdit_56->setText(QString("%1").arg(currentUser.walletBalance));
        ui->label_25->setText(QString("%1").arg(currentUser.walletBalance));
        refreshAds();
        ui->App->setCurrentIndex(4);

    } else {
        QMessageBox::warning(this, "Purchase Failed", message);
    }
}

void MainWindow::onDiscountChecked(bool success, QString message, int percent)
{
    // Reset Button UI
    ui->Apply_discount->setText("Apply");
    ui->Apply_discount->setEnabled(true);

    if (success) {
        this->discountFactor = (float)percent / 100.0f;

        QMessageBox::information(this, "Success", QString("Code Applied! You get %1% Off.").arg(percent));

        ui->Discount->setDisabled(true);

        refreshCartDisplay();

    } else {
        this->discountFactor = 0.0f;

        QMessageBox::warning(this, "Error", message);

        refreshCartDisplay();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    refreshAds();
}
void MainWindow::on_All_clicked()
{
    refreshAds();
}
void MainWindow::on_House_clicked()
{
    refreshAds();
}
void MainWindow::on_Vehicles_clicked()
{
    refreshAds();
}
void MainWindow::on_DigitalDevices_clicked()
{
    refreshAds();
}
void MainWindow::on_Kitchen_clicked()
{
    refreshAds();
}
void MainWindow::on_Personalthings_clicked()
{
    refreshAds();
}


void MainWindow::on_pushButton_clicked()
{
    ui->App->setCurrentIndex(5);
}


void MainWindow::on_NewInform_Ok_clicked()
{
    if(ui->lineEdit_51->text().isEmpty()||ui->lineEdit_52->text().isEmpty()||ui->lineEdit_53->text().isEmpty()
        ||ui->lineEdit_55->text().isEmpty()){
        QMessageBox::warning(this,"Error", "pleas fill in all the blanks\n");
        return;
    }
    QRegularExpression phonenumRegex(R"(\b09\d{9}\b)");
    if (!phonenumRegex.match(ui->lineEdit_52->text()).hasMatch()) {
        QMessageBox::warning(this,"Error", "Invalid phonenumber format\n");
        return;
    }
    QRegularExpression emailRegex(R"((^[^\s@]+@[^\s@]+\.[^\s@]+$))");
    if (!emailRegex.match(ui->lineEdit_53->text()).hasMatch()) {
        QMessageBox::warning(this,"Error", "Invalid email format\n");
        return;
    }

    User u;
    u.username = currentUser.username;
    u.passwordHash = currentUser.passwordHash; //
    u.fullName = ui->lineEdit_51->text();
    u.phoneNumber = ui->lineEdit_52->text();
    u.email = ui->lineEdit_53->text();
    u.address = ui->lineEdit_55->text();
    u.walletBalance = currentUser.walletBalance;
    u.isAdmin = false;


    NetworkManager::instance().sendUpdateProfile(u);

}


void MainWindow::on_profile_button_clicked()
{
    ui->App->setCurrentIndex(9);

    QPixmap NewInform(":/Images/Personal_Info_back.png");
    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(NewInform));
    ui->Personal_Info->setPalette(palette);
    ui->Personal_Info->setAutoFillBackground(true);

    ui->lineEdit_50->setText(currentUser.username);
    ui->lineEdit_51->setText(currentUser.fullName);
    ui->lineEdit_52->setText(currentUser.phoneNumber);
    ui->lineEdit_53->setText(currentUser.email);
    ui->lineEdit_55->setText(currentUser.address);
    ui->lineEdit_56->setText(QString::number(currentUser.walletBalance));
}


void MainWindow::on_NewInform_Back_clicked()
{
    ui->App->setCurrentIndex(4);
}


void MainWindow::on_pushButton_3_clicked()
{
    // 1. Open File Dialog
    QString fileName = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.jpeg)");
    if (fileName.isEmpty()) return;

    // 2. Load Image
    QPixmap pix(fileName);
    if (pix.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to load image.");
        return;
    }

    pix = pix.scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->pushButton_3->setIcon(QIcon(pix));

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);

    pix.save(&buffer, "JPG", 50);
    selectedAdImageBase64 = bytes.toBase64();
}


void MainWindow::on_NewInform_Ok_2_clicked()
{
    if (ui->Ad_title->text().isEmpty() ||
        ui->Ad_price->text().isEmpty() ||
        selectedAdImageBase64.isEmpty()||
        ui->comboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "Missing Info", "Please fill title, price, and select an image.");
        return;
    }

    // 2. Create Ad Object
    Ad newAd;
    newAd.title = ui->Ad_title->text();
    newAd.price = ui->Ad_price->text().toLongLong();
    newAd.description = ui->Ad_desc->toPlainText();
    newAd.category = ui->comboBox->currentText();
    newAd.imageBase64 = selectedAdImageBase64;
    newAd.sellerUsername = currentUser.username;
    newAd.status = AdStatus::PENDING;
    selectedAdImageBase64 = "";
    qDebug() << "Received Create Ad Request from:" << newAd.sellerUsername;

    // Try to save it to the database
    qDebug() << newAd.date;
    qDebug() << newAd.category;
    qDebug() << newAd.imageBase64;
    // 3. Send to Server
    NetworkManager::instance().sendCreateAd(newAd);

    // Optional: Disable button to prevent double-click
    ui->NewInform_Ok_2->setEnabled(false);
}


void MainWindow::on_pushButton_4_clicked()
{
    ui->Ad_title->clear();
    ui->Ad_price->clear();
    ui->Ad_desc->clear();
    ui->comboBox->clear();


    QPixmap pix(":/Images/image-upload.png");
    pix = pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->pushButton_3->setIcon(QIcon(pix));

    selectedAdImageBase64.clear();
    ui->App->setCurrentIndex(4);
}

void MainWindow::onAdAddedToCart(Ad ad)
{
    for(const Ad &item : shoppingCart) {
        if(item.id == ad.id) {
            QMessageBox::information(this, "Cart", "Item already in cart.");
            return;
        }
    }
    shoppingCart.append(ad);
    QMessageBox::information(this, "Cart", "Added to Cart!");

    // Optional: Update cart button text
    // ui->btn_cart->setText("Cart (" + QString::number(shoppingCart.size()) + ")");
}

void MainWindow::onRemoveFromCart(QString adId)
{
    for (int i = 0; i < shoppingCart.size(); ++i) {
        if (shoppingCart[i].id == adId) {
            shoppingCart.remove(i);
            break;
        }
    }

    refreshCartDisplay();
}




void MainWindow::on_basket_button_clicked()
{

    ui->App->setCurrentIndex(7);
    ui->Discount->setEnabled(true);
    ui->Discount->clear();
    this->discountFactor = 0.0;


    refreshCartDisplay();
}


void MainWindow::on_pushButton_5_clicked()
{
    QString card = ui->CardNumber->text();
    QString amountStr = ui->lineEdit_5->text();
    QString captchaInput = ui->CaptchaInput->text();

    if (card.length() != 16) {
        QMessageBox::warning(this, "Error", "Card number must be exactly 16 digits.");
        return;
    }
    if (amountStr.toLongLong() <= 0) {
        QMessageBox::warning(this, "Error", "Please enter a valid amount.");
        return;
    }
    if (captchaInput.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter the captcha code.");
        return;
    }
    if(ui->Captcha->text() != ui->CaptchaInput->text()){
        QMessageBox::warning(this, "Error", "Captcha is incorrect.");
        return;
    }

    // 2. Send to Server
    NetworkManager::instance().sendChargeWallet(currentUser.username, card, amountStr.toLongLong(), captchaInput);

    // Disable button to prevent double-click
    ui->pushButton_5->setEnabled(false);
}


void MainWindow::on_pushButton_7_clicked()
{
    if (shoppingCart.isEmpty()) return;

    // Calculate total logic again to be safe
    long long total = 0;
    for(const Ad &a : shoppingCart) total += a.price;

    long long finalTotal = total * (1.0 - discountFactor);

    // Check Wallet
    if (currentUser.walletBalance < finalTotal) {
        QMessageBox::critical(this, "Error", "Insufficient funds.");
        return;
    }

    // Send Requests
    for (const Ad &ad : shoppingCart) {
        // Calculate price for THIS item
        long long itemPrice = ad.price * (1.0 - discountFactor);
        to_Pay = itemPrice;
        // Use updated Network Function
        NetworkManager::instance().sendBuyItem(ad.id, currentUser.username, itemPrice);
    }

    shoppingCart.clear();
    discountFactor = 0.0; // Reset discount
    ui->Discount->clear();
    refreshCartDisplay();
}


void MainWindow::on_pushButton_6_clicked()
{
    refreshAds();
    ui->App->setCurrentIndex(4);
}


void MainWindow::on_pushButton_9_clicked()
{
    NetworkManager::instance().sendGetCaptcha();
}


void MainWindow::on_pushButton_8_clicked()
{
    ui->App->setCurrentIndex(9);
}

void MainWindow::onCaptchaReceived(QString code)
{
    ui->Captcha->setText(code);
    ui->Captcha->setStyleSheet("font-size: 24px; font-weight: bold; color: blue; letter-spacing: 6px;");
}

void MainWindow::onChargeResponse(bool success, QString message, long long newBalance)
{
    ui->pushButton_5->setEnabled(true);

    if (success) {
        QMessageBox::information(this, "Success", "Wallet charged successfully!");

        // 1. Update Local User Data
        currentUser.walletBalance = newBalance;

        ui->lineEdit_56->setText(QString::number(newBalance));
        // 3. Go back to Dashboard
        ui->App->setCurrentIndex(9);
    } else {
        QMessageBox::warning(this, "Failed", message);

        NetworkManager::instance().sendGetCaptcha();
        ui->Captcha->clear();
    }
}

void MainWindow::onGlobalChatReceived(QVector<Message> msgs) {
    ui->list_global_chat->clear();

    // Set List Style (Put this in Constructor ideally)
    ui->list_global_chat->setStyleSheet(
        "QListWidget { background-color: #E5DDD5; border: none; outline: none; }"
        "QListWidget::item { padding: 5px; border: none; }"
        "QListWidget::item:selected { background-color: transparent; }" // No blue selection
        );
    ui->list_global_chat->setSpacing(5);

    for (const Message &m : msgs) {
        bool isMe = (m.sender == currentUser.username);

        // 1. Create Custom Widget
        QWidget *bubbleWidget = createMessageWidget(m, isMe);

        // 2. Create List Item
        QListWidgetItem *item = new QListWidgetItem();

        // 3. Size Hint (Crucial! Tells list how big the widget is)
        item->setSizeHint(bubbleWidget->sizeHint());

        // 4. Add to List
        ui->list_global_chat->addItem(item);
        ui->list_global_chat->setItemWidget(item, bubbleWidget);
    }

    ui->list_global_chat->scrollToBottom();
}




void MainWindow::on_Add_money_clicked()
{

    ui->lineEdit_5->clear();
    ui->CardNumber->clear();
    ui->CaptchaInput->clear();

    NetworkManager::instance().sendGetCaptcha();

    ui->App->setCurrentIndex(8);

}


void MainWindow::on_referesh_clicked()
{
    refreshHistory();
    ui->App->setCurrentIndex(10);

}


void MainWindow::on_pushButton_10_clicked()
{
    ui->App->setCurrentIndex(9);
}


void MainWindow::on_pushButton_11_clicked()
{
    QString text = ui->lineEdit_3->text();
    if (text.isEmpty()) return;

    NetworkManager::instance().sendGlobalMessage(currentUser.username, text);

    ui->lineEdit_3->clear();

    NetworkManager::instance().getGlobalChat();
}


void MainWindow::on_chat_button_clicked()
{
    ui->list_global_chat->clear();
    ui->lineEdit_3->clear();
    NetworkManager::instance().getGlobalChat();
    ui->App->setCurrentIndex(6);

}


void MainWindow::on_pushButton_12_clicked()
{
    ui->list_global_chat->clear();
    ui->lineEdit_3->clear();
    ui->App->setCurrentIndex(4);
}


void MainWindow::on_Apply_discount_clicked()
{
    QString code = ui->Discount->text().trimmed().toUpper();

    if (code.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a discount code.");
        return;
    }

    NetworkManager::instance().sendCheckDiscount(code);

    ui->Apply_discount->setText("Checking...");
    ui->Apply_discount->setEnabled(false);
}


