#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "message.h"
#include "user.h"
#include "ad.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void updateAdDisplay();
    void refreshCartDisplay();
    void updateCartCount();
    void refreshHistory();
    ~MainWindow();

private slots:
    void on_Ok_clicked();

    void on_eye_login_clicked();

    void on_forgot_pass_clicked();

    void on_SignUp_clicked();

    void on_Back_clicked();

    void on_eye_signup_clicked();

    void on_SignupCheck_clicked();

    void handleLoginResponse(bool success, QString message, User user);

    void handleRegisterResponse(bool success, QString message, User user);

    void onGetAdsResponse(bool success, QString message, QVector<Ad> ads);
    void onUpdateProfileResponse(bool success, QString message);
    void onCreateAdResponse(bool success, QString message);
    void onBuyItemResponse(bool success, QString message);

    void refreshAds();

    void on_pushButton_2_clicked();

    void on_All_clicked();

    void on_House_clicked();

    void on_Vehicles_clicked();

    void on_DigitalDevices_clicked();

    void on_Kitchen_clicked();

    void on_Personalthings_clicked();

    void on_pushButton_clicked();

    void on_NewInform_Ok_clicked();

    void on_profile_button_clicked();

    void on_NewInform_Back_clicked();

    void on_pushButton_3_clicked();

    void on_NewInform_Ok_2_clicked();


    void on_pushButton_4_clicked();

    void onAdAddedToCart(Ad ad);
    void onRemoveFromCart(QString adId);

    void on_basket_button_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_8_clicked();

    void onCaptchaReceived(QString code);
    void onChargeResponse(bool success, QString message, long long newBalance);
    void onGlobalChatReceived(QVector<Message> msgs);

    void on_Add_money_clicked();

    void on_referesh_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_chat_button_clicked();

    void on_pushButton_12_clicked();

private:
    Ui::MainWindow *ui;
    QVBoxLayout *scrollLayout;
    User currentUser;
    QVector<Ad> adList;
    QVector<Ad> shoppingCart;
    float discountFactor = 0.0f;
    QString selectedAdImageBase64;
    long long to_Pay;
};
#endif // MAINWINDOW_H
