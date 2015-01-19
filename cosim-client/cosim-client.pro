TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QT -= core gui

SOURCES += main.cpp

include(../cosim-proto/libs.pri)

INCLUDEPATH += ../cosim-proto

QMAKE_CXXFLAGS += -std=c++11 -Werror

target.path=$$PREFIX/bin

INSTALLS+=target
