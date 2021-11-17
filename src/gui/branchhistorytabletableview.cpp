#include <QHeaderView>
#include <QFontMetrics>
#include <QScrollBar>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include "branchhistorytabletableview.h"
#include "branchhistorytablemodel.h"
#include "comboboxitemdelegate.h"
#include "hinttabledelegate.h"

BranchPredictorTableView::BranchPredictorTableView(QWidget *parent) : Super(parent) {
    setItemDelegate(new HintTableDelegate);
    setTextElideMode(Qt::ElideNone);
    verticalHeader()->setVisible(false);
}

void BranchPredictorTableView::adjustColumnCount() {
    QModelIndex idx;
    int cwidth_dh;
    int totwidth;

    BranchHistoryTableModel *m = dynamic_cast<BranchHistoryTableModel*>(model());

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
                                          "STRONGLY_NT").width() + 2;
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(1, cwidth_dh);
    totwidth += cwidth_dh;

    idx = m->index(0, 2);
    cwidth_dh = delegate->sizeHintForText(viewOptions(), idx,
                                          "STRONGLY_NT").width() + 2;
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(2, cwidth_dh);
    totwidth += cwidth_dh;

    totwidth += verticalHeader()->width();
    setColumnHidden(2, totwidth > width());
}

void BranchPredictorTableView::resizeEvent(QResizeEvent *event) {
    Super::resizeEvent(event);
    adjustColumnCount();
}

void BranchPredictorTableView::setModel(QAbstractItemModel *model) {
    // This function assumes that we already have a machine.
    auto *pmodel = dynamic_cast<BranchHistoryTableModel*>(model);
    QVector<QString> items;

    Super::setModel(model);

    if (pmodel && pmodel->getMachine()) {
        switch (pmodel->getMachine()->config().control_hazard_unit()) {
            case machine::MachineConfig::CHU_ONE_BIT_BP:
                items.append("NT");
                items.append("T");
                break;
            case machine::MachineConfig::CHU_TWO_BIT_BP:
                items.append("STRONGLY_NT");
                items.append("WEAKLY_NT");
                items.append("WEAKLY_T");
                items.append("STRONGLY_T");
                break;
            default:
                SANITY_ASSERT(0, "Debug me :)");
        }

        ComboBoxItemDelegate *cb = new ComboBoxItemDelegate(this, items);
        setItemDelegateForColumn(1, cb);
    }
}

void BranchPredictorTableView::focus_row(int32_t row) {
    BranchHistoryTableModel *pmodel = dynamic_cast<BranchHistoryTableModel*>(model());

    scrollTo(pmodel->index(row, 0), QAbstractItemView::PositionAtCenter);
    update();
}

void BranchPredictorTableView::keyPressEvent(QKeyEvent *event) {
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
