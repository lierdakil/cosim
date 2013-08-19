TEMPLATE = app
CONFIG += console ordered
CONFIG -= app_bundle
#CONFIG -= qt
QT -= gui core

SOURCES += main.cpp \
    initialconnectionhandler.cpp \
    guestlinkhandler.cpp

include(../cosim-proto/libs.pri)

INCLUDEPATH += ../cosim-proto

QMAKE_CXXFLAGS += -std=c++11

HEADERS += \
    initialconnectionhandler.h \
    guestlinkhandler.h
