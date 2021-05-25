#include "branchtargetbufferdock.h"
#include "branchtargetbuffermodel.h"

BranchTargetBufferDock::BranchTargetBufferDock(QWidget *parent) : Super(parent) {
    QWidget *content = new QWidget();
    layout = new QHBoxLayout();
    btb_content = new BranchTargetBufferTableView(this);

    setObjectName("BranchTargetBuffer");
    setWindowTitle("BranchTargetBuffer");
    layout->addWidget(btb_content);
    content->setLayout(layout);
    setWidget(content);
}

void BranchTargetBufferDock::setup(machine::QtMipsMachine *machine) {
    BranchTargetBufferModel *pmodel = new BranchTargetBufferModel(this);
    pmodel->setup(machine);
    btb_content->setModel(pmodel);
    layout->update();
}
