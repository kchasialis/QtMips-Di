#include <QApplication>
#include <QKeyEvent>
#include <QClipboard>
#include <QHeaderView>
#include "predictortableview.h"
#include "comboboxitemdelegate.h"
#include "hinttabledelegate.h"
#include "branchtargetbuffertableview.h"
#include "branchtargetbuffermodel.h"

BranchTargetBufferTableView::BranchTargetBufferTableView(QWidget *parent) : Super(parent) {
    setItemDelegate(new HintTableDelegate);
    setTextElideMode(Qt::ElideNone);
    verticalHeader()->setVisible(false);
}

void BranchTargetBufferTableView::adjustColumnCount() {
    QModelIndex idx;
    int cwidth_dh;
    int totwidth;

    BranchTargetBufferModel *m = dynamic_cast<BranchTargetBufferModel*>(model());

    if (m == nullptr)
        return;

    HintTableDelegate *delegate = dynamic_cast<HintTableDelegate*>(itemDelegate());
    if (delegate == nullptr)
        return;

    idx = m->index(0, 0);
    cwidth_dh = delegate->sizeHintForText(viewOptions(), idx,
                                          "100000").width() + 2;
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(0, cwidth_dh);
    totwidth = cwidth_dh;

    idx = m->index(0, 1);
    cwidth_dh = delegate->sizeHintForText(viewOptions(), idx,
                                          "1").width() + 2;
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(1, cwidth_dh);
    totwidth += cwidth_dh;

    idx = m->index(0, 2);
    cwidth_dh = delegate->sizeHintForText(viewOptions(), idx,
                                          "0xdeadbeef").width() + 2;
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(2, cwidth_dh);
    totwidth += cwidth_dh;

    horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    idx = m->index(0, 3);
    totwidth += delegate->sizeHintForText(viewOptions(), idx,
                                          "0xdeadbeef").width() + 2;
    totwidth += verticalHeader()->width();
    setColumnHidden(2, totwidth > width());
}

void BranchTargetBufferTableView::resizeEvent(QResizeEvent *event) {
    Super::resizeEvent(event);
    adjustColumnCount();
}

void BranchTargetBufferTableView::keyPressEvent(QKeyEvent *event) {
    if(event->matches(QKeySequence::Copy)) {
            QString text;
            QItemSelectionRange range = selectionModel()->selection().first();
            for (auto i = range.top(); i <= range.bottom(); ++i)
            {
                QStringList rowContents;
                for (auto j = range.left(); j <= range.right(); ++j)
                    rowContents << model()->index(i,j).data().toString();
                text += rowContents.join("\t");
                text += "\n";
            }
            QApplication::clipboard()->setText(text);
    } else
        Super::keyPressEvent(event);
}
