
#include "dbbrowser.h"
#include "properties.h"
#include "helper.h"

#include <QDebug>
#include <QAction>
#include <QHBoxLayout>
#include <QToolBar>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QSplitter>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlQuery>
#include <QFileDialog>

DBBrowser::DBBrowser(QWidget *parent)
    : QWidget(parent)
{
    auto actionUpdateDB = new QAction(QIcon(":/resources/img/update.svg"), "Update the database view", this);
    actionInsertRow = new QAction(QIcon(":/resources/img/plus.svg"), "Add row", this);
    actionInsertRow->setDisabled(true);
    actionDeleteRow = new QAction(QIcon(":/resources/img/minus.svg"), "Delete row", this);
    actionDeleteRow->setDisabled(true);
    actionSchemaDB = new QAction(QIcon(":/resources/img/tag.svg"), "Display the table schema", this);
    actionSchemaDB->setDisabled(true);
    actionClearTable = new QAction(QIcon(":/resources/img/delete.svg"), "Clear table", this);
    actionClearTable->setDisabled(true);
    actionReport = new QAction(QIcon(":/resources/img/text.svg"), "Report", this);
    actionReport->setDisabled(true);
    auto actionLoad = new QAction(QIcon(":/resources/img/load.svg"), "Load query", this);
    actionSearch = new QAction(QIcon(":/resources/img/search.svg"), "Search player by name", this);
    actionSearch->setDisabled(true);

    connect(actionUpdateDB, &QAction::triggered, this, &DBBrowser::slotRefresh);
    connect(actionSchemaDB, &QAction::triggered, this, &DBBrowser::slotMetaData);
    connect(actionInsertRow, &QAction::triggered, this, &DBBrowser::slotInsertRow);
    connect(actionDeleteRow, &QAction::triggered, this, &DBBrowser::slotDeleteRow);
    connect(actionClearTable, &QAction::triggered, this, &DBBrowser::slotClearTable);
    connect(actionReport, &QAction::triggered, this, &DBBrowser::slotReport);
    connect(actionLoad, &QAction::triggered, this, &DBBrowser::slotLoadQuery);
    connect(actionSearch, &QAction::triggered, this, &DBBrowser::slotSearch);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(1);
    layout->setMargin(1);

    auto toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));
    toolBar->setMovable(false);
    toolBar->addAction(actionUpdateDB);
    toolBar->addAction(actionSchemaDB);
    toolBar->addSeparator();
    toolBar->addAction(actionReport);
    toolBar->addAction(actionSearch);
    toolBar->addAction(actionLoad);
    toolBar->addSeparator();
    if(config->AdvancedDBMode()) toolBar->addAction(actionClearTable);
    if(config->AdvancedDBMode()) toolBar->addAction(actionInsertRow);
    if(config->AdvancedDBMode()) toolBar->addAction(actionDeleteRow);
    layout->addWidget(toolBar);

    tree = new QTreeWidget();
    tree->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));
    tree->header()->hide();
    connect(tree, &QTreeWidget::currentItemChanged, this, &DBBrowser::slotTreeCurrentItemChanged);
    connect(tree, &QTreeWidget::itemActivated, this, &DBBrowser::slotTreeItemActivated);

    auto frameTree = new QFrame();
    auto layoutTree = new QVBoxLayout();
    layoutTree->setAlignment(Qt::AlignTop);
    layoutTree->setSpacing(1);
    layoutTree->setMargin(0);
    frameTree->setLayout(layoutTree);
    layoutTree->addWidget(tree);

    labelTree = new QLabel();
    labelTree->setText("-");
    layoutTree->addWidget(labelTree);

    table = new QTableView();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    //table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionMode(QAbstractItemView::MultiSelection);

    auto frameTable = new QFrame();
    auto layoutTable = new QVBoxLayout();
    layoutTable->setAlignment(Qt::AlignTop);
    layoutTable->setSpacing(1);
    layoutTable->setMargin(0);
    frameTable->setLayout(layoutTable);
    layoutTable->addWidget(table);

    labelTable = new QLabel();
    labelTable->setText("-");
    layoutTable->addWidget(labelTable);

    auto splitter = new QSplitter();
    splitter->setOrientation(Qt::Orientation::Horizontal);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(frameTree);
    splitter->addWidget(frameTable);
    splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
    layout->addWidget(splitter);

    QMetaObject::connectSlotsByName(this);
    if(!config->AdvancedDBMode()) qInfo() << "Advanced DB Mode OFF, see" << APP_CFG << "file";
}

static QString dbCaption(const QSqlDatabase &db)
{
    if(!db.isOpen()) return "error";

    auto caption = QString("[%1] %2%3").
                   arg(db.driverName(),
                       db.userName().isEmpty() ? "" : QString("<%1> ").arg(db.userName()),
                       QFileInfo(db.databaseName()).fileName());
    return caption;
}

