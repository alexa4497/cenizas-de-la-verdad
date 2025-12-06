QT          += core gui multimedia

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fragmentos.cpp \
    fuego.cpp \
    inquisidor.cpp \
    main.cpp \
    mainwindow.cpp \
    niveldos.cpp \
    niveltres.cpp \
    niveluno.cpp \
    obstaculos.cpp \
    personajes.cpp

HEADERS += \
    fragmentos.h \
    fuego.h \
    inquisidor.h \
    mainwindow.h \
    niveldos.h \
    niveltres.h \
    niveluno.h \
    ob.h \
    obstaculos.h \
    personajes.h

FORMS += \
    mainwindow.ui \
    niveluno.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    multimedia.qrc
