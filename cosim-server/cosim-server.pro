TEMPLATE = app
CONFIG += console ordered
CONFIG -= app_bundle
#CONFIG -= qt
QT -= gui core

SOURCES += main.cpp \
    initialconnectionhandler.cpp \
    guestlinkhandler.cpp

LIBS += -lprotobuf -lboost_coroutine -lboost_system -lboost_context -lboost_random -lcosim-proto \
    -L$$PWD/../cosim-proto -lcosim-proto

INCLUDEPATH += ../cosim-proto

QMAKE_CXXFLAGS += -std=c++11

HEADERS += \
    initialconnectionhandler.h \
    guestlinkhandler.h
