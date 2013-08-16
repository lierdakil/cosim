TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../cosim-server/linkimplshmem.cpp \
    ../cosim-server/linkimpl.cpp \
    ../cosim-server/link.cpp \
    cosimconnection.cpp

HEADERS += \
    ../cosim-server/linkimplshmem.h \
    ../cosim-server/linkimpl.h \
    ../cosim-server/link.h \
    cosimconnection.h

PROTOS += \
    ../cosim-proto/generalmessage.proto \
    ../cosim-proto/termination.proto \
    ../cosim-proto/stepvalue.proto \
    ../cosim-proto/parametervalues.proto \
    ../cosim-proto/exportrequest.proto \
    ../cosim-proto/init.proto \
    ../cosim-proto/initial.proto \
    ../cosim-proto/messagesize.proto

OTHER_FILES += $$PROTOS

LIBS += -lprotobuf -lboost_system -lboost_random

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
