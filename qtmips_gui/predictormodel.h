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

public slots:
    void setup(machine::QtMipsMachine *machine);

private:
    QFont data_font;
    machine::QtMipsMachine *machine;
};

#endif // PREDICTORMODEL_H
