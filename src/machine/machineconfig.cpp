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

#include "machineconfig.h"
#include <QMap>

using namespace machine;

//////////////////////////////////////////////////////////////////////////////
/// Default config of MachineConfig
#define DF_PIPELINE true
#define DF_DHUNIT DHU_STALL_FORWARD
#define DF_CHUNIT CHU_DELAY_SLOT
#define DF_BP_BITS 0
#define DF_B_RES_ID true
#define DF_EXEC_PROTEC false
#define DF_WRITE_PROTEC false
#define DF_DRAM_ACC_READ 80
#define DF_DRAM_ACC_WRITE 80
#define DF_DRAM_ACC_BURST 0
#define DF_ELF QString("")
//////////////////////////////////////////////////////////////////////////////
/// Default config of MachineConfigCache
#define DFC_EN false
#define DFC_L1_PROG_ACC_READ 1
#define DFC_L1_PROG_ACC_WRITE 0
#define DFC_L1_PROG_ACC_BURST 0
#define DFC_L1_DATA_ACC_READ 1
#define DFC_L1_DATA_ACC_WRITE 1
#define DFC_L1_DATA_ACC_BURST 0
#define DFC_L2_UNIFIED_ACC_READ 5
#define DFC_L2_UNIFIED_ACC_WRITE 5
#define DFC_L2_UNIFIED_ACC_BURST 0
#define DFC_SETS 1
#define DFC_BLOCKS 1
#define DFC_ASSOC 1
#define DFC_REPLAC ReplacementPolicy::RP_RAND
#define DFC_WRITE WritePolicy::WP_THROUGH_NOALLOC
//////////////////////////////////////////////////////////////////////////////

MachineConfigCache::MachineConfigCache(const MemoryAccess::MemoryType &ct) :
                    en(DFC_EN), n_sets(DFC_SETS), n_blocks(DFC_BLOCKS), d_associativity(DFC_ASSOC),
                    replac_pol(DFC_REPLAC), write_pol(DFC_WRITE), cache_type(ct) {

    switch (ct) {
        case MemoryAccess::MemoryType::L1_PROGRAM_CACHE:
            m_time_read = DFC_L1_PROG_ACC_READ;
            m_time_write = DFC_L1_PROG_ACC_WRITE;
            m_time_burst = DFC_L1_PROG_ACC_BURST;
            break;
        case MemoryAccess::MemoryType::L1_DATA_CACHE:
            m_time_read = DFC_L1_DATA_ACC_READ;
            m_time_write = DFC_L1_DATA_ACC_WRITE;
            m_time_burst = DFC_L1_DATA_ACC_BURST;
            break;
        case MemoryAccess::MemoryType::L2_UNIFIED_CACHE:
            m_time_read = DFC_L2_UNIFIED_ACC_READ;
            m_time_write = DFC_L2_UNIFIED_ACC_WRITE;
            m_time_burst = DFC_L2_UNIFIED_ACC_BURST;
            break;
        default:
            SANITY_ASSERT(0, "Invalid type for cache memory.");
    }
}

MachineConfigCache::MachineConfigCache(const MachineConfigCache &cc) :
                                        en(cc.enabled()), m_time_read(cc.mem_access_read()), m_time_write(cc.mem_access_write()),
                                        m_time_burst(cc.mem_access_burst()), n_sets(cc.sets()), n_blocks(cc.blocks()),
                                        d_associativity(cc.associativity()), replac_pol(cc.replacement_policy()),
                                        write_pol(cc.write_policy()), cache_type(cc.type()) {}

#define N(STR) (prefix + QString(STR))

