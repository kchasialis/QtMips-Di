#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "branchpredictor.h"
#include "predictordock.h"
#include "predictormodel.h"
#include "predictortableview.h"

PredictorDock::PredictorDock(QWidget *parent) : Super(parent) {
    setObjectName("Predictor");
    setWindowTitle("Predictor");

    QWidget *content = new QWidget();

    QHBoxLayout *hlayout_top = new QHBoxLayout();
    predictor_content = new PredictorTableView(this);
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

    bht_entries_val->setReadOnly(true);
    history_bits_val->setReadOnly(true);
    bht_entries_val->setReadOnly(true);
    history_bits_val->setReadOnly(true);
    instr_val->setReadOnly(true);
    pc_val->setReadOnly(true);
    bht_index_val->setReadOnly(true);

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

    vlayout->addLayout(hlayout_top);
    vlayout->addLayout(vlayout_mid);
    vlayout->addWidget(predictor_content);

    content->setLayout(vlayout);

    setWidget(content);
}

void PredictorDock::setup(machine::QtMipsMachine *machine) {
    PredictorModel *pmodel = new PredictorModel(this);

    this->machine = machine;
    pmodel->setup(machine);
    predictor_content->setModel(pmodel);
    vlayout->update();

    connect(machine->core(), SIGNAL(fetch_inst_addr_value(std::uint32_t)), this, SLOT(update_pc_val(std::uint32_t)));
    connect(machine->core(), SIGNAL(fetch_instr_instr_value(const machine::Instruction&)), this, SLOT(update_instr_val(const machine::Instruction&)));
    connect(machine->core(), SIGNAL(fetch_inst_addr_value(std::uint32_t)), this, SLOT(update_bht_index_val(std::uint32_t)));

    history_bits_val->setReadOnly(false);
    switch (this->machine->config().branch_unit()) {
    case machine::MachineConfig::BU_ONE_BIT_BP:
        history_bits_val->setText("One Bit");
        break;
    case machine::MachineConfig::BU_TWO_BIT_BP:
        history_bits_val->setText("Two Bit");
        break;
    default:
        SANITY_ASSERT(0, "Debug me.");
    }
    history_bits_val->setReadOnly(true);

    bht_entries_val->setReadOnly(false);
    bht_entries_val->setText(QString::number(pow(2, this->machine->config().bht_bits())));
    bht_entries_val->setReadOnly(true);
}

void PredictorDock::update_pc_val(std::uint32_t inst_addr) {
    QString s,t;

    instr_val->setReadOnly(false);
    t = QString::number(inst_addr, 16);
    s.fill('0', 8 - t.count());
    instr_val->setText("0x" + s + t.toUpper());
    instr_val->setReadOnly(true);
}

void PredictorDock::update_instr_val(const machine::Instruction &instr) {
    QString s,t;

    pc_val->setReadOnly(false);
    t = QString::number(instr.data(), 16);
    s.fill('0', 8 - t.count());
    pc_val->setText("0x" + s + t.toUpper());
    pc_val->setReadOnly(true);
}

void PredictorDock::update_bht_index_val(std::uint32_t inst_addr) {
    bht_index_val->setReadOnly(false);
    bht_index_val->setText(QString::number(machine->bp()->bht_idx(inst_addr, true)));
    bht_index_val->setReadOnly(true);
}
