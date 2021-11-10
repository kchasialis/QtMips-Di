#include "branchtargetbufferdock.h"
#include "branchtargetbuffermodel.h"
#include "branchpredictor.h"
#include "branchtargetbuffer.h"

BranchTargetBufferDock::BranchTargetBufferDock(QWidget *parent) : Super(parent) {
    QWidget *content = new QWidget();
    layout = new QHBoxLayout();
    btb_content = new BranchTargetBufferTableView(this);

    setObjectName("Branch Target Buffer");
    setWindowTitle("Branch Target Buffer");
    layout->addWidget(btb_content);
    content->setLayout(layout);
    setWidget(content);
}

void BranchTargetBufferDock::setup(machine::QtMipsMachine *machine) {
    BranchTargetBufferModel *btb_model = new BranchTargetBufferModel(this);
    btb_model->setup(machine);
    btb_content->setModel(btb_model);
    layout->update();

    if (machine) {
        connect(machine->bp()->btb(), SIGNAL(pred_updated_btb(std::int32_t)), btb_model,
                SLOT(update_pos_btb_update(std::int32_t)));
        connect(machine->bp()->btb(), SIGNAL(pred_accessed_btb(std::int32_t)), btb_model,
                SLOT(update_pos_btb_access(std::int32_t)));
        connect(machine->bp()->btb(), SIGNAL(pred_updated_btb(std::int32_t)), btb_content,
                SLOT(focus_row(std::int32_t)));
        connect(machine->bp()->btb(), SIGNAL(pred_accessed_btb(std::int32_t)), btb_content,
                SLOT(focus_row(std::int32_t)));
    }
}
