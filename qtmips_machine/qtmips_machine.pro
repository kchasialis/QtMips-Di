QT -= gui

TARGET = qtmips_machine
CONFIG += c++11

TEMPLATE = lib
CONFIG += staticlib

# TODO: remove these and leave only -lelf
QMAKE_APPLE_DEVICE_ARCHS=arm64
CONFIG += sdk_no_version_check
INCLUDEPATH += /opt/homebrew/Cellar/libelf/include
QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS_DEBUG += -ggdb

DEFINES += QTMIPS_MACHINE_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    branchpredictor.cpp \
    branchtargetbuffer.cpp \
    qtmipsmachine.cpp \
    qtmipsexception.cpp \
    core.cpp \
    memory.cpp \
    instruction.cpp \
    registers.cpp \
    programloader.cpp \
    cache.cpp \
    alu.cpp \
    machineconfig.cpp \
    utils.cpp \
    physaddrspace.cpp \
    peripheral.cpp \
    serialport.cpp \
    peripspiled.cpp \
    lcddisplay.cpp \
    symboltable.cpp \
    cop0state.cpp

HEADERS += \
    branchpredictor.h \
    branchtargetbuffer.h \
    qtmipsmachine.h \
    qtmipsexception.h \
    core.h \
    memory.h \
    instruction.h \
    registers.h \
    programloader.h \
    cache.h \
    alu.h \
    machineconfig.h \
    utils.h \
    machinedefs.h \
    physaddrspace.h \
    peripheral.h \
    serialport.h \
    peripspiled.h \
    lcddisplay.h \
    symboltable.h \
    cop0state.h
