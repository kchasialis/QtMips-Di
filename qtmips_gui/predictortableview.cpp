#include <QHeaderView>
#include <QFontMetrics>
#include <QScrollBar>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include "predictortableview.h"
#include "comboboxitemdelegate.h"
#include "hinttabledelegate.h"
#include "predictormodel.h"

PredictorTableView::PredictorTableView(QWidget *parent, QSettings *settings) : Super(parent) {
    setItemDelegate(new HintTableDelegate);
    setTextElideMode(Qt::ElideNone);
    this->settings = settings;
}

void PredictorTableView::adjustColumnCount() {
    QModelIndex idx;
    int cwidth_dh;
    int totwidth;

    PredictorModel *m = dynamic_cast<PredictorModel*>(model());

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
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(1, cwidth_dh);
    totwidth += cwidth_dh;

    idx = m->index(0, 2);
    cwidth_dh = delegate->sizeHintForText(viewOptions(), idx,
                                          "NT").width() + 2;
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(2, cwidth_dh);
    totwidth += cwidth_dh;

    horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    idx = m->index(0, 3);
    totwidth += delegate->sizeHintForText(viewOptions(), idx,
                                          "100.0").width() + 2;
    totwidth += verticalHeader()->width();
    setColumnHidden(2, totwidth > width());
}

void PredictorTableView::resizeEvent(QResizeEvent *event) {
    Super::resizeEvent(event);
    adjustColumnCount();
}

void PredictorTableView::setModel(QAbstractItemModel *model) {
    PredictorModel *pmodel = dynamic_cast<PredictorModel*>(model);
    QVector<QString> items;

    Super::setModel(model);

    if (pmodel) {
        ComboBoxItemDelegate *cb = new ComboBoxItemDelegate(this);

        switch (pmodel->getMachine()->config().branch_unit()) {
        case machine::MachineConfig::BU_ONE_BIT_BP:
            items.append("NT");
            items.append("T");
            break;
        case machine::MachineConfig::BU_TWO_BIT_BP:
            items.append("STRONGLY_NT");
            items.append("WEAKLY_NT");
            items.append("WEAKLY_T");
            items.append("STRONGLY_T");
            break;
        default:
            SANITY_ASSERT(0, "Debug me :)");
        }

        cb->setItems(items);
        setItemDelegateForColumn(1, cb);
    }
}

void PredictorTableView::keyPressEvent(QKeyEvent *event) {
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