MachineConfigCache::MachineConfigCache(const MemoryAccess::MemoryType &ct, const QSettings *sts, const QString &prefix) :
                                       en(sts->value(N("Enabled"), DFC_EN).toUInt()),
                                       n_sets(sts->value(N("Sets"), DFC_SETS).toUInt()),
                                       n_blocks(sts->value(N("Blocks"), DFC_BLOCKS).toUInt()),
                                       d_associativity(sts->value(N("Associativity"), DFC_ASSOC).toUInt()),
                                       replac_pol((ReplacementPolicy)sts->value(N("Replacement"), (int32_t) DFC_REPLAC).toUInt()),
                                       write_pol((WritePolicy)sts->value(N("Write"), (int32_t) DFC_WRITE).toUInt()),
                                       cache_type(ct) {

    switch (cache_type) {
        case MemoryAccess::MemoryType::L1_PROGRAM_CACHE:
            m_time_read = sts->value(N("AccessTimeRead"), DFC_L1_PROG_ACC_READ).toUInt();
            m_time_write = sts->value(N("AccessTimeWrite"), DFC_L1_PROG_ACC_WRITE).toUInt();
            m_time_burst = sts->value(N("AccessTimeBurst"), DFC_L1_PROG_ACC_BURST).toUInt();
            break;
        case MemoryAccess::MemoryType::L1_DATA_CACHE:
            m_time_read = sts->value(N("AccessTimeRead"), DFC_L1_DATA_ACC_READ).toUInt();
            m_time_write = sts->value(N("AccessTimeWrite"), DFC_L1_DATA_ACC_WRITE).toUInt();
            m_time_burst = sts->value(N("AccessTimeBurst"), DFC_L1_DATA_ACC_BURST).toUInt();
            break;
        case MemoryAccess::MemoryType::L2_UNIFIED_CACHE:
            m_time_read = sts->value(N("AccessTimeRead"), DFC_L2_UNIFIED_ACC_READ).toUInt();
            m_time_write = sts->value(N("AccessTimeRead"), DFC_L2_UNIFIED_ACC_WRITE).toUInt();
            m_time_burst = sts->value(N("AccessTimeRead"), DFC_L2_UNIFIED_ACC_BURST).toUInt();
            break;
        default:
            SANITY_ASSERT(0, "Invalid type for cache memory.");
    }
}

void MachineConfigCache::store(QSettings *sts, const QString &prefix) {
    sts->setValue(N("Enabled"), enabled());
    sts->setValue(N("AccessTimeRead"), mem_access_read());
    sts->setValue(N("AccessTimeWrite"), mem_access_write());
    sts->setValue(N("AccessTimeBurst"), mem_access_burst());
    sts->setValue(N("Sets"), sets());
    sts->setValue(N("Blocks"), blocks());
    sts->setValue(N("Associativity"), associativity());
    sts->setValue(N("Replacement"), (std::uint32_t)replacement_policy());
    sts->setValue(N("Write"), (std::uint32_t)write_policy());
}

#undef N

void MachineConfigCache::preset(ConfigPresets p) {
    bool is_level1;

    switch (type()) {
        case MemoryAccess::MemoryType::L1_PROGRAM_CACHE:
            // Default settings for L1.
            is_level1 = true;
            set_mem_access_read(DFC_L1_PROG_ACC_READ);
            set_mem_access_write(DFC_L1_PROG_ACC_WRITE);
            set_mem_access_burst(DFC_L1_PROG_ACC_BURST);
            break;
        case MemoryAccess::MemoryType::L1_DATA_CACHE:
            is_level1 = true;
            set_mem_access_read(DFC_L1_DATA_ACC_READ);
            set_mem_access_write(DFC_L1_DATA_ACC_WRITE);
            set_mem_access_burst(DFC_L1_DATA_ACC_BURST);
            break;
        case MemoryAccess::MemoryType::L2_UNIFIED_CACHE:
            is_level1 = false;
            set_mem_access_read(DFC_L2_UNIFIED_ACC_READ);
            set_mem_access_write(DFC_L2_UNIFIED_ACC_WRITE);
            set_mem_access_burst(DFC_L2_UNIFIED_ACC_BURST);
            break;
        default:
            SANITY_ASSERT(0, "Invalid type for cache memory.");
    }

    switch (p) {
    case ConfigPresets::CP_PIPE:
    case ConfigPresets::CP_SINGLE_CACHE:
        if (is_level1) {
            // Default settings for L1.
            set_enabled(true);
            set_sets(4);
            set_blocks(2);
            set_associativity(2);
            set_replacement_policy(ReplacementPolicy::RP_RAND);
            set_write_policy(WritePolicy::WP_THROUGH_NOALLOC);
        } else {
            // By default L2 is not enabled.
            set_enabled(false);
        }
        break;
    case ConfigPresets::CP_SINGLE:
    case ConfigPresets::CP_PIPE_NO_HAZARD:
        set_enabled(false);
        break;
    default:
        SANITY_ASSERT(0, "Unhandled config preset.");
    }
}

