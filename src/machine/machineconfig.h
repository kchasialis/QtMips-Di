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

#ifndef MACHINECONFIG_H
#define MACHINECONFIG_H

#include <QString>
#include <QSettings>
#include "memory.h"

namespace machine {

// TODO: add more options regarding L2.
enum class ConfigPresets {
    CP_SINGLE, // No pipeline cpu without cache.
    CP_SINGLE_CACHE, // No pipeline cpu with a L1+L2 cache.
    CP_PIPE_NO_HAZARD, // Pipelined cpu without hazard unit and without cache.
    CP_PIPE // Full pipelined cpu with a L1+L2 cache.
};

class MachineConfigCache {
public:
    explicit MachineConfigCache(const MemoryAccess::MemoryType& = MemoryAccess::MemoryType::L1_PROGRAM_CACHE);
    MachineConfigCache(const MachineConfigCache &cc);
    MachineConfigCache(const MemoryAccess::MemoryType &ct, const QSettings*, const QString &prefix = "");

    void store(QSettings*, const QString &prefix = "");

    void preset(enum ConfigPresets);

    enum class ReplacementPolicy {
        RP_RAND, // Random
        RP_LRU, // Least recently used
        RP_LFU // Least frequently used
    };

    enum class WritePolicy {
        WP_THROUGH_NOALLOC, // Write through
        WP_THROUGH_ALLOC, // Write through
        WP_BACK // Write back
    };

    // If cache should be used or not
    void set_enabled(bool e);
    void set_mem_access_read(std::uint32_t ar);
    void set_mem_access_write(std::uint32_t aw);
    void set_mem_access_burst(std::uint32_t ab);
    void set_sets(std::uint32_t s); // Number of sets
    void set_blocks(std::uint32_t b); // Number of blocks
    void set_associativity(std::uint32_t a); // Degree of associativity
    void set_replacement_policy(ReplacementPolicy rp);
    void set_write_policy(WritePolicy wp);
    void set_type(MemoryAccess::MemoryType ct);

    bool enabled() const;
    std::uint32_t mem_access_read() const;
    std::uint32_t mem_access_write() const;
    std::uint32_t mem_access_burst() const;
    std::uint32_t sets() const;
    std::uint32_t blocks() const;
    std::uint32_t associativity() const;
    ReplacementPolicy replacement_policy() const;
    WritePolicy write_policy() const;
    MemoryAccess::MemoryType type() const;

    bool operator ==(const MachineConfigCache &c) const;
    bool operator !=(const MachineConfigCache &c) const;

private:
    bool en;
    std::uint32_t m_time_read, m_time_write, m_time_burst;
    std::uint32_t n_sets, n_blocks, d_associativity;
    ReplacementPolicy replac_pol;
    WritePolicy write_pol;
    MemoryAccess::MemoryType cache_type;
};

class MachineConfig {
public:
    MachineConfig();
    MachineConfig(const MachineConfig& cc) noexcept;
    MachineConfig(const QSettings*, const QString &prefix = "");

    void store(QSettings*, const QString &prefix = "");

    void preset(enum ConfigPresets);

    enum DataHazardUnit {
        DHU_NONE,
        DHU_STALL,
        DHU_STALL_FORWARD
    };

    enum ControlHazardUnit {
        CHU_NONE,
        CHU_STALL,
        CHU_DELAY_SLOT,
        CHU_ONE_BIT_BP,
        CHU_TWO_BIT_BP
    };

    // Configure if CPU is pipelined
    // In default disabled.
    void set_pipelined(bool);
    // Hazard unit
    void set_data_hazard_unit(DataHazardUnit);
    bool set_data_hazard_unit(QString);
    // Control Hazard Unit unit
    void set_control_hazard_unit(ControlHazardUnit);
    // Branch history table lookup bits
    void set_bht_bits(std::int8_t);
    // Wether or not branch resolution is done on ID.
    void set_branch_res_id(bool);
    // Protect data memory from execution. Only program sections can be executed.
    void set_memory_execute_protection(bool);
    // Protect program memory from accidental writes.
    void set_memory_write_protection(bool);
    // Operating system and exceptions setup
    void set_osemu_enable(bool);
    void set_osemu_known_syscall_stop(bool);
    void set_osemu_unknown_syscall_stop(bool);
    void set_osemu_interrupt_stop(bool);
    void set_osemu_exception_stop(bool);
    void set_osemu_fs_root(QString);
    // reset machine befor internal compile/reload after external make
    void set_reset_at_compile(bool);
    // Set path to source elf file. This has to be set before core is initialized.
    void set_elf(QString);
    // Configure DRAM access times.
    void set_ram_access_read(std::uint32_t);
    void set_ram_access_write(std::uint32_t);
    void set_ram_access_burst(std::uint32_t);
    // Configure cache
    void set_l1_data_cache(const MachineConfigCache&);
    void set_l1_program_cache(const MachineConfigCache&);
    void set_l2_unified_cache(const MachineConfigCache&);

    bool pipelined() const;
    bool predictor() const;
    enum DataHazardUnit data_hazard_unit() const;
    enum ControlHazardUnit control_hazard_unit() const;
    std::int8_t bht_bits() const;
    bool branch_res_id() const;
    bool memory_execute_protection() const;
    bool memory_write_protection() const;
    bool osemu_enable() const;
    bool osemu_known_syscall_stop() const;
    bool osemu_unknown_syscall_stop() const;
    bool osemu_interrupt_stop() const;
    bool osemu_exception_stop() const;
    QString osemu_fs_root() const;
    bool reset_at_compile() const;
    QString elf() const;
    std::uint32_t ram_access_read() const;
    std::uint32_t ram_access_write() const;
    std::uint32_t ram_access_burst() const;

    const MachineConfigCache &l1_data_cache() const;
    const MachineConfigCache &l1_program_cache() const;
    const MachineConfigCache &l2_unified_cache() const;
    MachineConfigCache *access_l1_data_cache();
    MachineConfigCache *access_l1_program_cache();
    MachineConfigCache *access_l2_unified_cache();

    bool operator ==(const MachineConfig&) const;
    bool operator !=(const MachineConfig&) const;

private:
    bool pipeline;
    DataHazardUnit dhunit;
    ControlHazardUnit chunit;
    std::uint8_t bp_bits;
    bool b_res_id;
    bool exec_protect, write_protect;
    bool osem_enable, osem_known_syscall_stop, osem_unknown_syscall_stop;
    bool osem_interrupt_stop, osem_exception_stop;
    QString osem_fs_root;
    bool res_at_compile;
    QString elf_path;
    std::uint32_t dram_access_read, dram_access_write, dram_access_burst;
    // L1 cache is split to data/program cache.
    MachineConfigCache l1_program, l1_data;
    // L2 cache is unified.
    MachineConfigCache l2_unified;
};

}

Q_DECLARE_METATYPE(machine::MachineConfigCache)

#endif // MACHINECONFIG_H
