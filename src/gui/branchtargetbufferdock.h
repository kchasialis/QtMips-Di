#ifndef BRANCHTARGETBUFFERDOCK_H
#define BRANCHTARGETBUFFERDOCK_H

#include <QDockWidget>
#include <QTableView>
#include <QHBoxLayout>
#include <qtmipsmachine.h>
#include "branchtargetbuffertableview.h"

class BranchTargetBufferDock : public QDockWidget {
    Q_OBJECT

    using Super = QDockWidget;

public:
    BranchTargetBufferDock(QWidget *parent);

    void setup(machine::QtMipsMachine *machine);
private:
    QTableView *btb_content;
    QHBoxLayout *layout;
};

#endif // BRANCHTARGETBUFFERDOCK_H
