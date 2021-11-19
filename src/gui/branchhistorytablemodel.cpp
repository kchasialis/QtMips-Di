#include "branchpredictor.h"
#include "branchhistorytablemodel.h"
#include <cmath>
#include <QBrush>

BranchHistoryTableModel::BranchHistoryTableModel(QObject *parent) : Super(parent), data_font("Monospace"), machine(nullptr), pos_bht_access(-1), pos_bht_update(-1) {
    data_font.setStyleHint(QFont::TypeWriter);
}

#include <QDebug>
int BranchHistoryTableModel::rowCount(const QModelIndex &index) const {
    return index.column() != 3 ? (machine == nullptr ? 750 : (int) pow(2, machine->config().bht_bits())) : 1;
}

int BranchHistoryTableModel::columnCount(const QModelIndex & /* parent */) const {
    return 3;
}

QVariant BranchHistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return tr("Index");
            case 1:
                return tr("State");
            case 2:
                return tr("Prediction");
            default:
                SANITY_ASSERT(0, "I got something wrong here");
            }
        }
    }
    return Super::headerData(section, orientation, role);
}

#include <QDebug>

QVariant BranchHistoryTableModel::data(const QModelIndex &index, int role) const {
    // This function works only if we already have a machine.
    // The code for case 1 and 2 is nearly identical, except in the 2-bit branch predictor.
    bool case1 = false;

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return index.row();
        case 1:
            case1 = true;
        case 2:
        {
            if (machine) {
                int one_bit_pred = machine->config().control_hazard_unit() == machine::MachineConfig::CHU_ONE_BIT_BP;
                int8_t bht_entry = machine->bp()->bht_entry(index.row());

                if (one_bit_pred) {
                    switch (bht_entry) {
                        case 0x0:
                            return "NT";
                        case 0x1:
                            return "T";
                        default:
                            SANITY_ASSERT(0, "Debug me :)");
                            return QVariant();
                    }
                } else {
                    switch (bht_entry) {
                        case 0x00:
                            return case1 ? "STRONG-NT" : "NT";
                        case 0x01:
                            return case1 ? "WEAK-NT" : "NT";
                        case 0x10:
                            return case1 ? "WEAK-T" : "T";
                        case 0x11:
                            return case1 ? "STRONG-T" : "T";
                        default:
                            SANITY_ASSERT(0, "Debug me :)");
                            return QVariant();
                    }
                }
            } else {
                return "Not Set";
            }
        }
        default:
            SANITY_ASSERT(0, "Debug me :)");
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        if (index.row() == pos_bht_access) {
            if (pos_bht_access == pos_bht_update) {
                // Pink - we updated the same entry we are accessing.
                QBrush bgd(QColor(255, 173, 230));
                return bgd;
            } else {
                // Green - we just accessed a new entry.
                QBrush bgd(QColor(193, 255, 173));
                return bgd;
            }
        } else if (index.row() == pos_bht_update) {
            // Orange - we updated an entry in the table.
            QBrush bgd(QColor(255, 173, 120));
            return bgd;
        }
        return QVariant();
    } else if (role == Qt::FontRole) {
        return data_font;
    } else {
        return QVariant();
    }
}

Qt::ItemFlags BranchHistoryTableModel::flags(const QModelIndex &index) const {
    return index.column() == 1 ? (QAbstractTableModel::flags(index) | Qt::ItemIsEditable) :
                                 QAbstractTableModel::flags(index);   
}

bool BranchHistoryTableModel::setData(const QModelIndex &idx, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        switch (idx.column()) {
        case 1:
        {
            if (machine) {
                /* History column is editable. */
                machine->bp()->set_bht_entry(idx.row(), value.toString());
                emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
                return true;
            } else {
                return false;
            }
        }
        case 0:
            /* Index column is not editable.  */
        case 2:
            /* Prediction column is not editable.  */
        case 3:
            /* Precision column is not editable.  */
            return false;
        default:
            SANITY_ASSERT(0, "I got something wrong here");
            return false;
        }
    }
    else {
        return true;
    }
}

const QFont &BranchHistoryTableModel::getFont() const {
    return data_font;
}

const machine::QtMipsMachine *BranchHistoryTableModel::getMachine() const {
    return machine;
}

void BranchHistoryTableModel::setup(machine::QtMipsMachine *machine) {
    this->machine = machine;
}

void BranchHistoryTableModel::update_pos_bht_update(std::int32_t pbu) {
    this->pos_bht_update = pbu;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void BranchHistoryTableModel::update_pos_bht_access(std::int32_t pba) {
    this->pos_bht_access = pba;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}
