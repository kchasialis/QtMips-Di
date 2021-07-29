project(qtmips_machine
        DESCRIPTION "The actual simulator as a library. Link with an UI of your choice.")

set(CMAKE_AUTOMOC ON)

set(qtmips_machine_SOURCES
        branchpredictor.cpp
        branchtargetbuffer.cpp
        qtmipsmachine.cpp
        qtmipsexception.cpp
        core.cpp
        memory.cpp
        instruction.cpp
        registers.cpp
        programloader.cpp
        cache.cpp
        alu.cpp
        machineconfig.cpp
        utils.cpp
        physaddrspace.cpp
        peripheral.cpp
        serialport.cpp
        peripspiled.cpp
        lcddisplay.cpp
        symboltable.cpp
        cop0state.cpp
        )

set(qtmips_machine_HEADERS
        branchpredictor.h
        branchtargetbuffer.h
        qtmipsmachine.h
        qtmipsexception.h
        core.h
        memory.h
        instruction.h
        registers.h
        programloader.h
        cache.h
        alu.h
        machineconfig.h
        utils.h
        machinedefs.h
        physaddrspace.h
        peripheral.h
        serialport.h
        peripspiled.h
        lcddisplay.h
        symboltable.h
        cop0state.h
        )
set(qtmips_machine_TESTS
        tests/testalu.cpp
        tests/testcache.cpp
        tests/testcore.cpp
        tests/testinstruction.cpp
        tests/testmemory.cpp
        tests/testprogramloader.cpp
        tests/testregisters.cpp
        tests/tst_machine.cpp
        tests/tst_machine.h
        )


# Object library is preferred, because the library archive is never really
# needed. This option skips the archive creation and links directly .o files.
add_library(qtmips_machine OBJECT
        ${qtmips_machine_SOURCES}
        ${qtmips_machine_HEADERS})
target_link_libraries(qtmips_machine
        PRIVATE Qt5::Core
        PUBLIC libelf)

if (NOT ${WASM})
    # Machine tests (not available on WASM)
    add_executable(qtmips_machine_unit_tests ${qtmips_machine_TESTS})
    target_link_libraries(qtmips_machine_unit_tests
            PRIVATE qtmips_machine Qt5::Core Qt5::Test)

    add_test(NAME qtmips_machine_unit_tests
            COMMAND qtmips_machine_unit_tests)
endif ()
