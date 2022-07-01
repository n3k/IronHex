#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QItemDelegate>
#include <QMainWindow>

class ItemDelegate : public QItemDelegate
{
    Q_OBJECT

private:
    QMainWindow *main_window;

public:
    explicit ItemDelegate(QObject *parent = 0);
    void set_main_window(QMainWindow *window);

protected:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget * editor, const QModelIndex & index) const;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;

signals:

public slots:

};

#endif // ITEMDELEGATE_H