void MachineConfigCache::set_enabled(bool e) {
    en = e;
}

void MachineConfigCache::set_mem_access_read(std::uint32_t ar) {
    m_time_read = ar;
}

void MachineConfigCache::set_mem_access_write(std::uint32_t aw) {
    m_time_write = aw;
}

void MachineConfigCache::set_mem_access_burst(std::uint32_t ab) {
    m_time_burst = ab;
}

void MachineConfigCache::set_sets(std::uint32_t s) {
    n_sets = s > 0 ? s : 1;
}

void MachineConfigCache::set_blocks(std::uint32_t b) {
    n_blocks = b > 0 ? b : 1;
}

void MachineConfigCache::set_associativity(std::uint32_t a) {
    d_associativity = a > 0 ? a : 1;
}

void MachineConfigCache::set_replacement_policy(ReplacementPolicy rp) {
    replac_pol = rp;
}

void MachineConfigCache::set_write_policy(WritePolicy wp) {
    write_pol = wp;
}

void MachineConfigCache::set_type(MemoryAccess::MemoryType ct) {
    cache_type = ct;
}

bool MachineConfigCache::enabled() const {
    return en;
}

std::uint32_t MachineConfigCache::mem_access_read() const {
    return m_time_read;
}

std::uint32_t MachineConfigCache::mem_access_write() const {
    return m_time_write;
}

std::uint32_t MachineConfigCache::mem_access_burst() const {
    return m_time_burst;
}

unsigned MachineConfigCache::sets() const {
    return n_sets;
}

unsigned MachineConfigCache::blocks() const {
    return n_blocks;
}

unsigned MachineConfigCache::associativity() const {
    return d_associativity;
}

MachineConfigCache::ReplacementPolicy MachineConfigCache::replacement_policy() const {
    return replac_pol;
}

MachineConfigCache::WritePolicy MachineConfigCache::write_policy() const {
    return write_pol;
}

MemoryAccess::MemoryType MachineConfigCache::type() const {
    return cache_type;
}

bool MachineConfigCache::operator==(const MachineConfigCache &c) const {
#define CMP(GETTER) (GETTER)() == (c.GETTER)()
    return CMP(enabled) && \
            CMP(mem_access_read) && \
            CMP(mem_access_write) && \
            CMP(mem_access_burst) && \
            CMP(sets) && \
            CMP(blocks) && \
            CMP(associativity) && \
            CMP(replacement_policy) && \
            CMP(write_policy);
#undef CMP
}

bool MachineConfigCache::operator!=(const MachineConfigCache &c) const {
    return !operator==(c);
}

MachineConfig::MachineConfig() : pipeline(DF_PIPELINE), dhunit(DF_DHUNIT), chunit(DF_CHUNIT), bp_bits(DF_BP_BITS),
                                 b_res_id(DF_B_RES_ID), exec_protect(DF_EXEC_PROTEC), write_protect(DF_WRITE_PROTEC),
                                 osem_enable(true), osem_known_syscall_stop(true), osem_unknown_syscall_stop(true),
                                 osem_interrupt_stop(true), osem_exception_stop(true), osem_fs_root(""),
                                 res_at_compile(true), elf_path(DF_ELF), dram_access_read(DF_DRAM_ACC_READ),
                                 dram_access_write(DF_DRAM_ACC_WRITE), dram_access_burst(DF_DRAM_ACC_BURST),
                                 l1_program(MemoryAccess::MemoryType::L1_PROGRAM_CACHE), l1_data(MemoryAccess::MemoryType::L1_DATA_CACHE),
                                 l2_unified(MemoryAccess::MemoryType::L2_UNIFIED_CACHE) {}

