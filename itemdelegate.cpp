#include "itemdelegate.h"
#include <QLineEdit>
//#include <QIntValidator>
#include <QRegularExpressionValidator>
#include "mainwindow.h"

ItemDelegate::ItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void ItemDelegate::set_main_window(QMainWindow *window) {
    this->main_window = window;
}

QWidget *ItemDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QRegularExpression rx("[aA|bB|cC|dD|eE|fF|\\d]{2}");
    QValidator *validator = new QRegularExpressionValidator(rx, parent);

    QLineEdit *editor = new QLineEdit(parent);

    editor->setValidator(validator);
    return editor;
}


void ItemDelegate::setEditorData(QWidget *editor,
                                 const QModelIndex &index) const
{
    QString value =index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *line = static_cast<QLineEdit*>(editor);
        line->setText(value);
}


void ItemDelegate::setModelData(QWidget *editor,
                                QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    QLineEdit *line = static_cast<QLineEdit*>(editor);
    QString value = line->text().toUpper();
    model->setData(index, value);
    //MainWindow *window = (MainWindow *) this->main_window;
    //window->re_load_ascii_area();

}


void ItemDelegate::updateEditorGeometry(QWidget *editor,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
