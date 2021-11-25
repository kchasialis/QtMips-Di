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

#include <QDebug>
#include "cache.h"

using namespace machine;

extern CycleStatistics cycle_stats;

Cache::Cache(const MachineConfigCache &cc, MemoryAccess *m, uint32_t acc_read, uint32_t acc_write,
             uint32_t acc_burst, uint32_t lower_acc_pen_r, uint32_t lower_acc_pen_w, uint32_t lower_acc_pen_b) :
                MemoryAccess(acc_read, acc_write, acc_burst), cnf(cc), mem_lower(m),
                access_pen_read(lower_acc_pen_r), access_pen_write(lower_acc_pen_w),
                access_pen_burst(lower_acc_pen_b), uncached_start(0xf0000000), uncached_last(0xffffffff),
                cache_type(cc.type()), read_hits(0), read_misses(0), write_hits(0), write_misses(0),
                mem_lower_reads(0), mem_lower_writes(0), burst_reads(0), burst_writes(0),
                change_counter(0), dt(nullptr), replc() {

    replc.lfu = nullptr;
    replc.lru = nullptr;

    // Skip any other initialization if cache is disabled
    if (!cc.enabled())
        return;

    // Allocate cache data structure
    dt = new cache_data*[cc.associativity()];
    for (size_t i = 0; i < cc.associativity(); i++) {
        dt[i] = new cache_data[cc.sets()];
        for (size_t y = 0; y < cc.sets(); y++) {
            dt[i][y].valid = false;
            dt[i][y].dirty = false;
            dt[i][y].data = new std::uint32_t[cc.blocks()];
        }
    }
    // Allocate replacement policy data
    switch (cnf.replacement_policy()) {
    case MachineConfigCache::ReplacementPolicy::RP_LFU:
        replc.lfu = new std::uint32_t *[cnf.sets()];
        for (size_t row = 0; row < cnf.sets(); row++) {
            replc.lfu[row] = new std::uint32_t[cnf.associativity()];
            for (size_t  i = 0; i < cnf.associativity(); i++)
                 replc.lfu[row][i] = 0;
        }
        break;
    case MachineConfigCache::ReplacementPolicy::RP_LRU:
        replc.lru = new std::uint32_t*[cnf.sets()];
        for (size_t row = 0; row < cnf.sets(); row++) {
            replc.lru[row] = new std::uint32_t[cnf.associativity()];
            for (size_t i = 0; i < cnf.associativity(); i++)
                replc.lru[row][i] = i;
        }
        break;
    case MachineConfigCache::ReplacementPolicy::RP_RAND:
    default:
        break;
    }
}

Cache::~Cache(){
    if (dt != nullptr) {
        for (size_t i = 0; i < cnf.associativity(); i++) {
            if (dt[i]) {
                    for (size_t y = 0; y < cnf.sets(); y++) {
                        delete[] dt[i][y].data;
                    }
                    delete[] dt[i];
                }
        }
        delete[] dt;
    }

    switch (cnf.replacement_policy()) {
    case MachineConfigCache::ReplacementPolicy::RP_LFU:
        if (replc.lfu == nullptr)
            break;
        for (std::uint32_t row = 0; row < cnf.sets(); row++)
            delete[] replc.lfu[row];
        delete [] replc.lfu;
        break;
    case MachineConfigCache::ReplacementPolicy::RP_LRU:
        if (replc.lru == nullptr)
            break;
        for (std::uint32_t row = 0; row < cnf.sets(); row++)
            delete[] replc.lru[row];
        delete[] replc.lru;
    default:
        break;
    }
}

bool Cache::wword(std::uint32_t address, std::uint32_t value) {
    std::uint32_t data;
    bool changed;
    bool out_of_bounds = address >= uncached_start && address <= uncached_last;

    if (!cnf.enabled() || out_of_bounds) {
        SANITY_ASSERT(0, "I shouldn't get called");
//        mem_lower_writes++;
//        emit_mem_lower_signal(false);
//        update_statistics();
//        return mem_lower->write_word(address, value);
    }

    writes++;
    changed = access(address, &data, true, value);

    if (cnf.write_alloc() == MachineConfigCache::WritePolicy::WP_THROUGH) {
        mem_lower_writes++;
        emit_mem_lower_signal(false);
        update_statistics();
        mem_lower->set_update_stats(true);
        return mem_lower->write_word(address, value);
    }

    return changed;
}

