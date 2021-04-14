QT -= gui

TARGET = qtmips_asm
CONFIG += c++11

TEMPLATE = lib
CONFIG += staticlib
CONFIG -= x86_64
CONFIG += sdk_no_version_check
QMAKE_APPLE_DEVICE_ARCHS=arm64

INCLUDEPATH += $$PWD/../qtmips_machine $$PWD/../qtmips_osemu
DEPENDPATH += $$PWD/../qtmips_machine

LIBS += -lelf
QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS_DEBUG += -ggdb

DEFINES += QTMIPS_MACHINE_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    fixmatheval.cpp \
    simpleasm.cpp

HEADERS += \
    fixmatheval.h \
    messagetype.h \
    simpleasm.h
