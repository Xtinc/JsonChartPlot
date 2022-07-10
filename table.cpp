#include "table.h"
#include <QHeaderView>
#include <QMenu>
#include <QLineEdit>

UTable::UTable(QMap<QString, QString> &map, QWidget *parent) : mMap(map), QTableWidget(0, 3, parent)
{
    setItemDelegate(new varDelegate);
    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    setHorizontalHeaderLabels(QStringList{"Name", "Expression", "Value"});
    addVariables("TimeCnt", "$(TimeCnt)");
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &UTable::customContextMenuRequested, this, &UTable::contextMenu);
    /*Timer = new QTimer(this);
    Timer->start(1000);
    connect(Timer, &QTimer::timeout, this, &UTable::refreshTable);*/
}

void UTable::addVariables(const QString &name, const QString &expression)
{
    QTableWidgetItem *varName = new QTableWidgetItem(name);
    varName->setFlags(varName->flags() ^ (Qt::ItemIsEditable | Qt::ItemIsSelectable));
    varName->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem *varExpr = new QTableWidgetItem(expression);
    varExpr->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem *varValue = new QTableWidgetItem(QString::number(0));
    varValue->setFlags(varValue->flags() ^ (Qt::ItemIsEditable | Qt::ItemIsSelectable));
    varValue->setTextAlignment(Qt::AlignCenter);

    int row = rowCount();
    insertRow(row);
    setItem(row, 0, varName);
    setItem(row, 1, varExpr);
    setItem(row, 2, varValue);
    mMap[name] = expression;
}

void UTable::contextMenu(const QPoint &pos)
{
    const QTableWidgetItem *item = itemAt(pos);
    if (!item)
        return;
    QMenu menu;
    QAction *showAction = menu.addAction("Show Chart");
    QAction *addAction = menu.addAction("add Variables");
    QAction *action = menu.exec(mapToGlobal(pos));
    if (!action)
    {
        return;
    }
    if (action == addAction)
    {
        addVariables("", "");
    }
}

void UTable::refreshTable()
{
    /*
    for (int row = 0; row < rowCount(); ++row)
    {
        auto name = itemAt(row, 0)->text();
        if (mMap.contains(name))
        {
            itemAt(row, 2)->setText(QString::number(mMap[name].second));
        }
    }
    Timer->start(1000);*/
}

varDelegate::varDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *varDelegate::createEditor(QWidget *parent,
                                   const QStyleOptionViewItem & /* option */,
                                   const QModelIndex & /* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
    return editor;
}

void varDelegate::setEditorData(QWidget *editor,
                                const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(value);
}

void varDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    auto value = lineEdit->text();
    model->setData(index, value, Qt::EditRole);
}

void varDelegate::updateEditorGeometry(QWidget *editor,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}