std::uint32_t Cache::rword(std::uint32_t address, bool debug_access) const {
    std::uint32_t data;
    bool out_of_bounds = address >= uncached_start && address <= uncached_last;

    if (!cnf.enabled() || out_of_bounds) {
        SANITY_ASSERT(0, "I shouldn't get called.");
        mem_lower_reads++;
        emit_mem_lower_signal(true);
        update_statistics();
        return mem_lower->read_word(address, debug_access);
    }

    if (debug_access) {
        SANITY_ASSERT(0, "I dont want this to be executed now.");
        if (!(location_status(address) & LOCSTAT_CACHED))
            return mem_lower->read_word(address, debug_access);
        return debug_rword(address);
    }

    reads++;
    access(address, &data, false);

    return data;
}

std::uint32_t Cache::get_change_counter() const {
    return change_counter;
}

MemoryAccess::MemoryType Cache::type() const {
    return cache_type;
}

void Cache::flush() {
    if (!cnf.enabled())
        return;

    for (size_t as = cnf.associativity(); as-- > 0 ; ) {
        for (size_t st = 0; st < cnf.sets(); st++) {
            if (dt[as][st].valid) {
                kick(as, st);
                emit cache_update(as, st, 0, false, false, 0, nullptr, false);
            }
        }
    }

    change_counter++;
    update_statistics();
}

void Cache::sync() {
    flush();
}

std::uint32_t Cache::hit() const {
    return read_hits + write_hits;
}

std::uint32_t Cache::miss() const {
    return read_misses + write_misses;
}

//std::uint32_t Cache::ml_reads() const {
//    return mem_lower_reads;
//}
//
//std::uint32_t Cache::ml_writes() const {
//    return mem_lower_writes;
//}

std::uint32_t Cache::stalled_cycles() const {
    uint32_t st_cycles = mem_lower_reads * access_pen_read + mem_lower_writes * access_pen_write;

    if (access_pen_burst != 0)
        st_cycles -= burst_reads * (access_pen_read - access_pen_burst) +
                     burst_writes * (access_pen_write - access_pen_burst);

    return st_cycles;
}

double Cache::speed_improvement() const {
    std::uint32_t lookup_time;
    std::uint32_t lower_access_time;
    std::uint32_t comp = read_hits + write_hits + read_misses + write_misses;

    if (comp == 0)
        return 100.0;

    lookup_time = read_hits + read_misses;
    if (cnf.write_alloc() == MachineConfigCache::WritePolicy::WP_BACK)
        lookup_time += write_hits + write_misses;

    lower_access_time = mem_lower_reads * access_pen_read +
                      mem_lower_writes * access_pen_write;

    if (access_pen_burst != 0)
        lower_access_time -= burst_reads * (access_pen_read - access_pen_burst) +
                           burst_writes * (access_pen_write - access_pen_burst);

    return (double)((read_hits + read_misses) * access_pen_read + (write_hits + write_misses) * access_pen_write) \
            / (double)(lookup_time + lower_access_time) \
            * 100;
}

double Cache::hit_rate() const {
    std::uint32_t comp = read_hits + write_hits + read_misses + write_misses;

    return comp == 0 ? 0.0 :
                       (double)(read_hits + write_hits) / (double)comp * 100.0;
}

void Cache::reset() {
    // Set all cells to invalid
    if (cnf.enabled()) {
        for (size_t as = 0; as < cnf.associativity(); as++)
            for (size_t st = 0; st < cnf.sets(); st++)
                dt[as][st].valid = false;
    }

    // Note: we don't have to zero replacement policy data as those are zeroed when first used on invalid cell
    // Zero hit and miss rate
    read_hits = 0;
    read_misses = 0;
    write_hits = 0;
    write_misses = 0;
    mem_lower_reads = 0;
    mem_lower_writes = 0;
    burst_reads = 0;
    burst_writes = 0;

    // Trigger signals
    emit hit_update(hit());
    emit miss_update(miss());
    emit_mem_lower_signal(true);
    emit_mem_lower_signal(false);
    update_statistics();
    if (cnf.enabled()) {
        for (size_t as = 0; as < cnf.associativity(); as++)
            for (size_t st = 0; st < cnf.sets(); st++)
                emit cache_update(as, st, 0, false, false, 0, 0, false);
    }
}

