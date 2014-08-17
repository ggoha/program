QT       += core
QT       += network
QT       += gui
QT       += sql

TARGET = test2
CONFIG   -= console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    UdpServer.cpp \
    User.cpp \
    StartDialog.cpp

HEADERS += \
    UdpServer.h \
    User.h \
    StartDialog.h
