TEMPLATE = subdirs

SUBDIRS += \
    qtmips_machine \
    qtmips_osemu \
    qtmips_asm

SUBDIRS += \
    qtmips_gui

# TODO: remove these when ready.
CONFIG -= x86_64
QMAKE_APPLE_DEVICE_ARCHS=arm64
CONFIG += sdk_no_version_check

qtmips_gui.depends = qtmips_machine qtmips_osemu qtmips_asm
qtmips_machine-tests.depends = qtmips_machine
