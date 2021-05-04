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
    PredictorTableView(QWidget *parent, QSettings *settings);

    void resizeEvent(QResizeEvent *event) override;
    void setModel(QAbstractItemModel *model) override;
//signals:
//    void address_changed(std::uint32_t address);
//    void adjust_scroll_pos_queue();
//public slots:
//    void go_to_address(std::uint32_t address);
//    void focus_address(std::uint32_t address);
//    void focus_address_with_save(std::uint32_t address);
protected:
    void keyPressEvent(QKeyEvent *event) override;
//private slots:
//    void adjust_scroll_pos_check();
//    void adjust_scroll_pos_process();
//private:
//    void go_to_address_priv(std::uint32_t address);
//    void addr0_save_change(std::uint32_t val);
    void adjustColumnCount();
    QSettings *settings;

//    std::uint32_t initial_address;
//    bool adjust_scroll_pos_in_progress;
//    bool need_addr0_save;
};

#endif // PREDICTORTABLEVIEW_H