void DBBrowser::slotMetaData()
{
    auto cItem = tree->currentItem();
    if (!cItem || !cItem->parent()) return;

    setTreeItemActive(cItem->parent());
    showMetaData(cItem->text(0));
}

void DBBrowser::slotRefresh()
{
    tree->clear();
    auto connectionNames = QSqlDatabase::connectionNames();
    int tableCount = 0;
    bool gotActiveDb = false;

    for(auto name: connectionNames)
    {
        auto root = new QTreeWidgetItem(tree);
        auto db = QSqlDatabase::database(name, false);
        root->setText(0, dbCaption(db));
        root->setIcon(0, QIcon(":/resources/img/database.svg"));
        if(name == activeDB)
        {
            gotActiveDb = true;
            setTreeItemActive(root);
        }
        if(db.isOpen())
        {
            auto tables = db.tables();
            for (int t = 0; t < tables.count(); ++t)
            {
                auto table = new QTreeWidgetItem(root);
                table->setText(0, tables.at(t));
                table->setIcon(0,QIcon(":/resources/img/grid.svg"));
                tableCount++;
            }
        }
    }
    if (!gotActiveDb)
    {
        activeDB = connectionNames.value(0);
        setTreeItemActive(tree->topLevelItem(0));
    }

    tree->doItemsLayout(); // hack
    clearTableView();
    labelTree->setText(QString("<b>Tables: %1</b>").arg(tableCount));
    actionDeleteRow->setEnabled(false);
    actionReport->setEnabled(false);
}

QSqlDatabase DBBrowser::database() const { return QSqlDatabase::database(activeDB); }

static void setBold(QTreeWidgetItem *item, bool bold)
{
    QFont font = item->font(0);
    font.setBold(bold);
    item->setFont(0, font);
}

// сделать активным элемент дерева БД
void DBBrowser::setTreeItemActive(QTreeWidgetItem *item)
{
    if (!item) return;

    for(int i = 0; i < tree->topLevelItemCount(); ++i)
        if (tree->topLevelItem(i)->font(0).bold()) setBold(tree->topLevelItem(i), false);

    setBold(item, true);
    activeDB = QSqlDatabase::connectionNames().value(tree->indexOfTopLevelItem(item));
}

// выбор элемента дерева БД
void DBBrowser::slotTreeCurrentItemChanged(QTreeWidgetItem *current)
{
    actionSchemaDB->setEnabled(current && current->parent());
    actionInsertRow->setEnabled(current && current->parent());
    actionClearTable->setEnabled(current && current->parent());
    actionSearch->setEnabled(current && current->parent());

    if(current && !current->parent()) clearTableView();
}

// активация элемента дерева БД
void DBBrowser::slotTreeItemActivated(QTreeWidgetItem *item)
{
    if (!item) return;

    if (!item->parent()) setTreeItemActive(item);
    else
    {
        setTreeItemActive(item->parent());
        showTable(item->text(0));
    }
}

// отобразить таблицу
void DBBrowser::showTable(const QString &tablename)
{
    auto db = database();
    if(!db.isOpen()) return;

    auto model = new MySqlTableModel(table, db);
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    model->setTable(db.driver()->escapeIdentifier(tablename, QSqlDriver::TableName));
    model->select();

    if (model->lastError().type() != QSqlError::NoError)
    {
        auto message = QString("[!]\tError at displaying the table '%1'.");
        auto error = model->lastError().text().simplified();
        if(error.isEmpty()) error = tablename;
        Q_EMIT signalMessage(message.arg(error));
    }

    table->setModel(model);

    if(config->AdvancedDBMode())
        table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    else
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DBBrowser::slotTableSelectionChanged);

    showTableInfo();
}

// отобразить метаданные
void DBBrowser::showMetaData(const QString &t)
{
    auto db = database();
    if(!db.isOpen()) return;

    auto rec = db.record(t);
    auto model = new QStandardItemModel(table);

    model->insertRows(0, rec.count());
    model->insertColumns(0, 7);

    model->setHeaderData(0, Qt::Horizontal, "Fieldname");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Length");
    model->setHeaderData(3, Qt::Horizontal, "Precision");
    model->setHeaderData(4, Qt::Horizontal, "Required");
    model->setHeaderData(5, Qt::Horizontal, "AutoValue");
    model->setHeaderData(6, Qt::Horizontal, "DefaultValue");

    for(int i = 0; i < rec.count(); ++i)
    {
        auto field = rec.field(i);
        model->setData(model->index(i, 0), field.name());
        model->setData(model->index(i, 1), field.typeID() == -1
                                               ? QString(QMetaType::typeName(static_cast<int>(field.type())))
                                               : QString("%1 (%2)").
                                                 arg(QMetaType::typeName(static_cast<int>(field.type()))).
                                                 arg(field.typeID()));
        model->setData(model->index(i, 2), field.length());
        model->setData(model->index(i, 3), field.precision());
        model->setData(model->index(i, 4),
                       field.requiredStatus() == -1 ? QVariant("?")
                                                    : QVariant(bool(field.requiredStatus())));
        model->setData(model->index(i, 5), field.isAutoValue());
        model->setData(model->index(i, 6), field.defaultValue());
    }

    table->setModel(model);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    actionInsertRow->setDisabled(true);
    actionDeleteRow->setDisabled(true);
    actionReport->setDisabled(true);
}

