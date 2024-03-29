#ifndef BRANCH_HISTORY_TABLE_MODEL_H
#define BRANCH_HISTORY_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include "qtmipsmachine.h"

class BranchHistoryTableModel : public QAbstractTableModel {
    Q_OBJECT

    using Super = QAbstractTableModel;
public:
    BranchHistoryTableModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &idx, const QVariant & value, int role) override;
    bool adjustRowAndOffset(int &row, std::uint32_t address);
    const QFont &getFont() const;
    const machine::QtMipsMachine *getMachine() const;

public slots:
    void setup(machine::QtMipsMachine *machine);
    void update_pos_bht_update(int32_t pbu);
    void update_pos_bht_access(int32_t pba);

private:
    QFont data_font;
    machine::QtMipsMachine *machine;
    std::int32_t pos_bht_access, pos_bht_update;
};

#endif // BRANCH_HISTORY_TABLE_MODEL_H
