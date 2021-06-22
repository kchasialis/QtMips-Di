#ifndef BRANCHPREDICTORTABLEVIEW_H
#define BRANCHPREDICTORTABLEVIEW_H

#include <QTableView>

class BranchPredictorTableView : public QTableView {
    Q_OBJECT

    using Super = QTableView;

public:
    BranchPredictorTableView(QWidget *parent);

    void resizeEvent(QResizeEvent *event) override;
    void setModel(QAbstractItemModel *model) override;

public slots:
    void focus_row(std::int32_t row);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void adjustColumnCount();
};

#endif // BRANCHPREDICTORTABLEVIEW_H
