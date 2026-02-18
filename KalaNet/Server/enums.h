#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>

enum class RequestType {
    LOGIN,
    REGISTER,
    UPDATE_PROFILE,

    // Ads
    CREATE_AD,
    GET_ALL_ADS,        // For the main feed (Search/Filter)
    GET_MY_ADS,         // For History
    EDIT_AD,
    ADMIN_ACTION_AD,    // Approve/Reject

    // Shopping
    ADD_TO_CART,
    REMOVE_FROM_CART,
    CHECKOUT,
    BUY_ITEM,      // Client sends Ad ID

    // Wallet
    GET_CAPTCHA,
    CHARGE_WALLET,

    // Bonus Features
    SEND_GLOBAL_MESSAGE, // Send a message to the public room
    GET_GLOBAL_CHAT,
    CHECK_DISCOUNT_CODE
};

enum class AdStatus {
    PENDING,    // Waiting for Admin
    APPROVED,   // Visible in shop
    REJECTED,   // Admin said no
    SOLD        // Bought by someone
};


#endif // ENUMS_H
