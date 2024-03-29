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

#include <QTime>
#include "branchpredictor.h"
#include "qtmipsmachine.h"
#include "programloader.h"

using namespace machine;

CycleStatistics cycle_stats;

QtMipsMachine::QtMipsMachine(const MachineConfig &cc, bool load_symtab, bool load_executable) :
                             QObject(), mcnf(cc) {
    MemoryAccess *cpu_mem, *core_mem_data, *core_mem_program;
    std::uint32_t min_cache_row_size;

    stat = ST_READY;
    symtab = nullptr;

    regs = new Registers();
    if (load_executable) {
        ProgramLoader program(cc.elf());
        mem_program_only = new Memory(cc.ram_access_read(), cc.ram_access_write(), cc.ram_access_burst());
        program.to_memory(mem_program_only);
        if (load_symtab)
            symtab = program.get_symbol_table();
        program_end = program.end();
        if (program.get_executable_entry())
            regs->pc_abs_jmp(program.get_executable_entry());
        mem = new Memory(*mem_program_only);
    } else {
        program_end = 0xf0000000;
        mem_program_only = nullptr;
        mem = new Memory(cc.ram_access_read(), cc.ram_access_write(), cc.ram_access_burst());
    }

    physaddrspace = new PhysAddrSpace(cc.ram_access_read(), cc.ram_access_write(), cc.ram_access_burst());
    physaddrspace->insert_range(mem, 0x00000000, 0xefffffff, false);
    cpu_mem = physaddrspace;

    ser_port = new SerialPort();
    addressapce_insert_range(ser_port, 0xffffc000, 0xffffc03f, true);
    addressapce_insert_range(ser_port, 0xffff0000, 0xffff003f, false);
    connect(ser_port, SIGNAL(signal_interrupt(uint,bool)),
            this, SIGNAL(set_interrupt_signal(uint,bool)));

    perip_spi_led = new PeripSpiLed();
    addressapce_insert_range(perip_spi_led, 0xffffc100, 0xffffc1ff, true);

    perip_lcd_display = new LcdDisplay();
    addressapce_insert_range(perip_lcd_display, 0xffe00000, 0xffe4afff, true);

    l2_unified = new Cache(cc.l2_unified_cache(), cpu_mem, cc.l2_unified_cache().mem_access_read(),
                           cc.l2_unified_cache().mem_access_write(), cc.l2_unified_cache().mem_access_burst(),
                           cc.ram_access_read(), cc.ram_access_write(), cc.ram_access_burst());
    if (cc.l2_unified_cache().enabled()) {
        SANITY_ASSERT(cc.l1_data_cache().enabled() || cc.l1_program_cache().enabled(), "L2 cache is enabled but none of L1 caches are enabled!");
    }

    if (cc.l2_unified_cache().enabled()) {
        l1_program = new Cache(cc.l1_program_cache(), l2_unified, cc.l1_program_cache().mem_access_read(),
                               cc.l1_program_cache().mem_access_write(), cc.l1_program_cache().mem_access_burst(),
                               cc.l2_unified_cache().mem_access_read(), cc.l2_unified_cache().mem_access_write(),
                               cc.l2_unified_cache().mem_access_burst());
        l1_data = new Cache(cc.l1_data_cache(), l2_unified, cc.l1_data_cache().mem_access_read(),
                            cc.l1_data_cache().mem_access_write(), cc.l1_data_cache().mem_access_burst(),
                            cc.l2_unified_cache().mem_access_read(), cc.l2_unified_cache().mem_access_write(),
                            cc.l2_unified_cache().mem_access_burst());
    } else {
        l1_program = new Cache(cc.l1_program_cache(), cpu_mem, cc.l1_program_cache().mem_access_read(),
                               cc.l1_program_cache().mem_access_write(), cc.l1_program_cache().mem_access_burst(),
                               cc.ram_access_read(), cc.ram_access_write(), cc.ram_access_burst());
        l1_data = new Cache(cc.l1_data_cache(), cpu_mem, cc.l1_data_cache().mem_access_read(),
                            cc.l1_data_cache().mem_access_write(), cc.l1_data_cache().mem_access_burst(),
                            cc.ram_access_read(), cc.ram_access_write(), cc.ram_access_burst());
    }

    core_mem_program = cc.l1_program_cache().enabled() ? l1_program : cpu_mem;
    core_mem_data = cc.l1_data_cache().enabled() ? l1_data : cpu_mem;

    min_cache_row_size = 16;
    if (cc.l1_data_cache().enabled())
        min_cache_row_size = cc.l1_data_cache().blocks() * 4;
    if (cc.l1_program_cache().enabled() &&
        cc.l1_program_cache().blocks() < min_cache_row_size)
        min_cache_row_size = cc.l1_program_cache().blocks() * 4;

    cop0st = new Cop0State();

    if (cc.pipelined()) {
        // Control hazard unit cannot be none if we are in pipeline mode.
        SANITY_ASSERT(cc.control_hazard_unit() != MachineConfig::CHU_NONE, "Invalid configuration for control branch unit.");
        cr = new CorePipelined(regs, core_mem_program, core_mem_data, cpu_mem, cc.l1_data_cache().enabled(),
                               cc.l1_program_cache().enabled(), cc.trace(), cc.data_hazard_unit(),
                               cc.control_hazard_unit(), cc.bht_bits(), cc.branch_res_id(),
                               min_cache_row_size, cop0st);
    }
    else {
        SANITY_ASSERT(cc.control_hazard_unit() == MachineConfig::CHU_NONE
                        || cc.control_hazard_unit() == MachineConfig::CHU_DELAY_SLOT, "Invalid configuration for control branch unit.");
        cr = new CoreSingle(regs, core_mem_program, core_mem_data,
                            cc.control_hazard_unit() == machine::MachineConfig::CHU_DELAY_SLOT,
                            cc.trace(), min_cache_row_size, cop0st);
    }

    connect(this, SIGNAL(set_interrupt_signal(uint,bool)),
            cop0st, SLOT(set_interrupt_signal(uint,bool)));

    run_t = new QTimer(this);
    set_speed(0); // In default run as fast as possible
    connect(run_t, SIGNAL(timeout()), this, SLOT(step_timer()));

    for (int i = 0; i < EXCAUSE_COUNT; i++) {
         if (i != EXCAUSE_INT && i != EXCAUSE_BREAK && i != EXCAUSE_HWBREAK) {
             set_stop_on_exception((enum ExceptionCause)i, cc.osemu_exception_stop());
             set_step_over_exception((enum ExceptionCause)i, cc.osemu_exception_stop());
         }
    }

    set_stop_on_exception(EXCAUSE_INT, cc.osemu_interrupt_stop());
    set_step_over_exception(EXCAUSE_INT, false);
}