MachineConfig::MachineConfig(const MachineConfig& cc) noexcept :
                                            pipeline(cc.pipelined()), dhunit(cc.data_hazard_unit()), chunit(cc.control_hazard_unit()),
                                            bp_bits(cc.bht_bits()), b_res_id(cc.branch_res_id()), exec_protect(cc.memory_execute_protection()),
                                            write_protect(cc.memory_write_protection()), osem_enable(cc.osemu_enable()),
                                            osem_known_syscall_stop(cc.osemu_known_syscall_stop()), osem_unknown_syscall_stop(cc.osemu_unknown_syscall_stop()),
                                            osem_interrupt_stop(cc.osemu_interrupt_stop()), osem_exception_stop(cc.osemu_exception_stop()),
                                            osem_fs_root(cc.osemu_fs_root()), res_at_compile(cc.reset_at_compile()), elf_path(cc.elf()),
                                            dram_access_read(cc.ram_access_read()), dram_access_write(cc.ram_access_write()),
                                            dram_access_burst(cc.ram_access_burst()), l1_program(cc.l1_program_cache()),
                                            l1_data(cc.l1_data_cache()), l2_unified(cc.l2_unified_cache()) {}

#define N(STR) (prefix + QString(STR))

MachineConfig::MachineConfig(const QSettings *sts, const QString &prefix) :
                                l1_program(MemoryAccess::MemoryType::L1_PROGRAM_CACHE, sts, N("L1ProgramCache_")),
                                l1_data(MemoryAccess::MemoryType::L1_DATA_CACHE, sts, N("L1DataCache_")),
                                l2_unified(MemoryAccess::MemoryType::L2_UNIFIED_CACHE, sts, N("L2UnifiedCache_")) {
    pipeline = sts->value(N("Pipelined"), DF_PIPELINE).toBool();
    dhunit = (DataHazardUnit)sts->value(N("DataHazardUnit"), DF_DHUNIT).toUInt();
    chunit = (ControlHazardUnit)sts->value(N("ControlHazardUnit"), DF_CHUNIT).toUInt();
    bp_bits = sts->value(N("BPbits"), DF_BP_BITS).toInt();
    b_res_id = sts->value(N("BResId"), DF_B_RES_ID).toBool();
    exec_protect = sts->value(N("MemoryExecuteProtection"), DF_EXEC_PROTEC).toBool();
    write_protect = sts->value(N("MemoryWriteProtection"), DF_WRITE_PROTEC).toBool();
    osem_enable = sts->value(N("OsemuEnable"), true).toBool();
    osem_known_syscall_stop = sts->value(N("OsemuKnownSyscallStop"), true).toBool();
    osem_unknown_syscall_stop = sts->value(N("OsemuUnknownSyscallStop"), true).toBool();
    osem_interrupt_stop = sts->value(N("OsemuInterruptStop"), true).toBool();
    osem_exception_stop = sts->value(N("OsemuExceptionStop"), true).toBool();
    osem_fs_root = sts->value(N("OsemuFilesystemRoot"), "").toString();
    res_at_compile = sts->value(N("ResetAtCompile"), true).toBool();
    elf_path = sts->value(N("Elf"), DF_ELF).toString();
    dram_access_read = sts->value(N("DRAMAccessRead"), DF_DRAM_ACC_READ).toUInt();
    dram_access_write = sts->value(N("DRAMAccessWrite"), DF_DRAM_ACC_WRITE).toUInt();
    dram_access_burst = sts->value(N("DRAMAccessBurst"), DF_DRAM_ACC_BURST).toUInt();
}

