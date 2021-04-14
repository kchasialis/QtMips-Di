QT -= gui

TARGET = qtmips_osemu
CONFIG += c++11

TEMPLATE = lib
CONFIG += staticlib
CONFIG -= x86_64
CONFIG += sdk_no_version_check
QMAKE_APPLE_DEVICE_ARCHS=arm64

LIBS += -L$$OUT_PWD/../qtmips_machine/ -lqtmips_machine

INCLUDEPATH += $$PWD/../qtmips_machine $$PWD/../qtmips_osemu
DEPENDPATH += $$PWD/../qtmips_machine

LIBS += -lelf
QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS_DEBUG += -ggdb

DEFINES += QTMIPS_OSEMU_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    ossyscall.cpp

HEADERS += \
    ossyscall.h \
    syscall_nr.h \
    target_errno.h
