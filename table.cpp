#include "table.h"
#include "expression.h"
#include <QHeaderView>
#include <QMenu>
#include <QLineEdit>
#include <QDebug>
#include <QInputDialog>

UTable::UTable(QWidget *parent) : QTableWidget(0, 3, parent)
{
    setItemDelegate(new varDelegate);
    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    setHorizontalHeaderLabels(QStringList{"Name", "Expression", "Value"});
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &UTable::customContextMenuRequested, this, &UTable::contextMenu);
}

void UTable::addVariables(const QString &name, const QString &expression, bool userInvoked)
{
    QString realname = name;
    if (userInvoked)
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Set Variable Name",
                                             "Var name:", QLineEdit::Normal,
                                             "tmp", &ok);
        if (ok && !text.isEmpty())
        {
            realname = text;
        }
    }
    QTableWidgetItem *varName = new QTableWidgetItem(realname);
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
    QExpression::m_variables["$" + realname] = 0.0;
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
        addVariables("tmp", "");
        return;
    }
    if (action == showAction)
    {
        findPlotVariables();
        return;
    }
}

void UTable::findPlotVariables()
{
    QMap<QString, QString> list;
    for (const auto &it : selectedItems())
    {
        auto row = it->row();
        list.insert(item(row, 0)->text(), item(row, 1)->text());
    }
    emit plotVariables(list);
}

void UTable::refreshTable()
{
    for (int row = 0; row < rowCount(); ++row)
    {
        auto name = item(row, 0)->text();
        QExpression e(item(row, 1)->text());
        if (e.eval())
        {
            item(row, 2)->setText(QString::number(e.result()));
        }
    }
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