const MachineConfigCache &Cache::config() const {
    return cnf;
}

enum LocationStatus Cache::location_status(std::uint32_t address) const {
    std::uint32_t row, col, tag;
    compute_row_col_tag(row, col, tag, address);

    if (cnf.enabled()) {
        for (unsigned indx = 0; indx < cnf.associativity(); indx++) {
            if (dt[indx][row].valid && dt[indx][row].tag == tag) {
                if (dt[indx][row].dirty &&
                    cnf.write_policy() == MachineConfigCache::WP_BACK)
                    return (enum LocationStatus)(LOCSTAT_CACHED | LOCSTAT_DIRTY);
                else
                    return (enum LocationStatus)LOCSTAT_CACHED;
            }
        }
    }

    return mem_lower->location_status(address);
}

void Cache::emit_mem_lower_signal(bool read) const {
    switch (mem_lower->type()) {
        case MemoryType::L1_PROGRAM_CACHE:
        case MemoryType::L1_DATA_CACHE:
            SANITY_ASSERT(0, "Lower level cannot be an L1 cache.");
            break;
        case MemoryType::L2_UNIFIED_CACHE:
            if (read)
                emit level2_cache_reads_update(mem_lower_reads);
            else
                emit level2_cache_writes_update(mem_lower_writes);
            break;
        case MemoryType::DRAM:
            if (read)
                emit memory_reads_update(mem_lower_reads);
            else
                emit memory_writes_update(mem_lower_writes);
            break;
        default:
            SANITY_ASSERT(0, "Unknown memory type.");
    }
}

std::uint32_t Cache::debug_rword(std::uint32_t address) const {
    std::uint32_t row, col, tag;

    compute_row_col_tag(row, col, tag, address);

    for (size_t indx = 0; indx < cnf.associativity(); indx++) {
        if (dt[indx][row].valid && dt[indx][row].tag == tag)
            return dt[indx][row].data[col];
    }

    return 0;
}

bool Cache::access(std::uint32_t address, std::uint32_t *data, bool write, std::uint32_t value) const {
    bool changed = false;
    uint32_t row, col, tag, indx;

    compute_row_col_tag(row, col, tag, address);

    indx = 0;
    // Try to locate exact block
    while (indx < cnf.associativity() && (!dt[indx][row].valid || dt[indx][row].tag != tag))
        indx++;
    // Need to find new block
    if (indx >= cnf.associativity()) {
        // return early if we do not need to allocate a block on write miss.
        if (write && !cnf.write_alloc()) {
            update_misses(false);
            emit miss_update(miss());
            update_statistics();
            return false;
        }
        // We have to kick something
        switch (cnf.replacement_policy()) {
        case MachineConfigCache::ReplacementPolicy::RP_RAND:
            indx = rand() % cnf.associativity();
            break;
        case MachineConfigCache::ReplacementPolicy::RP_LRU:
            indx = replc.lru[row][0];
            break;
        case MachineConfigCache::ReplacementPolicy::RP_LFU:
            {
                uint32_t lowest = replc.lfu[row][0];
                indx = 0;
                for (size_t i = 1; i < cnf.associativity(); i++) {
                    if (!dt[i][row].valid) {
                        indx = i;
                        break;
                    }
                    if (lowest > replc.lfu[row][i]) {
                        lowest = replc.lfu[row][i];
                        indx = i;
                    }
                }
                break;
            }
        }
    }
    SANITY_ASSERT(indx < cnf.associativity(), "Probably unimplemented replacement policy");

    cache_data &cd = dt[indx][row];

    // Verify if we are not replacing
    if (cd.valid && cd.tag != tag) {
        kick(indx, row);
        change_counter++;
    }

    // Update statistics and otherwise read from memory
    if (cd.valid) {
        if (write)
            update_hits(false);
        else
            update_hits(true);

        emit hit_update(hit());
        update_statistics();
    } else {
        if (write)
            update_misses(false);
        else
            update_misses(true);
        emit miss_update(miss());

        // We allocate a block in cache if its a read miss or a write miss with write-allocate.
        if (!write || cnf.write_alloc()) {
            for (size_t i = 0; i < cnf.blocks(); i++) {
                mem_lower->set_update_stats(i == 0);
                cd.data[i] = mem_lower->read_word(base_address(tag, row) + (4 * i));
                change_counter++;
            }

            ++mem_lower_reads;
            burst_reads += cnf.blocks() - 1;
            emit_mem_lower_signal(true);
            update_statistics();
        }
    }

    // Update replacement data
    switch (cnf.replacement_policy()) {
        case MachineConfigCache::ReplacementPolicy::RP_LRU:
        {
            uint32_t next_asi = indx;
            int i = cnf.associativity() - 1;
            uint32_t tmp_asi = replc.lru[row][i];
            while (tmp_asi != indx) {
                SANITY_ASSERT(i >= 0, "LRU lost the way from priority queue - access");
                tmp_asi = replc.lru[row][i];
                replc.lru[row][i] = next_asi;
                next_asi = tmp_asi;
                i--;
            }
            break;
        }
        case MachineConfigCache::ReplacementPolicy::RP_LFU:
            if (cd.valid)
                replc.lfu[row][indx]++;
            else
                replc.lfu[row][indx] = 0;
            break;
        default:
            break;
    }

    cd.valid = true; // We either write to it or we read from memory. Either way it's valid when we leave Cache class
    cd.dirty = cd.dirty || write;
    cd.tag = tag;
    *data = cd.data[col];

    if (write) {
        changed = cd.data[col] != value;
        cd.data[col] = value;
    }

    emit cache_update(indx, row, col, cd.valid, cd.dirty, cd.tag, cd.data, write);
    if (changed)
        change_counter++;
    return changed;
}

