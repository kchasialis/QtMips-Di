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
    MachineConfigCache(MemoryAccess::MemoryType ct = MemoryAccess::MemoryType::L1_CACHE);
    MachineConfigCache(const MachineConfigCache &cc) = default;
    MachineConfigCache(MemoryAccess::MemoryType ct, const QSettings*, const QString &prefix = "");

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
    void set_upper_mem_access_read(std::uint32_t ar);
    void set_upper_mem_access_write(std::uint32_t aw);
    void set_upper_mem_access_burst(std::uint32_t ab);
    void set_sets(std::uint32_t s); // Number of sets
    void set_blocks(std::uint32_t b); // Number of blocks
    void set_associativity(std::uint32_t a); // Degree of associativity
    void set_replacement_policy(ReplacementPolicy rp);
    void set_write_policy(WritePolicy wp);
    void set_type(MemoryAccess::MemoryType ct);

    bool enabled() const;
    std::uint32_t upper_mem_access_read() const;
    std::uint32_t upper_mem_access_write() const;
    std::uint32_t upper_mem_access_burst() const;
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
    std::uint32_t upper_mem_time_read, upper_mem_time_write, upper_mem_time_burst;
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

    enum HazardUnit {
        HU_NONE,
        HU_STALL,
        HU_STALL_FORWARD
    };

    enum BranchUnit {
        BU_NONE, // Neither delay slot nor branch predictor
        BU_DELAY_SLOT, // Delay slot approach
        BU_ONE_BIT_BP, // 1-bit branch predictor
        BU_TWO_BIT_BP // 2-bit branch predictor
    };

    // Configure if CPU is pipelined
    // In default disabled.
    void set_pipelined(bool);
    // Hazard unit
    void set_hazard_unit(HazardUnit);
    bool set_hazard_unit(QString);
    // Branch unit
    // When pipelined, delay slot or branch predictor are only possible options.
    // When not pipelined, none and delay slot are only possible options.
    void set_branch_unit(BranchUnit);
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
    // Configure cache
    void set_l1_data_cache(const MachineConfigCache&);
    void set_l1_program_cache(const MachineConfigCache&);
    void set_l2_unified_cache(const MachineConfigCache&);

    bool pipelined() const;
    bool predictor() const;
    enum BranchUnit branch_unit() const;
    std::int8_t bht_bits() const;
    bool branch_res_id() const;
    enum HazardUnit hazard_unit() const;
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
    BranchUnit bunit;
    std::uint8_t bp_bits;
    bool b_res_id;
    HazardUnit hunit;
    bool exec_protect, write_protect;
    bool osem_enable, osem_known_syscall_stop, osem_unknown_syscall_stop;
    bool osem_interrupt_stop, osem_exception_stop;
    bool res_at_compile;
    QString osem_fs_root;
    QString elf_path;
    // L1 cache is split to data/program cache.
    MachineConfigCache l1_data, l1_program;
    // L2 cache is unified.
    MachineConfigCache l2_unified;
};

}

Q_DECLARE_METATYPE(machine::MachineConfigCache)

#endif // MACHINECONFIG_H
