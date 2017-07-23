#-------------------------------------------------
#
# Project created by QtCreator 2016-07-08T19:32:00
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SingleUp
TEMPLATE = app


SOURCES += main.cpp\
        Singleup.cpp \
    Crc16.cpp \
    updatethread.cpp \
    Production.cpp

HEADERS  += Singleup.h \
    Crc16.h \
    updatethread.h \
    Production.h

FORMS    += Singleup.ui

RC_FILE = sigle.rc

RESOURCES += \
    status.qrc
