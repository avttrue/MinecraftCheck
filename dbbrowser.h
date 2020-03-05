
#ifndef DBBROWSER_H
#define DBBROWSER_H

#include <QWidget>
#include <QSqlTableModel>

class QTreeWidgetItem;
class QTreeWidget;
class QLabel;
class QTableView;

class DBBrowser: public QWidget
{
    Q_OBJECT
public:
    DBBrowser(QWidget *parent = nullptr);

protected:
    QSqlDatabase database() const;
    void setTreeItemActive(QTreeWidgetItem*);
    void showTable(const QString &tablename);
    void showMetaData(const QString &table);
    void clearTableView();
    void showTableInfo();

private:
    QAction* actionSchemaDB;
    QAction* actionInsertRow;
    QAction* actionDeleteRow;
    QAction* actionClearTable;
    QAction* actionReport;
    QAction* actionLoad;
    QTableView *table;
    QTreeWidget *tree;
    QString activeDB;
    QLabel* labelTree;
    QLabel* labelTable;

Q_SIGNALS:
    void signalMessage(const QString& text);
    void signalQuery(const QString& text);
    void signalReport(QStringList list);

public Q_SLOTS:
    void slotRefresh();

private Q_SLOTS:
    void slotTreeCurrentItemChanged(QTreeWidgetItem *current);
    void slotTreeItemActivated(QTreeWidgetItem *item);
    void slotTableSelectionChanged();
    void slotInsertRow();
    void slotDeleteRow();
    void slotMetaData();
    void slotClearTable();
    void slotReport();
    void slotLoadQuery();
};

class MySqlTableModel: public QSqlTableModel
{
    Q_OBJECT
public:
    explicit MySqlTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlTableModel(parent, db)
    {}

    QVariant data(const QModelIndex &idx, int role) const override;
};

#endif
