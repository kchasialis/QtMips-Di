#ifndef BRANCH_HISTORY_TABLE_DOCK_H
#define BRANCH_HISTORY_TABLE_DOCK_H

#include <QDockWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTableView>
#include "qtmipsmachine.h"
#include "branchhistorytablemodel.h"

class BranchHistoryTableDock : public QDockWidget  {
    Q_OBJECT

    using Super = QDockWidget;

public:
    explicit BranchHistoryTableDock(QWidget *parent);

    void setup(machine::QtMipsMachine *machine);

public slots:
    void update_pc_val(std::uint32_t inst_addr);
    void update_instr_val(const machine::Instruction &instr);
    void update_bht_index_val(std::uint32_t inst_addr);
    void update_accuracy_val(std::int32_t);
    void reset();

private:
    QTableView *predictor_content;
    QVBoxLayout *vlayout;
    QLineEdit *bht_entries_val;
    QLineEdit *history_bits_val;
    QLineEdit *pc_val;
    QLineEdit *instr_val;
    QLineEdit *bht_index_val;
    QLineEdit *accuracy_val;
    machine::QtMipsMachine *machine;
};


#endif // BRANCH_HISTORY_TABLE_DOCK_H