void MachineConfig::store(QSettings *sts, const QString &prefix) {
    sts->setValue(N("Pipelined"), pipelined());
    sts->setValue(N("DataHazardUnit"), (unsigned)data_hazard_unit());
    sts->setValue(N("ControlHazardUnit"), (unsigned)control_hazard_unit());
    sts->setValue(N("BPbits"), bht_bits());
    sts->setValue(N("BResId"), branch_res_id());
    sts->setValue(N("OsemuEnable"), osemu_enable());
    sts->setValue(N("OsemuKnownSyscallStop"), osemu_known_syscall_stop());
    sts->setValue(N("OsemuUnknownSyscallStop"), osemu_unknown_syscall_stop());
    sts->setValue(N("OsemuInterruptStop"), osemu_interrupt_stop());
    sts->setValue(N("OsemuExceptionStop"), osemu_exception_stop());
    sts->setValue(N("OsemuFilesystemRoot"), osemu_fs_root());
    sts->setValue(N("ResetAtCompile"), reset_at_compile());
    sts->setValue(N("Elf"), elf());
    sts->setValue(N("DRAMAccessRead"), ram_access_read());
    sts->setValue(N("DRAMAccessWrite"), ram_access_write());
    sts->setValue(N("DRAMAccessBurst"), ram_access_burst());
    l1_data.store(sts, N("L1DataCache_"));
    l1_program.store(sts, N("L1ProgramCache_"));
    l2_unified.store(sts, N("L2UnifiedCache_"));
}

#undef N

void MachineConfig::preset(enum ConfigPresets p) {
    // Note: we set just a minimal subset to get preset (preserving as much of hidden configuration as possible)
    set_control_hazard_unit(ControlHazardUnit::CHU_DELAY_SLOT);
    set_bht_bits(-1);

    switch (p) {
    case ConfigPresets::CP_SINGLE:
    case ConfigPresets::CP_SINGLE_CACHE:
        set_pipelined(false);
        break;
    case ConfigPresets::CP_PIPE_NO_HAZARD:
        set_pipelined(true);
        set_data_hazard_unit(MachineConfig::DHU_NONE);
        break;
    case ConfigPresets::CP_PIPE:
        set_pipelined(true);
        set_data_hazard_unit(MachineConfig::DHU_STALL_FORWARD);
        break;
    }
    // Some common configurations
    set_memory_execute_protection(DF_EXEC_PROTEC);
    set_memory_write_protection(DF_WRITE_PROTEC);

    access_l1_data_cache()->preset(p);
    access_l1_program_cache()->preset(p);
    access_l2_unified_cache()->preset(p);
}

void MachineConfig::set_pipelined(bool v) {
    pipeline = v;
}

void MachineConfig::set_data_hazard_unit(enum MachineConfig::DataHazardUnit dhu)  {
    dhunit = dhu;
}

bool MachineConfig::set_data_hazard_unit(QString dhukind) {
    static QMap<QString, enum DataHazardUnit> dhukind_map =  {
        {"none",  DHU_NONE},
        {"stall", DHU_STALL},
        {"forward", DHU_STALL_FORWARD},
        {"stall-forward", DHU_STALL_FORWARD},
    };
    if (!dhukind_map.contains(dhukind))
        return false;

    set_data_hazard_unit(dhukind_map.value(dhukind));

    return true;
}

void MachineConfig::set_control_hazard_unit(MachineConfig::ControlHazardUnit chu) {
    chunit = chu;
}

void MachineConfig::set_bht_bits(int8_t b) {
    bp_bits = b;
}

void MachineConfig::set_branch_res_id(bool bri) {
    b_res_id = bri;
}

void MachineConfig::set_memory_execute_protection(bool ep) {
    exec_protect = ep;
}

void MachineConfig::set_memory_write_protection(bool wp) {
    write_protect = wp;
}

void MachineConfig::set_osemu_enable(bool e) {
    osem_enable = e;
}

void MachineConfig::set_osemu_known_syscall_stop(bool ks) {
    osem_known_syscall_stop = ks;
}

void MachineConfig::set_osemu_unknown_syscall_stop(bool us) {
    osem_unknown_syscall_stop = us;
}

void MachineConfig::set_osemu_interrupt_stop(bool is) {
    osem_interrupt_stop = is;
}

void MachineConfig::set_osemu_exception_stop(bool es) {
    osem_exception_stop = es;
}

void MachineConfig::set_osemu_fs_root(QString r) {
    osem_fs_root = r;
}

