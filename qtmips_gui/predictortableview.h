#ifndef PREDICTORTABLEVIEW_H
#define PREDICTORTABLEVIEW_H

#include <QObject>
#include <QSettings>
#include <QTableView>
#include <QSharedPointer>

class PredictorTableView : public QTableView {
    Q_OBJECT

    using Super = QTableView;

public:
    PredictorTableView(QWidget *parent);

    void resizeEvent(QResizeEvent *event) override;
    void setModel(QAbstractItemModel *model) override;
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void adjustColumnCount();
};

#endif // PREDICTORTABLEVIEW_H
