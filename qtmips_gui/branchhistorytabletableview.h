#ifndef BRANCHHISTORYTABLETABLEVIEW_H
#define BRANCHHISTORYTABLETABLEVIEW_H

#include <QTableView>

class BranchHistoryTableTableView : public QTableView {
    Q_OBJECT

    using Super = QTableView;

public:
    BranchHistoryTableTableView(QWidget *parent);

    void resizeEvent(QResizeEvent *event) override;
    void setModel(QAbstractItemModel *model) override;

public slots:
    void focus_row(std::int32_t row);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void adjustColumnCount();
};

#endif // BRANCHHISTORYTABLETABLEVIEW_H