void MachineConfig::set_reset_at_compile(bool r) {
    res_at_compile = r;
}

void MachineConfig::set_elf(QString path) {
    elf_path = path;
}

void MachineConfig::set_ram_access_read(std::uint32_t dar) {
    dram_access_read = dar;
}

void MachineConfig::set_ram_access_write(std::uint32_t daw) {
    dram_access_write = daw;
}

void MachineConfig::set_ram_access_burst(std::uint32_t dab) {
    dram_access_burst = dab;
}

void MachineConfig::set_l1_data_cache(const MachineConfigCache& l1_d) {
    l1_data = l1_d;
}

void MachineConfig::set_l1_program_cache(const MachineConfigCache& l1_p) {
    l1_program = l1_p;
}

void MachineConfig::set_l2_unified_cache(const MachineConfigCache& l2) {
    l2_unified = l2;
}

bool MachineConfig::pipelined() const {
    return pipeline;
}

// Returns true if predictor is enabled.
bool MachineConfig::predictor() const {
    return chunit == CHU_ONE_BIT_BP || chunit == CHU_TWO_BIT_BP;
}

int8_t MachineConfig::bht_bits() const {
    return bp_bits;
}

bool MachineConfig::branch_res_id() const {
    return b_res_id;
}

enum MachineConfig::DataHazardUnit MachineConfig::data_hazard_unit() const {
    // Hazard unit is always off when there is no pipeline
    return pipeline ? dhunit : machine::MachineConfig::DHU_NONE;
}

// When pipelined, delay_slot or branch predictor are only possible options.
// When not pipelined, none and delay_slot are only possible options.
enum MachineConfig::ControlHazardUnit MachineConfig::control_hazard_unit() const {
    return chunit;
}

bool MachineConfig::memory_execute_protection() const {
    return exec_protect;
}

bool MachineConfig::memory_write_protection() const {
    return write_protect;
}

bool MachineConfig::osemu_enable() const {
    return osem_enable;
}
bool MachineConfig::osemu_known_syscall_stop() const {
    return osem_known_syscall_stop;
}
bool MachineConfig::osemu_unknown_syscall_stop() const {
    return osem_unknown_syscall_stop;
}
bool MachineConfig::osemu_interrupt_stop() const {
    return osem_interrupt_stop;
}
bool MachineConfig::osemu_exception_stop() const {
    return osem_exception_stop;
}

QString MachineConfig::osemu_fs_root() const {
    return osem_fs_root;
}

bool MachineConfig::reset_at_compile() const {
    return res_at_compile;
}

QString MachineConfig::elf() const {
    return elf_path;
}

std::uint32_t MachineConfig::ram_access_read() const {
    return dram_access_read;
}

std::uint32_t MachineConfig::ram_access_write() const {
    return dram_access_write;
}

std::uint32_t MachineConfig::ram_access_burst() const {
    return dram_access_burst;
}

const MachineConfigCache &MachineConfig::l1_data_cache() const {
    return l1_data;
}

const MachineConfigCache &MachineConfig::l1_program_cache() const {
    return l1_program;
}

const MachineConfigCache &MachineConfig::l2_unified_cache() const {
    return l2_unified;
}

MachineConfigCache *MachineConfig::access_l1_data_cache() {
    return &l1_data;
}

MachineConfigCache *MachineConfig::access_l1_program_cache() {
    return &l1_program;
}

MachineConfigCache *MachineConfig::access_l2_unified_cache() {
    return &l2_unified;
}

bool MachineConfig::operator==(const MachineConfig &c) const {
#define CMP(GETTER) (GETTER)() == (c.GETTER)()
    return CMP(pipelined) && \
            CMP(data_hazard_unit) && \
            CMP(control_hazard_unit) && \
            CMP(bht_bits) && \
            CMP(memory_execute_protection) && \
            CMP(memory_write_protection) && \
            CMP(elf) && \
            CMP(l1_data_cache) && \
            CMP(l1_program_cache) && \
            CMP(l2_unified_cache);
#undef CMP
}

bool MachineConfig::operator!=(const MachineConfig &c) const {
    return !operator==(c);
}
