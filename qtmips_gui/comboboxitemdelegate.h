#ifndef COMBOBOXITEMDELEGATE_H
#define COMBOBOXITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QVector>

class ComboBoxItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

    using Super = QStyledItemDelegate;
public:
    ComboBoxItemDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void setItems(const QVector<QString> &items);

private:
   QVector<QString> items;
};

#endif // COMBOBOXITEMDELEGATE_H
