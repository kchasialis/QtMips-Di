#ifndef PREDICTORMODEL_H
#define PREDICTORMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include "qtmipsmachine.h"

class PredictorModel : public QAbstractTableModel {
    Q_OBJECT

    using Super = QAbstractTableModel;
public:
    PredictorModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;
    bool adjustRowAndOffset(int &row, std::uint32_t address);
    const QFont &getFont() const;
    const machine::QtMipsMachine *getMachine() const;

//    inline std::uint32_t getIndex0Offset() const {
//        return index0_offset;
//    }

//    inline unsigned int cellSizeBytes() const {
//        return 4;
//    }
//    inline bool get_row_address(std::uint32_t &address, int row) const {
//        address = index0_offset + row * cellSizeBytes();
//        return address >= index0_offset;
//    }
//    inline bool get_row_for_address(int &row, std::uint32_t address) const {
//        if (address < index0_offset) {
//            row = -1;
//            return false;
//        }
//        row = (address - index0_offset) / cellSizeBytes();
//        if ((address - index0_offset > 0x80000000) || row > rowCount()) {
//            row = rowCount();
//            return false;
//        }
//        return true;
//    }

//    enum StageAddress {
//        STAGEADDR_FETCH,
//        STAGEADDR_DECODE,
//        STAGEADDR_EXECUTE,
//        STAGEADDR_MEMORY,
//        STAGEADDR_WRITEBACK,
//        STAGEADDR_COUNT,
//    };

//signals:
//    void report_error(QString error);
public slots:
    void setup(machine::QtMipsMachine *machine);
//    void check_for_updates();
//    void toggle_hw_break(const QModelIndex & index);
//    void update_stage_addr(uint stage, std::uint32_t addr);
//    void update_all();

//private:
//    const machine::MemoryAccess *mem_access() const;
//    machine::MemoryAccess *mem_access_rw() const;
//    std::uint32_t index0_offset;
//    QFont data_font;
//    machine::QtMipsMachine *machine;
//    std::uint32_t memory_change_counter;
//    std::uint32_t cache_program_change_counter;
//    std::uint32_t stage_addr[STAGEADDR_COUNT];
//    bool stages_need_update;

private:
    QFont data_font;
    machine::QtMipsMachine *machine;
};

#endif // PREDICTORMODEL_H
