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
    QLabel *pc = new QLabel("PC");
    pc_val = new QLineEdit();
    QLabel *bht_index = new QLabel("BHT Index");
    bht_index_val = new QLineEdit();
    QLabel *accuracy = new QLabel("Accuracy");
    accuracy_val = new QLineEdit();

    bht_entries_val->setReadOnly(true);
    history_bits_val->setReadOnly(true);
    bht_entries_val->setReadOnly(true);
    history_bits_val->setReadOnly(true);
    instr_val->setReadOnly(true);
    pc_val->setReadOnly(true);
    bht_index_val->setReadOnly(true);
    accuracy_val->setReadOnly(true);

    hlayout_top->addWidget(bht_entries);
    hlayout_top->addWidget(bht_entries_val);
    hlayout_top->addWidget(history_bits);
    hlayout_top->addWidget(history_bits_val);

    vlayout_mid->addWidget(instr);
    vlayout_mid->addWidget(instr_val);
    vlayout_mid->addWidget(pc);
    vlayout_mid->addWidget(pc_val);
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

#include <QDebug>

void BranchHistoryTableDock::setup(machine::QtMipsMachine *machine) {
    auto *pmodel = new BranchPredictorModel(this);
    QString text;

    this->machine = machine;
    pmodel->setup(machine);
    predictor_content->setModel(pmodel);
    vlayout->update();

    qDebug() << machine->bp();

    connect(machine->bp(), SIGNAL(pred_inst_addr_value(std::uint32_t)), this, SLOT(update_pc_val(std::uint32_t)));
    connect(machine->bp(), SIGNAL(pred_inst_addr_value(std::uint32_t)), this, SLOT(update_bht_index_val(std::uint32_t)));
    connect(machine->bp(), SIGNAL(pred_updated_bht(std::int32_t)), this, SLOT(update_accuracy_val(std::int32_t)));
    connect(machine->bp(), SIGNAL(pred_reset()), this, SLOT(reset()));
    connect(machine->bp(), SIGNAL(pred_instr_value(const machine::Instruction&)), this, SLOT(update_instr_val(const machine::Instruction&)));
    connect(machine->bp(), SIGNAL(pred_updated_bht(std::int32_t)), pmodel, SLOT(update_pos_bht_update(std::int32_t)));
    connect(machine->bp(), SIGNAL(pred_accessed_bht(std::int32_t)), pmodel, SLOT(update_pos_bht_access(std::int32_t)));
    connect(machine->bp(), SIGNAL(pred_accessed_bht(std::int32_t)), predictor_content, SLOT(focus_row(std::int32_t)));
    connect(machine->bp(), SIGNAL(pred_updated_bht(std::int32_t)), predictor_content, SLOT(focus_row(std::int32_t)));

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
}

void BranchHistoryTableDock::update_pc_val(std::uint32_t inst_addr) {
    QString s,t, text;

    t = QString::number(inst_addr, 16);
    s.fill('0', 8 - t.count());
    text = "0x" + s + t.toUpper();

    set_qline_val(pc_val, text);
}

void BranchHistoryTableDock::update_instr_val(const machine::Instruction &instr) {
    QString s,t, text;

//    t = QString::number(instr.data(), 16);
//    s.fill('0', 8 - t.count());
//    text = "0x" + s + t.toUpper();

    set_qline_val(instr_val, instr.to_str());
}

void BranchHistoryTableDock::update_bht_index_val(std::uint32_t inst_addr) {
    set_qline_val(bht_index_val, QString::number(machine->bp()->bht_idx(inst_addr, true)));
}

void BranchHistoryTableDock::update_accuracy_val(std::int32_t) {
    set_qline_val(accuracy_val, QString::number(machine->bp()->accuracy()) + "%");
}

void BranchHistoryTableDock::reset() {
    qDebug() << "HI BITCH!";
    set_qline_val(instr_val, "");
    set_qline_val(bht_index_val, "");
    set_qline_val(accuracy_val, "100.0%");
}