QtMipsMachine::~QtMipsMachine() {
    delete run_t;
    delete cr;
    delete cop0st;
    delete regs;
    delete mem;
    delete l1_program;
    delete l1_data;
    delete l2_unified;
    delete physaddrspace;
    delete mem_program_only;
    delete symtab;
}

const MachineConfig &QtMipsMachine::config() const {
    return mcnf;
}

void QtMipsMachine::set_speed(unsigned int ips, unsigned int time_chunk) {
    this->time_chunk = time_chunk;
    run_t->setInterval(ips);
}

const Registers *QtMipsMachine::registers() {
    return regs;
}

const Cop0State *QtMipsMachine::cop0state() {
    return cop0st;
}

const Memory *QtMipsMachine::memory() {
    return mem;
}

Memory *QtMipsMachine::memory_rw() {
    return mem;
}

const Cache *QtMipsMachine::l1_program_cache() const {
    return l1_program;
}

const Cache *QtMipsMachine::l1_data_cache() const {
    return l1_data;
}

const Cache *QtMipsMachine::l2_unified_cache() const {
    return l2_unified;
}

Cache *QtMipsMachine::l1_data_cache_rw() const {
    return l1_data;
}

Cache *QtMipsMachine::l2_unified_cache_rw() const {
    return l2_unified;
}

void QtMipsMachine::mem_sync() {
    if (l1_program != nullptr)
        l1_program->sync();
    if (l1_data != nullptr)
        l1_data->sync();
    if (l2_unified != nullptr)
        l2_unified->sync();
}

const PhysAddrSpace *QtMipsMachine::physical_address_space() {
    return physaddrspace;
}

PhysAddrSpace *QtMipsMachine::physical_address_space_rw() {
    return physaddrspace;
}

SerialPort *QtMipsMachine::serial_port() {
    return ser_port;
}

PeripSpiLed *QtMipsMachine::peripheral_spi_led() {
    return perip_spi_led;
}

LcdDisplay *QtMipsMachine::peripheral_lcd_display() {
    return perip_lcd_display;
}

const SymbolTable *QtMipsMachine::symbol_table(bool create) {
    return symbol_table_rw(create);
}

SymbolTable *QtMipsMachine::symbol_table_rw(bool create) {
    if (create && (symtab == nullptr))
        symtab = new SymbolTable;
    return symtab;
}

void QtMipsMachine::set_symbol(QString name, std::uint32_t value,
                               size_t size, std::uint8_t info,
                               std::uint8_t other) {
    if (symtab == nullptr)
        symtab = new SymbolTable;
    symtab->set_symbol(name, value, size, info, other);
}

const Core *QtMipsMachine::core() {
    return cr;
}

