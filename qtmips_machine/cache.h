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

#ifndef CACHE_H
#define CACHE_H

#include <memory.h>
#include <machineconfig.h>
#include <stdint.h>
#include <time.h>

namespace machine {

class Cache : public MemoryAccess {
    Q_OBJECT
public:
    Cache(MemoryAccess *m, const MachineConfigCache &cc, std::uint32_t lower_access_penalty_r,
          std::uint32_t lower_access_penalty_w, std::uint32_t lower_access_penalty_b);
    ~Cache();

    bool wword(std::uint32_t address, std::uint32_t value) override;
    std::uint32_t rword(std::uint32_t address, bool debug_access = false) const override;
    virtual std::uint32_t get_change_counter() const override;
    virtual MemoryType type() const override;

    void flush(); // flush cache.
    void sync() override; // Same as flush.

    std::uint32_t hit() const; // Number of recorded hits.
    std::uint32_t miss() const; // Number of recorded misses.
    std::uint32_t mu_reads() const; // Number of reads on the lower level (L* or memory).
    std::uint32_t mu_writes() const; // Number of writes on the lower level (L* or memory).
    std::uint32_t stalled_cycles() const; // Number of wasted cycles in lower level.
    double speed_improvement() const; // Speed improvement in percents in comare with no used cache.
    double hit_rate() const; // Usage efficiency in percents.

    void reset(); // Reset whole state of cache.

    const MachineConfigCache &config() const;
    enum LocationStatus location_status(std::uint32_t address) const override;

signals:
    void hit_update(std::uint32_t) const;
    void miss_update(std::uint32_t) const;
    void statistics_update(std::uint32_t stalled_cycles, double speed_improv, double hit_rate) const;
    void cache_update(std::uint32_t associat, std::uint32_t set, std::uint32_t col, bool valid, bool dirty,
                      std::uint32_t tag, const std::uint32_t *data, bool write) const;
    void level2_cache_reads_update(std::uint32_t) const;
    void level2_cache_writes_update(std::uint32_t) const;
    void memory_writes_update(std::uint32_t) const;
    void memory_reads_update(std::uint32_t) const;

private:
    MachineConfigCache cnf;
    MemoryAccess *mem_lower;
    std::uint32_t access_pen_r, access_pen_w, access_pen_b;
    std::uint32_t uncached_start;
    std::uint32_t uncached_last;
    MemoryType cache_type;
    mutable std::uint32_t read_hits, read_misses, write_hits, write_misses;
    mutable std::uint32_t mem_lower_reads, mem_lower_writes;
    mutable std::uint32_t burst_reads, burst_writes;
    mutable std::uint32_t change_counter;

    struct cache_data {
        bool valid, dirty;
        std::uint32_t tag;
        std::uint32_t *data;
    };
    mutable cache_data **dt;

    union {
        std::uint32_t **lru; // Access time
        std::uint32_t **lfu; // Access count
    } replc; // Data used for replacement policy

    void emit_mem_lower_signal(bool read) const;
    std::uint32_t debug_rword(std::uint32_t address) const;
    bool access(std::uint32_t address, std::uint32_t *data, bool write, std::uint32_t value = 0) const;
    void kick(std::uint32_t associat_indx, std::uint32_t row) const;
    std::uint32_t base_address(std::uint32_t tag, std::uint32_t row) const;
    void update_statistics() const;
    inline void compute_row_col_tag(std::uint32_t &row, std::uint32_t &col,
                               std::uint32_t &tag, std::uint32_t address) const {
        std::uint32_t ssize, index;

        address = address >> 2;
        ssize = cnf.blocks() * cnf.sets();
        tag = address / ssize;
        index = address % ssize;
        row = index / cnf.blocks();
        col = index % cnf.blocks();
    }
};

}

#endif // CACHE_H
