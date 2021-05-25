#include "branchtargetbuffermodel.h"
#include "branchpredictor.h"

BranchTargetBufferModel::BranchTargetBufferModel(QObject *parent) : Super(parent), data_font("Monospace"), machine(nullptr) {
    data_font.setStyleHint(QFont::TypeWriter);
}

int BranchTargetBufferModel::rowCount(const QModelIndex & /*index*/) const {
    return pow(2, machine->config().bht_bits());
}

int BranchTargetBufferModel::columnCount(const QModelIndex & /* parent */) const {
    return 4;
}

QVariant BranchTargetBufferModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return tr("Index");
            case 1:
                return tr("Valid");
            case 2:
                return tr("Address");
            case 3:
                return tr("Tag");
            default:
                SANITY_ASSERT(0, "I got something wrong here");
            }
        }
    }
    return Super::headerData(section, orientation, role);
}

QVariant BranchTargetBufferModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return index.row();
        case 1:
        {
            QString valid = QString::number(machine->bp()->get_btb_entry_valid(index.row()), 16);
            return valid;
        }
        case 2:
        {
            QString address = QString::number(machine->bp()->get_btb_entry_address(index.row()), 16);
            QString zeroes;

            zeroes.fill('0', 8 - address.count());
            return "0x" + zeroes + address.toUpper();
        }
        case 3:
        {
            QString tag = QString::number(machine->bp()->get_btb_entry_tag(index.row()), 16);
            QString zeroes;

            zeroes.fill('0', 8 - tag.count());
            return "0x" + zeroes + tag.toUpper();
        }
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

const QFont &BranchTargetBufferModel::getFont() const {
    return data_font;
}

const machine::QtMipsMachine *BranchTargetBufferModel::getMachine() const {
    return machine;
}

void BranchTargetBufferModel::setup(machine::QtMipsMachine *machine) {
    this->machine = machine;
}
