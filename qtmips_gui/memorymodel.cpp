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

#include <QBrush>

#include "memorymodel.h"

MemoryModel::MemoryModel(QObject *parent)
    : Super(parent), data_font("Monospace") {
    cell_size = CELLSIZE_WORD;
    cells_per_row = 1;
    index0_offset = 0;
    data_font.setStyleHint(QFont::TypeWriter);
    machine = nullptr;
    memory_change_counter = 0;
    cache_data_change_counter = 0;
    access_through_cache = 0;
}

const machine::MemoryAccess *MemoryModel::mem_access() const {
    if (machine == nullptr)
        return nullptr;
    if (machine->physical_address_space() != nullptr)
        return machine->physical_address_space();
    return machine->memory();
}

machine::MemoryAccess *MemoryModel::mem_access_rw() const {
    if (machine == nullptr)
        return nullptr;
    if (machine->physical_address_space_rw() != nullptr)
        return machine->physical_address_space_rw();
    return machine->memory_rw();
}

int MemoryModel::rowCount(const QModelIndex & /*parent*/) const {
   // std::uint64_t rows = (0x2000 + cells_per_row - 1) / cells_per_row;
   return 750;
}

int MemoryModel::columnCount(const QModelIndex & /*parent*/) const {
    return cells_per_row + 1;
}

QVariant MemoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal) {
        if(role == Qt::DisplayRole) {
            if(section == 0) {
                return tr("Address");
            }
            else {
                std::uint32_t addr = (section - 1) * cellSizeBytes();
                QString ret = "+" + QString::number(addr, 10);
                return ret;
            }
        }
    }
    return Super::headerData(section, orientation, role);
}

QVariant MemoryModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        QString s, t;
        std::uint32_t address;
        std::uint32_t data;
        const machine::MemoryAccess *mem;
        if (!get_row_address(address, index.row()))
            return QString("");
        if (index.column() == 0) {
            t = QString::number(address, 16);
            s.fill('0', 8 - t.count());
            return "0x" + s + t.toUpper();
        }
        if (machine == nullptr)
            return QString("");
        mem = mem_access();
        if (mem == nullptr)
            return QString("");
        if ((access_through_cache > 0) && (machine->cache_data() != nullptr))
            mem = machine->cache_data();
        address += cellSizeBytes() * (index.column() - 1);
        if (address < index0_offset)
            return QString("");
        switch (cell_size) {
        case CELLSIZE_BYTE:
            data = mem->read_byte(address, true);
            break;
        case CELLSIZE_HWORD:
            data = mem->read_hword(address, true);
            break;
        default:
        case CELLSIZE_WORD:
            data = mem->read_word(address, true);
            break;
        }

        t = QString::number(data, 16);
        s.fill('0', cellSizeBytes() * 2 - t.count());
        t = s + t.toUpper();
#if 0
        machine::LocationStatus loc_stat = machine::LOCSTAT_NONE;
        if (machine->cache_data() != nullptr) {
            loc_stat = machine->cache_data()->location_status(address);
            if (loc_stat & machine::LOCSTAT_DIRTY)
                t += " D";
            else if (loc_stat & machine::LOCSTAT_CACHED)
                t += " C";
        }
#endif
        return t;
    }
    if (role == Qt::BackgroundRole) {
        std::uint32_t address;
        if (!get_row_address(address, index.row()) ||
            machine == nullptr || index.column() == 0)
            return QVariant();
        address += cellSizeBytes() * (index.column() - 1);
        if (machine->cache_data() != nullptr) {
            machine::LocationStatus loc_stat;
            loc_stat = machine->cache_data()->location_status(address);
            if (loc_stat & machine::LOCSTAT_DIRTY) {
                QBrush bgd(Qt::yellow);
                return bgd;
            } else if (loc_stat & machine::LOCSTAT_CACHED) {
                QBrush bgd(Qt::lightGray);
                return bgd;
            }
        }
        return QVariant();
    }
    if (role==Qt::FontRole)
            return data_font;
    return QVariant();
}

void MemoryModel::setup(machine::QtMipsMachine *machine) {
    this->machine = machine;
    if (machine != nullptr)
        connect(machine, SIGNAL(post_tick()), this, SLOT(check_for_updates()));
    if (mem_access() != nullptr)
        connect(mem_access(), SIGNAL(external_change_notify(const MemoryAccess*,std::uint32_t,std::uint32_t,bool)),
                this, SLOT(check_for_updates()));
    emit update_all();
    emit setup_done();
}

void MemoryModel::setCellsPerRow(unsigned int cells) {
    beginResetModel();
    cells_per_row = cells;
    endResetModel();
}

void MemoryModel::set_cell_size(int index) {
    beginResetModel();
    cell_size = (enum MemoryCellSize)index;
    index0_offset -= index0_offset % cellSizeBytes();
    endResetModel();
    emit cell_size_changed();
}

void MemoryModel::update_all() {
    const machine::MemoryAccess *mem;
    mem = mem_access();
    if (mem != nullptr) {
        memory_change_counter = mem->get_change_counter();
        if (machine->cache_data() != nullptr)
            cache_data_change_counter = machine->cache_data()->get_change_counter();
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void MemoryModel::check_for_updates() {
    bool need_update = false;
    const machine::MemoryAccess *mem;
    mem = mem_access();
    if (mem == nullptr)
        return;

    if (memory_change_counter != mem->get_change_counter())
        need_update = true;
    if (machine->cache_data() != nullptr) {
        if (cache_data_change_counter != machine->cache_data()->get_change_counter())
            need_update = true;
    }
    if (!need_update)
        return;
    update_all();
}

bool MemoryModel::adjustRowAndOffset(int &row, std::uint32_t address) {
    row = rowCount() / 2;
    address -= address % cellSizeBytes();
    std::uint32_t row_bytes = cells_per_row * cellSizeBytes();
    std::uint32_t diff = row * row_bytes;
    if (diff > address) {
        row = address / row_bytes;
        if (row == 0) {
            index0_offset = 0;
        } else {
           index0_offset = address - row * row_bytes;
        }
    } else {
        index0_offset = address - diff;
    }
    return get_row_for_address(row, address);
}

void MemoryModel::cached_access(int cached) {
    access_through_cache = cached;
    update_all();
}

Qt::ItemFlags MemoryModel::flags(const QModelIndex &index) const {
    if (index.column() == 0)
        return QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool MemoryModel::setData(const QModelIndex & index, const QVariant & value, int role) {
    if (role == Qt::EditRole)
    {
        bool ok;
        std::uint32_t address;
        machine::MemoryAccess *mem;
        std::uint32_t data = value.toString().toULong(&ok, 16);
        if (!ok)
            return false;
        if (!get_row_address(address, index.row()))
            return false;
        if (index.column() == 0 || machine == nullptr)
            return false;
        mem = mem_access_rw();
        if (mem == nullptr)
            return false;
        if ((access_through_cache > 0) && (machine->cache_data_rw() != nullptr))
            mem = machine->cache_data_rw();
        address += cellSizeBytes() * (index.column() - 1);
        switch (cell_size) {
        case CELLSIZE_BYTE:
            mem->write_byte(address, data);
            break;
        case CELLSIZE_HWORD:
            mem->write_hword(address, data);
            break;
        default:
        case CELLSIZE_WORD:
            mem->write_word(address, data);
            break;
        }
    }
    return true;
}
