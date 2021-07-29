#ifndef BRANCHTARGETBUFFERTABLEVIEW_H
#define BRANCHTARGETBUFFERTABLEVIEW_H

#include <QTableView>

class BranchTargetBufferTableView : public QTableView {
    Q_OBJECT

    using Super = QTableView;

public:
    BranchTargetBufferTableView(QWidget *parent);

    void resizeEvent(QResizeEvent *event) override;

public slots:
    void focus_row(std::int32_t row);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void adjustColumnCount();
};

#endif // BRANCHTARGETBUFFERTABLEVIEW_H
