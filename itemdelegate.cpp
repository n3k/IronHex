#include "itemdelegate.h"
#include <QLineEdit>
//#include <QIntValidator>
#include <QRegularExpressionValidator>

ItemDelegate::ItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *ItemDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QRegularExpression rx("[a|b|c|d|e|f|\\d]{2}");
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
}


void ItemDelegate::updateEditorGeometry(QWidget *editor,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
