TEMPLATE = subdirs

SUBDIRS += \
    qtmips_machine \
    qtmips_osemu \
    qtmips_asm

SUBDIRS += \
    qtmips_gui

qtmips_gui.depends = qtmips_machine qtmips_osemu qtmips_asm
qtmips_machine-tests.depends = qtmips_machine
