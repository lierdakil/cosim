#-------------------------------------------------
#
# Project created by QtCreator 2013-08-19T16:20:14
#
#-------------------------------------------------

QT       -= core gui
CONFIG -= qt

TARGET = cosim-proto
TEMPLATE = lib

DEFINES += COSIMPROTO_LIBRARY

SOURCES += \
    cosimconnection.cpp \
    linkimplshmem.cpp \
    linkimpl.cpp \
    link.cpp

HEADERS +=\
        cosim-proto_global.h \
    cosimconnection.h \
    link.h \
    linkimpl.h \
    linkimplshmem.h

target.path=$$PREFIX/lib

INSTALLS+=target

PROTOS += \
    generalmessage.proto \
    termination.proto \
    stepvalue.proto \
    parametervalues.proto \
    exportrequest.proto \
    init.proto \
    initial.proto \
    messagesize.proto

OTHER_FILES += $$PROTOS \
    libs.pri

QMAKE_CXXFLAGS += -std=c++11

message("Generating protocol buffer classes from .proto files.")

protobuf_decl.name = protobuf headers
protobuf_decl.input = PROTOS
protobuf_decl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
protobuf_decl.commands = protoc --cpp_out=${QMAKE_FILE_IN_PATH} --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
protobuf_decl.variable_out = HEADERS
protobuf_decl.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf sources
protobuf_impl.input = PROTOS
protobuf_impl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\n)
protobuf_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl
