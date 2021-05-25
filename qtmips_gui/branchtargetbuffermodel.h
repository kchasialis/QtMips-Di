#ifndef BRANCHTARGETBUFFERMODEL_H
#define BRANCHTARGETBUFFERMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include "qtmipsmachine.h"

class BranchTargetBufferModel : public QAbstractTableModel {
    Q_OBJECT

    using Super = QAbstractTableModel;
public:
    BranchTargetBufferModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool adjustRowAndOffset(int &row, std::uint32_t address);
    const QFont &getFont() const;
    const machine::QtMipsMachine *getMachine() const;

public slots:
    void setup(machine::QtMipsMachine *machine);

private:
    QFont data_font;
    machine::QtMipsMachine *machine;
};

#endif // BRANCHTARGETBUFFERMODEL_H
