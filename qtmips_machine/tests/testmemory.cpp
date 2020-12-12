// SPDX-License-Identifier: GPL-2.0+
/*******************************************************************************
 * QtMips - MIPS 32-bit Architecture Subset Simulator
 *
 * Implemented to support following courses:
 *
 *   B35APO - Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b35apo
 *
 *   B4M35PAP - Advanced Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b4m35pap/start
 *
 * Copyright (c) 2017-2019 Karel Koci<cynerd@email.cz>
 * Copyright (c) 2019      Pavel Pisa <pisa@cmp.felk.cvut.cz>
 *
 * Faculty of Electrical Engineering (http://www.fel.cvut.cz)
 * Czech Technical University        (http://www.cvut.cz/)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ******************************************************************************/

#include "tst_machine.h"
#include "memory.h"

using namespace machine;

void MachineTests::memory_data() {
    QTest::addColumn<std::uint32_t>("address");

    QTest::newRow("memory begin") << (std::uint32_t)0x00;
    QTest::newRow("memory end") << (std::uint32_t)0xFFFFFFFC;
    QTest::newRow("memory midle start") << (std::uint32_t)0xFFFF00;
    QTest::newRow("memory midle end") << (std::uint32_t)0xFFFFFF;
}

void MachineTests::memory() {
    Memory m;

    QFETCH(std::uint32_t, address);

    // Uninitialize memory should read as zero
    QCOMPARE(m.read_byte(address), (std::uint8_t)0);
    QCOMPARE(m.read_hword(address), (std::uint16_t)0);
    QCOMPARE(m.read_word(address), (std::uint32_t)0);
    // Just a byte
    m.write_byte(address, 0x42);
    QCOMPARE(m.read_byte(address), (std::uint8_t)0x42);
    // Half word
    m.write_hword(address, 0x4243);
    QCOMPARE(m.read_hword(address), (std::uint16_t)0x4243);
    // Word
    m.write_word(address, 0x42434445);
    QCOMPARE(m.read_word(address), (std::uint32_t)0x42434445);
}

void MachineTests::memory_section_data() {
    QTest::addColumn<std::uint32_t>("address");

    QTest::newRow("memory begin") << (std::uint32_t)0x00;
    QTest::newRow("memory end") << (std::uint32_t)0xFFFFFFFF;
    QTest::newRow("memory midle start") << (std::uint32_t)0xFFFF00;
    QTest::newRow("memory midle end") << (std::uint32_t)0xFFFFFF;
}

void MachineTests::memory_section() {
    Memory m;

    QFETCH(std::uint32_t, address);

    // First section shouldn't exists QCOMPARE(m.get_section(address, false), (MemorySection*)nullptr);
    // Create section
    MemorySection *s = m.get_section(address, true);
    QVERIFY(s != nullptr);

    // Write some data to memory
    m.write_byte(address, 0x42);
    // Read it trough section (mask bits outside of the memory section)
    QCOMPARE(s->read_byte(address & 0xFF), (std::uint8_t)0x42);
    // Write some other data trough section
    s->write_byte(address & 0xFF, 0x66);
    // Read trough memory
    QCOMPARE(m.read_byte(address), (std::uint8_t)0x66);
}

void MachineTests::memory_endian() {
    Memory m;

    // Memory should be big endian so write bytes from most significant byte
    m.write_byte(0x00, 0x12);
    m.write_byte(0x01, 0x34);
    m.write_byte(0x02, 0x56);
    m.write_byte(0x03, 0x78);
    QCOMPARE(m.read_hword(0x00), (std::uint16_t)0x1234);
    QCOMPARE(m.read_word(0x00), (std::uint32_t)0x12345678);

    m.write_hword(0x80, 0x1234);
    QCOMPARE(m.read_byte(0x80), (std::uint8_t)0x12);
    QCOMPARE(m.read_byte(0x81), (std::uint8_t)0x34);

    m.write_word(0xF0, 0x12345678);
    QCOMPARE(m.read_byte(0xF0), (std::uint8_t)0x12);
    QCOMPARE(m.read_byte(0xF1), (std::uint8_t)0x34);
    QCOMPARE(m.read_byte(0xF2), (std::uint8_t)0x56);
    QCOMPARE(m.read_byte(0xF3), (std::uint8_t)0x78);
}

void MachineTests::memory_compare() {
    Memory m1, m2;
    QCOMPARE(m1, m2);
    m1.write_byte(0x20,0x0);
    QVERIFY(m1 != m2); // This should not be equal as this identifies also memory write (difference between no write and zero write)
    m1.write_byte(0x20,0x24);
    QVERIFY(m1 != m2);
    m2.write_byte(0x20,0x23);
    QVERIFY(m1 != m2);
    m2.write_byte(0x20,0x24);
    QCOMPARE(m1, m2);
    // Do the same with some other section
    m1.write_byte(0xFFFF20, 0x24);
    QVERIFY(m1 != m2);
    m2.write_byte(0xFFFF20, 0x24);
    QCOMPARE(m1, m2);
    // And also check memory copy
    Memory m3(m1);
    QCOMPARE(m1, m3);
    m3.write_byte(0x18, 0x22);
    QVERIFY(m1 != m3);
}

void MachineTests::memory_write_ctl_data() {
    QTest::addColumn<AccessControl>("ctl");
    QTest::addColumn<Memory>("result");

    Memory mem;
    QTest::newRow("none") << AC_NONE \
                          << mem;
    mem.write_byte(0x20, 0x26);
    QTest::newRow("byte") << AC_BYTE \
                          << mem;
    QTest::newRow("byte-unsigned") << AC_BYTE_UNSIGNED \
                          << mem;
    mem.write_hword(0x20, 0x2526);
    QTest::newRow("halfword") << AC_HALFWORD \
                          << mem;
    QTest::newRow("haldword-unsigned") << AC_HALFWORD_UNSIGNED \
                          << mem;
    mem.write_word(0x20, 0x23242526);
    QTest::newRow("word") << AC_WORD \
                          << mem;
}

void MachineTests::memory_write_ctl() {
    QFETCH(AccessControl, ctl);
    QFETCH(Memory, result);

    Memory mem;
    mem.write_ctl(ctl, 0x20, 0x23242526);
    QCOMPARE(mem, result);
}

void MachineTests::memory_read_ctl_data() {
    QTest::addColumn<AccessControl>("ctl");
    QTest::addColumn<std::uint32_t>("result");

    QTest::newRow("none") << AC_NONE \
                          << (std::uint32_t)0;
    QTest::newRow("byte") << AC_BYTE \
                          << (std::uint32_t)0xFFFFFFA3;
    QTest::newRow("halfword") << AC_HALFWORD \
                          << (std::uint32_t)0xFFFFA324;
    QTest::newRow("word") << AC_WORD \
                          << (std::uint32_t)0xA3242526;
    QTest::newRow("byte-unsigned") << AC_BYTE_UNSIGNED \
                          << (std::uint32_t)0xA3;
    QTest::newRow("halfword-unsigned") << AC_HALFWORD_UNSIGNED \
                          << (std::uint32_t)0xA324;
}

void MachineTests::memory_read_ctl() {
    QFETCH(AccessControl, ctl);
    QFETCH(std::uint32_t, result);

    Memory mem;
    mem.write_word(0x20, 0xA3242526);
    QCOMPARE(mem.read_ctl(ctl, 0x20), result);
}
