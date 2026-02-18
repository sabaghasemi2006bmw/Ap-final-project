QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


SOURCES += \
    ad.cpp \
    clienthandler.cpp \
    databasemanager.cpp \
    main.cpp \
    mainwindow.cpp \
    message.cpp \
    myserver.cpp \
    user.cpp

HEADERS += \
    ad.h \
    clienthandler.h \
    databasemanager.h \
    discountcode.h \
    enums.h \
    mainwindow.h \
    message.h \
    myserver.h \
    user.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Images.qrc
