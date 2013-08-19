TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
#CONFIG -= qt
QT -= core gui

SOURCES += main.cpp

LIBS += -lcosim-proto \
    -L$$PWD/../cosim-proto

LIBS += -lprotobuf -lboost_system -lboost_random

INCLUDEPATH += ../cosim-proto

QMAKE_CXXFLAGS += -std=c++11
