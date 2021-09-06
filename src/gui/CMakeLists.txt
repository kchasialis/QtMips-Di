project(qtmips_gui
        VERSION 0.8.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(qtmips_gui_SOURCES
        coreview/programcounter.cpp
        coreview/multiplexer.cpp
        coreview/connection.cpp
        coreview/latch.cpp
        coreview/alu.cpp
        coreview/memory.cpp
        coreview/instructionview.cpp
        coreview/registers.cpp
        coreview/adder.cpp
        coreview/constant.cpp
        coreview/junction.cpp
        coreview/logicblock.cpp
        coreview/and.cpp
        coreview/branchpredictor.cpp
        branchtargetbufferdock.cpp
        branchtargetbuffermodel.cpp
        branchtargetbuffertableview.cpp
        branchhistorytabledock.cpp
        branchhistorytablemodel.cpp
        branchhistorytabletableview.cpp
        comboboxitemdelegate.cpp
        coreview/branchtargetbuffer.cpp
        main.cpp
        mainwindow.cpp
        newdialog.cpp
        coreview.cpp
        registersdock.cpp
        programdock.cpp
        memorydock.cpp
        statictable.cpp
        cacheview.cpp
        cachedock.cpp
        graphicsview.cpp
        coreview/value.cpp
        memorymodel.cpp
        memorytableview.cpp
        hexlineedit.cpp
        programmodel.cpp
        programtableview.cpp
        aboutdialog.cpp
        peripheralsdock.cpp
        terminaldock.cpp
        peripheralsview.cpp
        lcddisplaydock.cpp
        lcddisplayview.cpp
        coreview/multitext.cpp
        fontsize.cpp
        gotosymboldialog.cpp
        cop0dock.cpp
        hinttabledelegate.cpp
        coreview/minimux.cpp
        srceditor.cpp
        highlighterasm.cpp
        highlighterc.cpp
        messagesdock.cpp
        messagesmodel.cpp
        messagesview.cpp
        extprocess.cpp
        savechangeddialog.cpp
        textsignalaction.cpp
        )
set(qtmips_gui_HEADERS
        coreview/programcounter.h
        coreview/multiplexer.h
        coreview/connection.h
        coreview/latch.h
        coreview/alu.h
        coreview/memory.h
        coreview/instructionview.h
        coreview/registers.h
        coreview/adder.h
        coreview/constant.h
        coreview/junction.h
        coreview/logicblock.h
        coreview/and.h
        coreview/branchpredictor.h
        branchtargetbufferdock.h
        branchtargetbuffermodel.h
        branchtargetbuffertableview.h
        branchhistorytabledock.h
        branchhistorytablemodel.h
        branchhistorytabletableview.h
        comboboxitemdelegate.h
        coreview/branchtargetbuffer.h
        mainwindow.h
        newdialog.h
        coreview.h
        registersdock.h
        programdock.h
        memorydock.h
        statictable.h
        cacheview.h
        cachedock.h
        graphicsview.h
        coreview/value.h
        memorymodel.h
        memorytableview.h
        hexlineedit.h
        programmodel.h
        programtableview.h
        aboutdialog.h
        peripheralsdock.h
        terminaldock.h
        peripheralsview.h
        lcddisplaydock.h
        lcddisplayview.h
        coreview/multitext.h
        fontsize.h
        gotosymboldialog.h
        cop0dock.h
        hinttabledelegate.h
        coreview/minimux.h
        srceditor.h
        highlighterasm.h
        highlighterc.h
        messagesdock.h
        messagesmodel.h
        messagesview.h
        extprocess.h
        savechangeddialog.h
        textsignalaction.h
        )
set(qtmips_gui_UI
        gotosymboldialog.ui
        NewDialog.ui
        peripheralsview.ui
        MainWindow.ui
        NewDialogCache.ui
        )
set(qtmips_gui_RESOURCES
        icons.qrc
        samples.qrc
        )

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Emscripten")
    message("qtmips_gui :: Including WASM only files.")
    list(APPEND qtmips_gui_SOURCES qhtml5file_html5.cpp)
    list(APPEND qtmips_gui_HEADERS qhtml5file.h)
endif ()


add_executable(qtmips_gui
        ${qtmips_gui_SOURCES}
        ${qtmips_gui_HEADERS}
        ${qtmips_gui_UI}
        ${qtmips_gui_RESOURCES})
target_compile_definitions(qtmips_gui
        PRIVATE
        APP_ORGANIZATION=\"CTU\"
        APP_ORGANIZATION_DOMAIN=\"https://fel.cvut.cz/\"
        APP_NAME=\"QtMips\"
        APP_VERSION=\"${PROJECT_VERSION}\"
        ENV_CONFIG_FILE_NAME=\"QTMIPS_CONFIG_FILE\")
target_include_directories(qtmips_gui PUBLIC . coreview)
target_link_libraries(qtmips_gui
        PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui
        PRIVATE qtmips_machine qtmips_osemu qtmips_asm
        PUBLIC m)

# =============================================================================
# Installation
# =============================================================================

# Prior to CMake version 3.13, installation must be performed in the subdirectory,
# there the target was created. Therefore executable installation is to be found
# in corresponding CMakeLists.txt.

install(TARGETS qtmips_gui
RUNTIME DESTINATION bin
BUNDLE DESTINATION ${EXECUTABLE_OUTPUT_PATH}
)
