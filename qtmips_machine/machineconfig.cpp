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
#define DF_PIPELINE false
#define DF_BUNIT BU_DELAY_SLOT
#define DF_BP_BITS -1
#define DF_HUNIT HU_STALL_FORWARD
#define DF_EXEC_PROTEC false
#define DF_WRITE_PROTEC false
#define DF_ELF QString("")
//////////////////////////////////////////////////////////////////////////////
/// Default config of MachineConfigCache
#define DFC_EN false
#define DFC_MEM_ACC_READ 80
#define DFC_MEM_ACC_WRITE 80
#define DFC_MEM_ACC_BURST 0
#define DFC_L2_ACC_READ 5
#define DFC_L2_ACC_WRITE 5
#define DFC_L2_ACC_BURST 0
#define DFC_SETS 1
#define DFC_BLOCKS 1
#define DFC_ASSOC 1
#define DFC_REPLAC ReplacementPolicy::RP_RAND
#define DFC_WRITE WritePolicy::WP_THROUGH_NOALLOC
//////////////////////////////////////////////////////////////////////////////

MachineConfigCache::MachineConfigCache(MemoryAccess::MemoryType ct) {
    switch (ct) {
    case MemoryAccess::MemoryType::L1_CACHE:
        upper_mem_time_read = DFC_L2_ACC_READ;
        upper_mem_time_write = DFC_L2_ACC_WRITE;
        upper_mem_time_burst = DFC_L2_ACC_BURST;
        break;
    case MemoryAccess::MemoryType::L2_CACHE:
        upper_mem_time_read = DFC_MEM_ACC_READ;
        upper_mem_time_write = DFC_MEM_ACC_WRITE;
        upper_mem_time_burst = DFC_MEM_ACC_BURST;
        break;
    case MemoryAccess::MemoryType::DRAM:
    default:
        SANITY_ASSERT(0, "This needs debugging.");
    }

    en = DFC_EN;
    n_sets = DFC_SETS;
    n_blocks = DFC_BLOCKS;
    d_associativity = DFC_ASSOC;
    replac_pol = DFC_REPLAC;
    write_pol = DFC_WRITE;
    cache_type = ct;
}

MachineConfigCache::MachineConfigCache(const MachineConfigCache& cc) noexcept {
    en = cc.enabled();
    upper_mem_time_read = cc.upper_mem_access_read();
    upper_mem_time_write = cc.upper_mem_access_write();
    upper_mem_time_burst = cc.upper_mem_access_burst();
    n_sets = cc.sets();
    n_blocks = cc.blocks();
    d_associativity = cc.associativity();
    replac_pol = cc.replacement_policy();
    write_pol = cc.write_policy();
    cache_type = cc.type();
}

#define N(STR) (prefix + QString(STR))

MachineConfigCache::MachineConfigCache(MemoryAccess::MemoryType ct, const QSettings *sts, const QString &prefix) {
    en = sts->value(N("Enabled"), DFC_EN).toUInt();

    cache_type = (MemoryAccess::MemoryType)sts->value(N("CacheType"), (int32_t) ct).toUInt();
    switch (cache_type) {
    case MemoryAccess::MemoryType::L1_CACHE:
        upper_mem_time_read = sts->value(N("UpperAccessTimeRead"), DFC_L2_ACC_READ).toUInt();
        upper_mem_time_write = sts->value(N("UpperAccessTimeWrite"), DFC_L2_ACC_WRITE).toUInt();
        upper_mem_time_burst = sts->value(N("UpperAccessTimeBurst"), DFC_L2_ACC_BURST).toUInt();
        break;
    case MemoryAccess::MemoryType::L2_CACHE:
        upper_mem_time_read = sts->value(N("UpperAccessTimeRead"), DFC_MEM_ACC_READ).toUInt();
        upper_mem_time_write = sts->value(N("UpperAccessTimeWrite"), DFC_MEM_ACC_WRITE).toUInt();
        upper_mem_time_burst = sts->value(N("UpperAccessTimeBurst"), DFC_MEM_ACC_BURST).toUInt();
        break;
    case MemoryAccess::MemoryType::DRAM:
    default:
        SANITY_ASSERT(0, "This needs debugging.");
    }

    n_sets = sts->value(N("Sets"), DFC_SETS).toUInt();
    n_blocks = sts->value(N("Blocks"), DFC_BLOCKS).toUInt();
    d_associativity = sts->value(N("Associativity"), DFC_ASSOC).toUInt();
    replac_pol = (ReplacementPolicy)sts->value(N("Replacement"), (int32_t) DFC_REPLAC).toUInt();
    write_pol = (WritePolicy)sts->value(N("Write"), (int32_t) DFC_WRITE).toUInt();
}

