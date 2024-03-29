#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <cmath>
#include "branchpredictor.h"
#include "branchhistorytabledock.h"
#include "branchhistorytabletableview.h"

BranchHistoryTableDock::BranchHistoryTableDock(QWidget *parent) : Super(parent) {
    setObjectName("Branch History Table");
    setWindowTitle("Branch History Table");

    QWidget *content = new QWidget();

    QHBoxLayout *hlayout_top = new QHBoxLayout();
    predictor_content = new BranchPredictorTableView(this);
    QVBoxLayout *vlayout_mid = new QVBoxLayout();
    vlayout = new QVBoxLayout();

    QLabel *bht_entries = new QLabel("BHT Entries");
    bht_entries_val = new QLineEdit("Not Set");
    QLabel *history_bits = new QLabel("History Bits");
    history_bits_val = new QLineEdit("Not Set");

    QLabel *instr = new QLabel("Instruction");
    instr_val = new QLineEdit();
    QLabel *addr = new QLabel("Address");
    addr_val = new QLineEdit();
    QLabel *bht_index = new QLabel("BHT Index");
    bht_index_val = new QLineEdit();
    QLabel *accuracy = new QLabel("Accuracy");
    accuracy_val = new QLineEdit("0.0%");

    bht_entries_val->setReadOnly(true);
    history_bits_val->setReadOnly(true);
    bht_entries_val->setReadOnly(true);
    history_bits_val->setReadOnly(true);
    instr_val->setReadOnly(true);
    addr_val->setReadOnly(true);
    bht_index_val->setReadOnly(true);
    accuracy_val->setReadOnly(true);

    hlayout_top->addWidget(bht_entries);
    hlayout_top->addWidget(bht_entries_val);
    hlayout_top->addWidget(history_bits);
    hlayout_top->addWidget(history_bits_val);

    vlayout_mid->addWidget(instr);
    vlayout_mid->addWidget(instr_val);
    vlayout_mid->addWidget(addr);
    vlayout_mid->addWidget(addr_val);
    vlayout_mid->addWidget(bht_index);
    vlayout_mid->addWidget(bht_index_val);
    vlayout_mid->addWidget(accuracy);
    vlayout_mid->addWidget(accuracy_val);

    vlayout->addLayout(hlayout_top);
    vlayout->addLayout(vlayout_mid);
    vlayout->addWidget(predictor_content);

    content->setLayout(vlayout);

    setWidget(content);
}

static void set_qline_val(QLineEdit *qline_obj, const QString &text) {
    qline_obj->setReadOnly(false);
    qline_obj->setText(text);
    qline_obj->setReadOnly(true);
}

void BranchHistoryTableDock::setup(machine::QtMipsMachine *machine) {
    auto *pmodel = new BranchHistoryTableModel(this);
    QString text;
    bool check;

    this->machine = machine;
    pmodel->setup(machine);
    predictor_content->setModel(pmodel);
    vlayout->update();

    if (machine) {
        check = connect(machine->bp(), SIGNAL(pred_inst_addr_value(uint32_t)), this, SLOT(update_pc_val(uint32_t)));
        Q_ASSERT(check);
        check = connect(machine->bp(), SIGNAL(pred_inst_addr_value(uint32_t)), this, SLOT(update_bht_index_val(uint32_t)));
        Q_ASSERT(check);
        check = connect(machine->bp(), SIGNAL(pred_updated_accuracy(double)), this, SLOT(update_accuracy_val(double)));
        Q_ASSERT(check);
        check = connect(machine->bp(), SIGNAL(pred_instr_value(const machine::Instruction&)), this, SLOT(update_instr_val(const machine::Instruction&)));
        Q_ASSERT(check);
        check = connect(machine->bp(), SIGNAL(pred_updated_bht(int32_t)), pmodel, SLOT(update_pos_bht_update(int32_t)));
        Q_ASSERT(check);
        check = connect(machine->bp(), SIGNAL(pred_accessed_bht(int32_t)), pmodel, SLOT(update_pos_bht_access(int32_t)));
        Q_ASSERT(check);
        check = connect(machine->bp(), SIGNAL(pred_accessed_bht(int32_t)), predictor_content, SLOT(focus_row(int32_t)));
        Q_ASSERT(check);
        check = connect(machine->bp(), SIGNAL(pred_updated_bht(int32_t)), predictor_content, SLOT(focus_row(int32_t)));
        Q_ASSERT(check);

        switch (this->machine->config().control_hazard_unit()) {
            case machine::MachineConfig::CHU_ONE_BIT_BP:
                text = "One Bit";
                break;
            case machine::MachineConfig::CHU_TWO_BIT_BP:
                text = "Two Bit";
                break;
            default:
                SANITY_ASSERT(0, "Debug me.");
        }

        set_qline_val(history_bits_val, text);
        set_qline_val(bht_entries_val, QString::number(pow(2, this->machine->config().bht_bits())));
    } else {
        set_qline_val(bht_entries_val, "Not Set");
        set_qline_val(history_bits_val, "Not Set");
    }
}

void BranchHistoryTableDock::update_pc_val(std::uint32_t inst_addr) {
    QString s,t, text;

    t = QString::number(inst_addr, 16);
    s.fill('0', 8 - t.count());
    text = "0x" + s + t.toUpper();

    set_qline_val(addr_val, text);
}

void BranchHistoryTableDock::update_instr_val(const machine::Instruction &instr) {
    QString s,t, text;

//    t = QString::number(instr.data(), 16);
//    s.fill('0', 8 - t.count());
//    text = "0x" + s + t.toUpper();

    set_qline_val(instr_val, instr.to_str());
}

void BranchHistoryTableDock::update_bht_index_val(std::uint32_t inst_addr) {
    if (machine)
        set_qline_val(bht_index_val, QString::number(machine->bp()->bht_idx(inst_addr, true)));
    else
        set_qline_val(bht_index_val, "Not Set");
}

void BranchHistoryTableDock::update_accuracy_val(double acc) {
    if (machine)
        set_qline_val(accuracy_val, QString::number(acc) + "%");
    else
        set_qline_val(accuracy_val, "Not Set");
}
