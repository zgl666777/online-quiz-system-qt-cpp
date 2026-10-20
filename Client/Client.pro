QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    brainstorm.cpp \
    communicate.cpp \
    login.cpp \
    main.cpp \
    register.cpp

HEADERS += \
    brainstorm.h \
    communicate.h \
    login.h \
    register.h

FORMS += \
    brainstorm.ui \
    login.ui \
    register.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