const CoreSingle *QtMipsMachine::core_singe() {
    return mcnf.pipelined() ? nullptr : (const CoreSingle*)cr;
}

const CorePipelined *QtMipsMachine::core_pipelined() {
    return mcnf.pipelined() ? (const CorePipelined*)cr : nullptr;
}

bool QtMipsMachine::executable_loaded() const {
    return mem_program_only != nullptr;
}

enum QtMipsMachine::Status QtMipsMachine::status() {
    return stat;
}

bool QtMipsMachine::exited() {
    return stat == ST_EXIT || stat == ST_TRAPPED;
}

// We don't allow to call control methods when machine exited or if it's busy
// We rather silently fail.
#define CTL_GUARD do { if (exited() || stat == ST_BUSY) return; } while(false)

void QtMipsMachine::play() {
    CTL_GUARD;
    set_status(ST_RUNNING);
    run_t->start();
    step_internal(true);
}

void QtMipsMachine::pause() {
    if (stat != ST_BUSY)
        CTL_GUARD;
    set_status(ST_READY);
    run_t->stop();
}

void QtMipsMachine::step_internal(bool skip_break) {

    CTL_GUARD;
    enum Status stat_prev = stat;
    set_status(ST_BUSY);
    emit tick();
    try {
        QTime start_time = QTime::currentTime();
        do {
            cr->step(skip_break);
            // Update cycles for cache misses in every step
            emit cycle_stats_update(cycle_stats);
        } while(time_chunk != 0 && stat == ST_BUSY && skip_break == false &&
                start_time.msecsTo(QTime::currentTime()) < (int)time_chunk);
    } catch (QtMipsException &e) {
        run_t->stop();
        set_status(ST_TRAPPED);
        emit program_trap(e);
        return;
    }
    if (regs->read_pc() >= program_end) {
        run_t->stop();
        set_status(ST_EXIT);
        emit program_exit();
    } else {
        if (stat == ST_BUSY)
            set_status(stat_prev);
    }
    emit post_tick();
}

void QtMipsMachine::step() {
    step_internal(true);
}

void QtMipsMachine::step_timer() {
    step_internal();
}

void QtMipsMachine::restart() {
    pause();
    regs->reset();
    if (mem_program_only != nullptr)
        mem->reset(*mem_program_only);
    l1_program->reset();
    l1_data->reset();
    l2_unified->reset();
    cr->reset();
    set_status(ST_READY);
}

void QtMipsMachine::set_status(enum Status st) {
    bool change = st != stat;
    stat = st;
    if (change)
        emit status_change(st);
}

void QtMipsMachine::register_exception_handler(ExceptionCause excause,
                                               ExceptionHandler *exhandler) {
    if (cr != nullptr)
        cr->register_exception_handler(excause, exhandler);
}

bool QtMipsMachine::addressapce_insert_range(MemoryAccess *mem_acces,
                        std::uint32_t start_addr, std::uint32_t last_addr,
                        bool move_ownership) {
    if (physaddrspace == nullptr)
        return false;
    return physaddrspace->insert_range(mem_acces, start_addr, last_addr,
                                       move_ownership);
}

void QtMipsMachine::insert_hwbreak(std::uint32_t address) {
    if (cr != nullptr)
        cr->insert_hwbreak(address);
}

void QtMipsMachine::remove_hwbreak(std::uint32_t address) {
    if (cr != nullptr)
        cr->remove_hwbreak(address);
}

bool QtMipsMachine::is_hwbreak(std::uint32_t address) {
    if (cr != nullptr)
        return cr->is_hwbreak(address);
    return false;
}

void QtMipsMachine::set_stop_on_exception(enum ExceptionCause excause, bool value) {
    if (cr != nullptr)
        cr->set_stop_on_exception(excause, value);
}

bool QtMipsMachine::get_stop_on_exception(enum ExceptionCause excause) const {
    if (cr != nullptr)
        return cr->get_stop_on_exception(excause);
    return false;
}

void QtMipsMachine::set_step_over_exception(enum ExceptionCause excause, bool value) {
    if (cr != nullptr)
        cr->set_step_over_exception(excause, value);
}

bool QtMipsMachine::get_step_over_exception(enum ExceptionCause excause) const {
    if (cr != nullptr)
        return cr->get_step_over_exception(excause);
    return false;
}

BranchPredictor *QtMipsMachine::bp() const {
    return cr->predictor();
}

enum ExceptionCause QtMipsMachine::get_exception_cause() const {
    std::uint32_t val;
    if (cop0st == nullptr)
        return EXCAUSE_NONE;
    val = (cop0st->read_cop0reg(Cop0State::Cause) >> 2) & 0x3f;
    if (val == 0)
        return EXCAUSE_INT;
    else
        return (ExceptionCause)val;
}
