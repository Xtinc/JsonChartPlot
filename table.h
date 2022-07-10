#ifndef TABLE_H
#define TABLE_H

#include <QTableWidget>
#include <QStyledItemDelegate>
#include <QTimer>

class UTable : public QTableWidget
{
    Q_OBJECT
public:
    UTable(QMap<QString, QString> &map, QWidget *parent = nullptr);
    void addVariables(const QString &name, const QString &expr);

private:
    QTimer *Timer;
    QMap<QString, QString> &mMap;
    void contextMenu(const QPoint &pos);
    void refreshTable();
};

class varDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    varDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

#endif