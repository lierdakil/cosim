TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
#CONFIG -= qt
QT -= core gui

SOURCES += main.cpp

LIBS += -lprotobuf -lboost_system -lboost_random \
    -L$$PWD/../cosim-proto -lcosim-proto

INCLUDEPATH += ../cosim-proto

QMAKE_CXXFLAGS += -std=c++11
