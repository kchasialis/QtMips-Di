#ifndef PREDICTORDOCK_H
#define PREDICTORDOCK_H

#include <QDockWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTableView>
#include "qtmipsmachine.h"
#include "predictormodel.h"

class PredictorDock : public QDockWidget  {
    Q_OBJECT

    using Super = QDockWidget;

public:
    PredictorDock(QWidget *parent);

    void setup(machine::QtMipsMachine *machine);

public slots:
    void update_pc_val(std::uint32_t inst_addr);
    void update_instr_val(const machine::Instruction &instr);
    void update_bht_index_val(std::uint32_t inst_addr);
private:
    QTableView *predictor_content;
    QVBoxLayout *vlayout;
    QLineEdit *bht_entries_val;
    QLineEdit *history_bits_val;
    QLineEdit *pc_val;
    QLineEdit *instr_val;
    QLineEdit *bht_index_val;
    machine::QtMipsMachine *machine;
};


#endif // PREDICTORDOCK_H