// вставить строку в таблицу
void DBBrowser::slotInsertRow()
{
    auto model = qobject_cast<QSqlTableModel*>(table->model());
    if (!model) return;

    auto insertIndex = table->currentIndex();
    auto row = insertIndex.row() == -1 ? 0 : insertIndex.row();
    model->insertRow(row);
    insertIndex = model->index(row, 0);
    table->setCurrentIndex(insertIndex);
    table->edit(insertIndex);
    showTableInfo();
}

// удалить строку из таблицы
void DBBrowser::slotDeleteRow()
{
    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model) return;

    if(table->selectionModel()->selectedIndexes().count() == 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm", "Delete a row from a table?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    auto currentSelection = table->selectionModel()->selectedIndexes();
    for(auto index: currentSelection)
    {
        if (index.column() != 0) continue;
        model->removeRow(index.row());
    }
    model->select();
    showTableInfo();
}

// выбор строки в таблице
void DBBrowser::slotTableSelectionChanged()
{
    auto model = qobject_cast<QSqlTableModel*>(table->model());

    if (model && table->selectionModel()->selectedIndexes().count() > 0)
        actionDeleteRow->setEnabled(true);
    else
    {
        actionDeleteRow->setEnabled(false);
        actionReport->setEnabled(false);
        return;
    }

    // проверка по конкретной таблице, для отчёта
    auto item = tree->currentItem();
    if (!item || !item->parent()) return;
    auto tableName = item->text(0);

    if(tableName == "Profiles") actionReport->setEnabled(true); // NOTE: 'Profiles' table
    else actionReport->setEnabled(false);
}

// очистить представление таблицы
void DBBrowser::clearTableView()
{
    labelTable->setText("-");
    auto model = qobject_cast<QSqlTableModel*>(table->model());
    if (!model) return;

    model->clear();
}

void DBBrowser::showTableInfo()
{
    QString info = "error";
    auto db = database();
    auto model = qobject_cast<QSqlTableModel*>(table->model());

    if(db.isOpen() && model)
    {
        auto tablename = model->tableName();
        QSqlQuery query(db);
        auto text = getTextFromRes(":/resources/sql/get_table_rows_count.sql").arg(tablename);
        if(query.exec(text))
        {
            query.first();
            auto value = query.value(0);
            if(value.isValid()) info = value.toString();
        }
    }

    labelTable->setText(QString("<b>Records: %1</b>").arg(info));
}

// очистить содержимое таблицы
void DBBrowser::slotClearTable()
{
    auto item = tree->currentItem();
    if (!item || !item->parent()) return;

    auto tableName = item->text(0);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm", QString("Clear the table '%1'?").arg(tableName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    Q_EMIT signalQuery(getTextFromRes(":/resources/sql/clear_table.sql").arg(tableName));
}

void DBBrowser::slotReport()
{
    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model || table->selectionModel()->selectedIndexes().count() == 0)
    {
        actionReport->setEnabled(false);
        return;
    }

    QStringList answer;
    auto currentSelection = table->selectionModel()->selectedIndexes();
    for(auto index: currentSelection)
    {
        if (index.column() != 0) continue;
        answer.append(model->record(index.row()).field("Uuid").value().toString()); //NOTE: 'Uuid' column

    }
    Q_EMIT signalReport(answer);
}

void DBBrowser::slotLoadQuery()
{
    auto db = database();
    if(!db.isOpen()) return;

    QString filename = QFileDialog::getOpenFileName(
        this, "Open sql query", config->LastDir(), "query (*.sql)");

    if(filename.isNull()) return;

    config->setLastDir(QFileInfo(filename).dir().path());

    auto text = fileToText(filename);

    Q_EMIT signalQuery(text);
}

void DBBrowser::slotSearch()
{
    // TODO:  slotSearch
}

QVariant MySqlTableModel::data(const QModelIndex &idx, int role) const
{
    if (role == Qt::BackgroundRole && isDirty(idx)) return QBrush(QColor(Qt::yellow));
    return QSqlTableModel::data(idx, role);
}