void Cache::kick(std::uint32_t associat_indx, std::uint32_t row) const {
    cache_data &cd = dt[associat_indx][row];

    if (cd.dirty) {
        if (cnf.write_alloc() == MachineConfigCache::WritePolicy::WP_BACK) {
            for (size_t i = 0; i < cnf.blocks(); i++) {
                mem_lower->set_update_stats(i == 0);
                mem_lower->write_word(base_address(cd.tag, row) + (4*i), cd.data[i]);
            }

            ++mem_lower_writes;
            burst_writes += cnf.blocks() - 1;
            emit_mem_lower_signal(false);
        }
    }

    cd.valid = false;
    cd.dirty = false;

    switch (cnf.replacement_policy()) {
        case MachineConfigCache::ReplacementPolicy::RP_LRU:
        {
            std::uint32_t next_asi = associat_indx;
            std::uint32_t tmp_asi = replc.lru[row][0];
            int i = 1;
            while (tmp_asi != associat_indx) {
                SANITY_ASSERT(i < (int)cnf.associativity(), "LRU lost the way from priority queue - kick");
                tmp_asi = replc.lru[row][i];
                replc.lru[row][i] = next_asi;
                next_asi = tmp_asi;
                i++;
            }
            break;
        }
        case MachineConfigCache::ReplacementPolicy::RP_LFU:
            replc.lfu[row][associat_indx] = 0;
            break;
        default:
            break;
    }
}

std::uint32_t Cache::base_address(std::uint32_t tag, std::uint32_t row) const {
    return ((tag * cnf.blocks() * cnf.sets()) + (row * cnf.blocks())) << 2;
}

void Cache::update_statistics() const {
    emit statistics_update(stalled_cycles(), speed_improvement(), hit_rate());
}

/* TODO: check if these work with various configurations for write policy!  */
void Cache::update_misses(bool read) const {
    if (update_stats) {
        read_misses += read ? 1 : 0;
        write_misses += !read ? 1 : 0;
        switch (cnf.type()) {
            case MemoryType::L1_PROGRAM_CACHE:
                cycle_stats.l1_program_stall_cycles = read ? access_pen_read : access_pen_write;
                break;
            case MemoryType::L1_DATA_CACHE:
                cycle_stats.l1_data_stall_cycles = read ? access_pen_read : access_pen_write;
                break;
            case MemoryType::L2_UNIFIED_CACHE:
                cycle_stats.l2_unified_stall_cycles = read ? access_pen_read : access_pen_write;
                break;
            default:
                SANITY_ASSERT(0, "Wrong type for cache.");
        }
        cycle_stats.memory_cycles += read ? access_pen_read : access_pen_write;
    }
}

void Cache::update_hits(bool read) const {
    if (update_stats) {
        read_hits += read ? 1 : 0;
        write_hits += !read ? 1 : 0;
    }
}