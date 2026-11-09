QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    book_core.c \
    login.cpp \
    main.cpp \
    mainwindow.cpp \
    user_core.c \
    welcome.cpp

HEADERS += \
    book_core.h \
    common.h \
    login.h \
    mainwindow.h \
    user_core.h \
    welcome.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