void MachineConfigCache::store(QSettings *sts, const QString &prefix) {
    sts->setValue(N("Enabled"), enabled());
    sts->setValue(N("UpperAccessTimeRead"), upper_mem_access_read());
    sts->setValue(N("UpperAccessTimeWrite"), upper_mem_access_write());
    sts->setValue(N("UpperAccessTimeBurst"), upper_mem_access_burst());
    sts->setValue(N("Sets"), sets());
    sts->setValue(N("Blocks"), blocks());
    sts->setValue(N("Associativity"), associativity());
    sts->setValue(N("Replacement"), (std::uint32_t)replacement_policy());
    sts->setValue(N("Write"), (std::uint32_t)write_policy());
    sts->setValue(N("CacheType"), (std::uint32_t)type());
}

#undef N

void MachineConfigCache::preset(ConfigPresets p) {
    bool is_level1;

    switch (type()) {
    case MemoryAccess::MemoryType::L1_CACHE:
        // Default settings for L1.
        is_level1 = true;
        set_upper_mem_access_read(DFC_L2_ACC_READ);
        set_upper_mem_access_write(DFC_L2_ACC_WRITE);
        set_upper_mem_access_burst(DFC_L2_ACC_BURST);
        break;
    case MemoryAccess::MemoryType::L2_CACHE:
        is_level1 = false;
        set_upper_mem_access_read(DFC_MEM_ACC_READ);
        set_upper_mem_access_write(DFC_MEM_ACC_WRITE);
        set_upper_mem_access_burst(DFC_MEM_ACC_BURST);
        break;
    case MemoryAccess::MemoryType::DRAM:
    default:
        SANITY_ASSERT(0, "This needs debugging.");
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

void MachineConfigCache::set_upper_mem_access_read(std::uint32_t ar) {
    upper_mem_time_read = ar;
}

void MachineConfigCache::set_upper_mem_access_write(std::uint32_t aw) {
    upper_mem_time_write = aw;
}

void MachineConfigCache::set_upper_mem_access_burst(std::uint32_t ab) {
    upper_mem_time_burst = ab;
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

std::uint32_t MachineConfigCache::upper_mem_access_read() const {
    return upper_mem_time_read;
}

std::uint32_t MachineConfigCache::upper_mem_access_write() const {
    return upper_mem_time_write;
}

std::uint32_t MachineConfigCache::upper_mem_access_burst() const {
    return upper_mem_time_burst;
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
            CMP(upper_mem_access_read) && \
            CMP(upper_mem_access_write) && \
            CMP(upper_mem_access_burst) && \
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

MachineConfig::MachineConfig() {
    pipeline = DF_PIPELINE;
    bunit = DF_BUNIT;
    bp_bits = DF_BP_BITS;
    hunit = DF_HUNIT;
    exec_protect = DF_EXEC_PROTEC;
    write_protect = DF_WRITE_PROTEC;
    osem_enable = true;
    osem_known_syscall_stop = true;
    osem_unknown_syscall_stop = true;
    osem_interrupt_stop = true;
    osem_exception_stop = true;
    osem_fs_root = "";
    res_at_compile = true;
    elf_path = DF_ELF;
    l1_data = MachineConfigCache(MemoryAccess::MemoryType::L1_CACHE);
    l1_program = MachineConfigCache(MemoryAccess::MemoryType::L1_CACHE);
    l2_unified = MachineConfigCache(MemoryAccess::MemoryType::L2_CACHE);
}

MachineConfig::MachineConfig(const MachineConfig& cc) noexcept {
    this->pipeline = cc.pipelined();
    this->bunit = cc.branch_unit();
    this->bp_bits = cc.bht_bits();
    this->hunit = cc.hazard_unit();
    this->exec_protect = cc.memory_execute_protection();
    this->write_protect = cc.memory_write_protection();
    this->osem_enable = cc.osemu_enable();
    this->osem_known_syscall_stop = cc.osemu_known_syscall_stop();
    this->osem_unknown_syscall_stop = cc.osemu_unknown_syscall_stop();
    this->osem_interrupt_stop = cc.osemu_interrupt_stop();
    this->osem_exception_stop = cc.osemu_exception_stop();
    this->osem_fs_root = cc.osemu_fs_root();
    this->res_at_compile = cc.reset_at_compile();
    this->elf_path = cc.elf();
    this->l1_data = cc.l1_data_cache();
    this->l1_program = cc.l1_program_cache();
    this->l2_unified = cc.l2_unified_cache();
}

#define N(STR) (prefix + QString(STR))

MachineConfig::MachineConfig(const QSettings *sts, const QString &prefix) {
    pipeline = sts->value(N("Pipelined"), DF_PIPELINE).toBool();
    bunit = (BranchUnit)sts->value(N("BranchUnit"), DF_BUNIT).toUInt();
    bp_bits = sts->value(N("BPbits"), DF_BP_BITS).toInt();
    hunit = (HazardUnit)sts->value(N("HazardUnit"), DF_HUNIT).toUInt();
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
    l1_data = MachineConfigCache(MemoryAccess::MemoryType::L1_CACHE, sts, N("L1DataCache_"));
    l1_program = MachineConfigCache(MemoryAccess::MemoryType::L1_CACHE, sts, N("L1ProgramCache_"));
    l2_unified = MachineConfigCache(MemoryAccess::MemoryType::L2_CACHE, sts, N("L2UnifiedCache_"));
}

void MachineConfig::store(QSettings *sts, const QString &prefix) {
    sts->setValue(N("Pipelined"), pipelined());
    sts->setValue(N("BranchUnit"), (unsigned)branch_unit());
    sts->setValue(N("BPbits"), bht_bits());
    sts->setValue(N("HazardUnit"), (unsigned)hazard_unit());
    sts->setValue(N("OsemuEnable"), osemu_enable());
    sts->setValue(N("OsemuKnownSyscallStop"), osemu_known_syscall_stop());
    sts->setValue(N("OsemuUnknownSyscallStop"), osemu_unknown_syscall_stop());
    sts->setValue(N("OsemuInterruptStop"), osemu_interrupt_stop());
    sts->setValue(N("OsemuExceptionStop"), osemu_exception_stop());
    sts->setValue(N("OsemuFilesystemRoot"), osemu_fs_root());
    sts->setValue(N("ResetAtCompile"), reset_at_compile());
    sts->setValue(N("Elf"), elf());
    l1_data.store(sts, N("L1DataCache_"));
    l1_program.store(sts, N("L1ProgramCache_"));
    l2_unified.store(sts, N("L2UnifiedCache_"));
}

#undef N

void MachineConfig::preset(enum ConfigPresets p) {
    // Note: we set just a minimal subset to get preset (preserving as much of hidden configuration as possible)
    set_branch_unit(BranchUnit::BU_DELAY_SLOT);
    set_bht_bits(-1);

    switch (p) {
    case ConfigPresets::CP_SINGLE:
    case ConfigPresets::CP_SINGLE_CACHE:
        set_pipelined(false);
        break;
    case ConfigPresets::CP_PIPE_NO_HAZARD:
        set_pipelined(true);
        set_hazard_unit(MachineConfig::HU_NONE);
        break;
    case ConfigPresets::CP_PIPE:
        set_pipelined(true);
        set_hazard_unit(MachineConfig::HU_STALL_FORWARD);
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

void MachineConfig::set_hazard_unit(enum MachineConfig::HazardUnit hu)  {
    hunit = hu;
}

bool MachineConfig::set_hazard_unit(QString hukind) {
    static QMap<QString, enum HazardUnit> hukind_map =  {
        {"none",  HU_NONE},
        {"stall", HU_STALL},
        {"forward", HU_STALL_FORWARD},
        {"stall-forward", HU_STALL_FORWARD},
    };
    if (!hukind_map.contains(hukind))
        return false;

    set_hazard_unit(hukind_map.value(hukind));

    return true;
}

void MachineConfig::set_branch_unit(MachineConfig::BranchUnit bu) {
    bunit = bu;
}

void MachineConfig::set_bht_bits(int8_t b) {
    bp_bits = b;
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

// When pipelined, delay_slot or branch predictor are only possible options.
// When not pipelined, none and delay_slot are only possible options.
enum MachineConfig::BranchUnit MachineConfig::branch_unit() const {
    return bunit;
}

int8_t MachineConfig::bht_bits() const {
    return bp_bits;
}

enum MachineConfig::HazardUnit MachineConfig::hazard_unit() const {
    // Hazard unit is always off when there is no pipeline
    return pipeline ? hunit : machine::MachineConfig::HU_NONE;
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
            CMP(branch_unit) && \
            CMP(bht_bits) && \
            CMP(hazard_unit) && \
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
