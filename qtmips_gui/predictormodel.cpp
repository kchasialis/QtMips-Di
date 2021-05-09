#include <cmath>
#include "branchpredictor.h"
#include "predictormodel.h"

PredictorModel::PredictorModel(QObject *parent) : Super(parent), data_font("Monospace"), machine(nullptr) {
    data_font.setStyleHint(QFont::TypeWriter);
}

int PredictorModel::rowCount(const QModelIndex &index) const {
    return index.column() != 3 ? (machine == nullptr ? 750 : pow(2, machine->config().bht_bits())) : 1;
}

int PredictorModel::columnCount(const QModelIndex & /* parent */) const {
    return 4;
}

QVariant PredictorModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return tr("Index");
            case 1:
                return tr("History");
            case 2:
                return tr("Prediction");
            case 3:
                return tr("Precision");
            default:
                SANITY_ASSERT(0, "I got something wrong here");
            }
        }
    }
    return Super::headerData(section, orientation, role);
}

QVariant PredictorModel::data(const QModelIndex &index, int role) const {
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
            bool one_bit_pred;
            int8_t bht_entry = machine->bp()->get_bht_entry(index.row());
            switch (machine->config().branch_unit()) {
            case machine::MachineConfig::BU_ONE_BIT_BP:
                one_bit_pred = true;
                break;
            case machine::MachineConfig::BU_TWO_BIT_BP:
                one_bit_pred = false;
                break;
            default:
                SANITY_ASSERT(0, "Debug me :)");
                return QVariant();
            }

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
                    return case1 ? "STRONGLY_NT" : "NT";
                case 0x01:
                    return case1 ? "WEAKLY_NT" : "NT";
                case 0x10:
                    return case1 ? "WEAKLY_T" : "T";
                case 0x11:
                    return case1 ? "STRONGLY_T" : "T";
                default:
                    SANITY_ASSERT(0, "Debug me :)");
                    return QVariant();
                }
            }
        }
        case 3:
            return QString::number(machine->bp()->get_precision());
        default:
            SANITY_ASSERT(0, "Debug me :)");
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        // idk yet.
        return QVariant();
    } else if (role == Qt::FontRole) {
        return data_font;
    } else {
        return QVariant();
    }
}

Qt::ItemFlags PredictorModel::flags(const QModelIndex &index) const {
    if (index.column() == 1) {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    } else {
        return QAbstractTableModel::flags(index);
    }
}

bool PredictorModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        switch (index.column()) {
        case 1:
        {
            /* History column is editable. */
            machine->bp()->set_bht_entry(index.row(), value.toString());
            return true;
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

const QFont &PredictorModel::getFont() const {
    return data_font;
}

const machine::QtMipsMachine *PredictorModel::getMachine() const {
    return machine;
}

void PredictorModel::setup(machine::QtMipsMachine *machine) {
    this->machine = machine;
}
