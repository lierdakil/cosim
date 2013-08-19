TEMPLATE = app
CONFIG += console ordered
CONFIG -= app_bundle
#CONFIG -= qt
QT -= gui core

SOURCES += main.cpp \
    initialconnectionhandler.cpp \
    guestlinkhandler.cpp

LIBS += -lcosim-proto \
    -L$$PWD/../cosim-proto

LIBS += -lprotobuf -lboost_system -lboost_random

INCLUDEPATH += ../cosim-proto

QMAKE_CXXFLAGS += -std=c++11

HEADERS += \
    initialconnectionhandler.h \
    guestlinkhandler.h
