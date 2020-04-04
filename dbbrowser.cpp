
#include "dbbrowser.h"
#include "properties.h"
#include "helper.h"
#include "helpergraphics.h"
#include "dialogs/dialogvalueslist.h"

#include <QDebug>
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
#include <QDateTime>

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
    actionFilter = new QAction(QIcon(":/resources/img/search.svg"), "Set filter", this);
    actionFilter->setShortcut(Qt::CTRL + Qt::Key_F);
    actionFilter->setDisabled(true);
    actionUpdateProfile = new QAction(QIcon(":/resources/img/refresh.svg"), "Reload profile", this);
    actionUpdateProfile->setShortcut(Qt::CTRL + Qt::Key_R);
    actionUpdateProfile->setDisabled(true);
    actionComment = new QAction(QIcon(":/resources/img/edit.svg"), "Edit comments", this);
    actionComment->setShortcut(Qt::CTRL + Qt::Key_E);
    actionComment->setDisabled(true);
    actionView = new QAction(QIcon(":/resources/img/eye.svg"), "View profile", this);
    actionView->setShortcut(Qt::CTRL + Qt::Key_W);
    actionView->setDisabled(true);

    QObject::connect(actionUpdateDB, &QAction::triggered, this, &DBBrowser::slotRefresh);
    QObject::connect(actionSchemaDB, &QAction::triggered, this, &DBBrowser::slotMetaData);
    QObject::connect(actionInsertRow, &QAction::triggered, this, &DBBrowser::slotInsertRow);
    QObject::connect(actionDeleteRow, &QAction::triggered, this, &DBBrowser::slotDeleteRow);
    QObject::connect(actionClearTable, &QAction::triggered, this, &DBBrowser::slotClearTable);
    QObject::connect(actionReport, &QAction::triggered, this, &DBBrowser::slotReport);
    QObject::connect(actionLoad, &QAction::triggered, this, &DBBrowser::slotLoadQuery);
    QObject::connect(actionFilter, &QAction::triggered, this, &DBBrowser::slotSearch);
    QObject::connect(actionUpdateProfile, &QAction::triggered, this, &DBBrowser::slotUpdateProfile);
    QObject::connect(actionComment, &QAction::triggered, this, &DBBrowser::slotComment);
    QObject::connect(actionView, &QAction::triggered, this, &DBBrowser::slotViewProfile);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(1);
    layout->setMargin(1);

    auto toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));
    toolBar->setMovable(false);
    toolBar->addAction(actionUpdateDB);
    toolBar->addAction(actionSchemaDB);
    toolBar->addSeparator();
    toolBar->addSeparator();
    toolBar->addAction(actionView);
    toolBar->addAction(actionUpdateProfile);
    toolBar->addAction(actionReport);
    toolBar->addAction(actionFilter);
    toolBar->addAction(actionComment);
    toolBar->addSeparator();
    toolBar->addAction(actionLoad);
    toolBar->addSeparator();
    if(config->AdvancedDBMode()) toolBar->addAction(actionClearTable);
    if(config->AdvancedDBMode()) toolBar->addAction(actionInsertRow);
    if(config->AdvancedDBMode()) toolBar->addAction(actionDeleteRow);
    layout->addWidget(toolBar);

    tree = new QTreeWidget();
    tree->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));
    tree->header()->hide();
    QObject::connect(tree, &QTreeWidget::currentItemChanged, this, &DBBrowser::slotTreeCurrentItemChanged);
    QObject::connect(tree, &QTreeWidget::itemActivated, this, &DBBrowser::slotTreeItemActivated);

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

    if(config->TableSkinMode().toLower() == "portrait" )
    {
        auto verticalHeader = table->verticalHeader();
        verticalHeader->setDefaultSectionSize(config->TablePortraitSize());
    }
    else if(config->TableSkinMode().toLower() == "skin" )
    {
        auto verticalHeader = table->verticalHeader();
        verticalHeader->setDefaultSectionSize(config->TableSkinSize());
    }

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
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    layout->addWidget(splitter);

    QMetaObject::connectSlotsByName(this);
    if(!config->AdvancedDBMode()) qInfo() << "Advanced DB Mode OFF, see" << APP_CFG << "file";
}

static QString dbCaption(const QSqlDatabase &db)
{
    if(!db.isOpen()) return "ERROR";

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
    actionDeleteRow->setEnabled(false);
    actionUpdateProfile->setEnabled(false);
    actionView->setEnabled(false);
    actionComment->setEnabled(false);
    actionFilter->setEnabled(false);
    actionReport->setEnabled(false);

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
            for(int t = 0; t < tables.count(); ++t)
            {
                auto table = new QTreeWidgetItem(root);
                table->setText(0, tables.at(t));
                table->setIcon(0,QIcon(":/resources/img/grid.svg"));
                tableCount++;
            }
            actionFilter->setEnabled(true);
            actionReport->setEnabled(true);
        }
    }
    if (!gotActiveDb)
    {
        activeDB = connectionNames.value(0);
        setTreeItemActive(tree->topLevelItem(0));
    }

    clearTableView();
    labelTree->setText(QString("<b>Tables: %1</b>").arg(tableCount));

    tree->doItemsLayout(); // hack
    tree->sortItems(0, Qt::AscendingOrder);
    tree->expandAll();
}

QSqlDatabase DBBrowser::database() const { return QSqlDatabase::database(activeDB); }

QString DBBrowser::tableName()
{
    auto item = tree->currentItem();
    if (!item || !item->parent()) return "";
    return item->text(0);
}

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

    // настройка сортировки
    if(tablename == "Profiles") // NOTE: 'Profiles' table
    {
        auto index = getColumnIndex(model, "DateTime"); // NOTE: 'DateTime' column
        if(index > -1) model->setSort(index, Qt::AscendingOrder);
    }
    else if(tablename == "NameHistory") // NOTE: 'NameHistory' table
    {
        auto index = getColumnIndex(model, "Uuid"); // NOTE: 'Uuid' column
        if(index > -1) model->setSort(index, Qt::AscendingOrder);
    }
    else if(tablename == "Capes") // NOTE: 'Capes' table
    {
        auto index = getColumnIndex(model, "Uuid"); // NOTE: 'Uuid' column
        if(index > -1) model->setSort(index, Qt::AscendingOrder);
    }

    if (model->lastError().type() != QSqlError::NoError)
    {
        auto message = QString("[!]\tERROR at displaying the table '%1'.");
        auto error = model->lastError().text().simplified();
        if(error.isEmpty()) error = tablename;
        Q_EMIT signalMessage(message.arg(error));
    }

    table->setModel(model);

    // перенос колонки Profiles.Skin на первое место
    if(tablename == "Profiles" && // NOTE: 'Profiles' table
        (config->TableSkinMode().toLower() == "portrait" ||
         config->TableSkinMode().toLower() == "skin"))
    {
        QHeaderView *header = table->horizontalHeader();
        auto index = getColumnIndex(model, "Skin"); // NOTE: 'Skin' column
        if(index > -1) header->moveSection(index, 0);
    }

    if(config->AdvancedDBMode())
        table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    else
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DBBrowser::slotTableSelectionChanged);

    model->select();
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

    model->setHeaderData(0, Qt::Horizontal, "Field Name");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Length");
    model->setHeaderData(3, Qt::Horizontal, "Precision");
    model->setHeaderData(4, Qt::Horizontal, "Required");
    model->setHeaderData(5, Qt::Horizontal, "Auto Value");
    model->setHeaderData(6, Qt::Horizontal, "Default Value");

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
    showTableInfo();
    actionInsertRow->setDisabled(true);
    actionDeleteRow->setDisabled(true);
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
    showTableInfo(model->filter());
}

// удалить строку из таблицы
void DBBrowser::slotDeleteRow()
{
    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model) return;

    if(table->selectionModel()->selectedRows().count() == 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm", "Delete a row from a table?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    auto currentSelection = table->selectionModel()->selectedRows();
    for(auto index: currentSelection) model->removeRow(index.row());

    model->select();
    showTableInfo(model->filter());
}

// выбор строки в таблице
void DBBrowser::slotTableSelectionChanged()
{
    auto model = qobject_cast<QSqlTableModel*>(table->model());

    if (model && table->selectionModel()->selectedRows().count() > 0)
        actionDeleteRow->setEnabled(true);
    else
    {
        actionDeleteRow->setEnabled(false);
        actionUpdateProfile->setEnabled(false);
        actionComment->setEnabled(false);
        actionView->setEnabled(false);
        return;
    }

    // проверка по конкретной таблице
    actionUpdateProfile->setEnabled(tableName() == "Profiles" &&
                                    table->selectionModel()->selectedRows().count() > 0);

    actionView->setEnabled(tableName() == "Profiles" &&
                           table->selectionModel()->selectedRows().count() > 0);

    actionComment->setEnabled(tableName() == "Profiles" &&
                              table->selectionModel()->selectedRows().count() > 0);
}

// очистить представление таблицы
void DBBrowser::clearTableView()
{
    labelTable->setText("-");
    auto model = qobject_cast<QSqlTableModel*>(table->model());
    if (!model) return;

    model->clear();
}

// отобразить информацию о таблице
QString DBBrowser::showTableInfo(const QString& where)
{
    auto size = getTableSize(where);
    QString info = size < 0 ? "-" : QString::number(size);
    labelTable->setText(QString("<b>Records: %1</b>").arg(info));
    return info;
}

// получить количество строк в таблице
int DBBrowser::getTableSize(const QString& where)
{
    int size = -1;
    auto db = database();
    auto model = qobject_cast<QSqlTableModel*>(table->model());

    if(db.isOpen() && model)
    {
        QSqlQuery query(db);
        auto text = where.isEmpty()
                        ? getTextFromRes(":/resources/sql/get_table_rows_count.sql").
                          arg(tableName())
                        : getTextFromRes(":/resources/sql/get_table_rows_count_where.sql").
                          arg(tableName(), where);
        if(query.exec(text))
        {
            query.first();
            auto value = query.value(0);
            bool ok = false;
            if(value.isValid()) size = value.toInt(&ok);
            if(!ok) return -1;
        }
        else
        {
            auto error = db.lastError().text().simplified();
            qCritical() << "DB error: '" << error << "', query: '" << text.simplified() << "'";
        }
    }

    return size;
}

// выбрать таблицу по имени
void DBBrowser::selectTable(const QString &name)
{
    if(name.isEmpty() || tableName() == name) return;

    QTreeWidgetItemIterator item(tree->topLevelItem(0));
    while(*item)
    {
        if((*item)->text(0) == name)
        {
            (*item)->setSelected(true);
            tree->setCurrentItem(*item);
            slotTreeItemActivated(*item);
            break;
        }
        ++item;
    }
}

void DBBrowser::rowToDialogValueMap(QMap<QString, DialogValue>* map, int row)
{
    if(!map) return;

    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model) return;

    auto count = getTableSize(model->filter());
    if(count <= row || row < 0) return;

    auto record = model->record(row);

    const QVector<QString> keys =
        {"01#_Uuid: ",
         "02#_Portrait: ",
         "03#_FirstName: ",
         "04#_CurrentName: ",
         "05#_Skin: ",
         "06#_SkinUrl: ",
         "07#_SkinModel: ",
         "08#_Comments: ",
         "09#_Name history: ",
         "10#_Capes: "
        };

    QStringList fields;
    fields.append(record.field("Uuid").value().toString()); // NOTE: 'Uuid' column
    fields.append(record.field("FirstName").value().toString()); // NOTE: 'FirstName' column
    fields.append(record.field("CurrentName").value().toString()); // NOTE: 'CurrentName' column
    fields.append(record.field("Skin").value().toString()); // NOTE: 'Skin' column
    fields.append(record.field("SkinUrl").value().toString()); // NOTE: 'SkinUrl' column
    fields.append(record.field("SkinModel").value().toString()); // NOTE: 'SkinModel' column
    fields.append(record.field("Comments").value().toString()); // NOTE: 'Comments' column
    fields.append(record.field("NameHistory").value().toString()); // NOTE: 'NameHistory' column
    fields.append(record.field("Capes").value().toString()); // NOTE: 'Capes' column

    auto scale = config->ReportImgScale();
    auto portrait = getBase64Image(getProfilePortrait(fields.at(3), config->ReportPortraitSize()));
    *map =
        {{keys.at(0), {QVariant::String, fields.at(0), 0, 0, DialogValueMode::Disabled}},
         {keys.at(1), {QVariant::String, portrait, 0, 0, DialogValueMode::Base64Image}},
         {keys.at(2), {QVariant::String, fields.at(1), 0, 0, DialogValueMode::Disabled}},
         {keys.at(3), {QVariant::String, fields.at(2), 0, 0, DialogValueMode::Disabled}},
         {keys.at(4), {QVariant::String, fields.at(3), scale, scale, DialogValueMode::Base64Image}},
         {keys.at(5), {QVariant::String, fields.at(4), 0, 0, DialogValueMode::Disabled}},
         {keys.at(6), {QVariant::String, fields.at(5), 0, 0, DialogValueMode::Disabled}},
         {keys.at(7), {QVariant::String, fields.at(6), 0, 0, DialogValueMode::Disabled}},
         {keys.at(8), {QVariant::String, fields.at(7) == "0" ? "NO" : "YES", "", "", DialogValueMode::Disabled}},
         {keys.at(9), {QVariant::String, fields.at(8) == "0" ? "NO" : "YES", "", "", DialogValueMode::Disabled}}
        };
}

// очистить содержимое таблицы
void DBBrowser::slotClearTable()
{
    auto tablename = tableName();
    if(tablename.isEmpty()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm", QString("Clear the table '%1'?").arg(tablename),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    Q_EMIT signalQuery(getTextFromRes(":/resources/sql/clear_table.sql").arg(tablename));
}

void DBBrowser::slotReport()
{
    auto db = database();
    if(!db.isOpen())
    {
        actionReport->setEnabled(false);
        return;
    }

    selectTable("Profiles"); //NOTE: 'Profiles' table

    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model)
    {
        actionReport->setEnabled(false);
        return;
    }

    QStringList answer;
    auto currentSelection = table->selectionModel()->selectedRows();
    for(auto index: currentSelection)
    {
        answer.append(model->record(index.row()).field("Uuid").value().toString()); //NOTE: 'Uuid' column
    }

    if(answer.isEmpty())
    {
        auto count = getTableSize(model->filter());
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm",
                                      QString("No profiles are selected. "
                                              "Create a report for all (%1) profiles?"
                                              "\nIt may take a long time!").
                                      arg(QString::number(count)),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) return;
    }
    Q_EMIT signalReport(answer);
}

void DBBrowser::slotLoadQuery()
{
    QString filename = QFileDialog::getOpenFileName(this, "SQL scripts", config->LastDir(), "query (*.sql)");

    if(filename.isNull()) return;

    config->setLastDir(QFileInfo(filename).dir().path());

    auto text = fileToText(filename);

    Q_EMIT signalQuery(text);
}

void DBBrowser::slotSearch()
{
    auto db = database();
    if(!db.isOpen())
    {
        actionFilter->setEnabled(false);
        return;
    }

    selectTable("Profiles"); //NOTE: 'Profiles' table

    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model)
    {
        actionFilter->setEnabled(false);
        return;
    }

        const QVector<QString> keys =
            {"3#_Value: ", "1#_Area: ", "2#_Precision: "};
    const QStringList arealist =
        {"NAMES in Profiles", "NAMES in Profiles and History",
         "COMMENTS", "ID in Profiles", "NAME HISTORY", "CAPES"};
    const QStringList preclist =
        {"Equal", "Like", "NOT Equal"};

        QMap<QString, DialogValue>
            map =
                {{keys.at(0), {QVariant::String, ""}},
                 {keys.at(1), {QVariant::StringList, arealist.at(0), "", arealist, DialogValueMode::OneFromList}},
                 {keys.at(2), {QVariant::StringList, preclist.at(0), "", preclist, DialogValueMode::OneFromList}},
                 };

    auto dvl = new DialogValuesList(this, ":/resources/img/search.svg", "Find a profile", &map, keys.at(0));
    if(!dvl->exec()) return;

    auto time = QDateTime::currentMSecsSinceEpoch();
    auto value = map.value(keys.at(0)).value.toString().simplified();

    // Precision
    auto prec = "=";
    if(map.value(keys.at(2)).value.toString() == preclist.at(0)) prec = "=";
    else if(map.value(keys.at(2)).value.toString() == preclist.at(1))
    {
        prec = "LIKE";
        value.prepend("%").append("%");
    }
    else if(map.value(keys.at(2)).value.toString() == preclist.at(2)) prec = "!=";

    // Area
    QString where;
    if(map.value(keys.at(1)).value.toString() == arealist.at(0))
    {
        where = QString("FirstName %1 '%2' OR CurrentName %1 '%2'").arg(prec, value); //NOTE: 'FirstName', 'CurrentName' column
    }
    else if(map.value(keys.at(1)).value.toString() == arealist.at(1))
    {
        auto arg = QString("Name %1 '%2'").arg(prec, value);
        auto text = getTextFromRes(":/resources/sql/select_history_where.sql").arg(arg);
        QStringList idsl;
        QString ids;

        QSqlQuery query(db);
        if(query.exec(text))
        {
            if(query.first())
                do
                {
                    auto value = query.value(0);
                    if(value.isValid()) idsl.append(value.toString());
                } while(query.next());
        }
        else
        {
            auto error = db.lastError().text().simplified();
            qCritical() << "DB error: '" << error << "', query: '" << text.simplified() << "'";
        }

        idsl.removeDuplicates();
        idsl.removeAll("");
        for(auto id: idsl) ids.append(QString(" OR Uuid = '%1'").arg(id)); //NOTE: 'Uuid' column

        where = QString("FirstName %1 '%2' OR CurrentName %1 '%2'%3").arg(prec, value, ids); //NOTE: 'FirstName', 'CurrentName' column
    }
    else if(map.value(keys.at(1)).value.toString() == arealist.at(2))
    {
        where = QString("Comments %1 '%2'").arg(prec, value); //NOTE: 'Comments' column
    }
    else if(map.value(keys.at(1)).value.toString() == arealist.at(3))
    {
        where = QString("Uuid %1 '%2'").arg(prec, value); //NOTE: 'Uuid' column
    }
    else if(map.value(keys.at(1)).value.toString() == arealist.at(4))
    {
        where = QString("NameHistory %1 '%2'").arg(prec, value); //NOTE: 'NameHistory' column
    }
    else if(map.value(keys.at(1)).value.toString() == arealist.at(5))
    {
        where = QString("Capes %1 '%2'").arg(prec, value); //NOTE: 'Capes' column
    }

    model->setFilter(where);
    model->select();
    auto count = showTableInfo(model->filter());
    table->selectRow(0);

    Q_EMIT signalMessage(QString("[i]\tSearching was completed in %1 ms, found %2 records").
                         arg(QString::number(QDateTime::currentMSecsSinceEpoch() - time), count));
}

void DBBrowser::slotUpdateProfile()
{
    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model || table->selectionModel()->selectedRows().count() == 0)
    { actionUpdateProfile->setEnabled(false); return; }

    auto count = table->selectionModel()->selectedRows().count();
    auto currentSelection = table->selectionModel()->selectedRows();
    auto record = model->record(currentSelection.at(count - 1).row());

    auto answer = record.field("Uuid").value().toString(); // NOTE: 'Uuid' column

    Q_EMIT signalUpdateProfile(answer);
}

void DBBrowser::slotComment()
{
    auto model = qobject_cast<QSqlTableModel *>(table->model());
    auto db = database();

    if(!model || !db.isOpen() || table->selectionModel()->selectedRows().count() == 0)
    { actionComment->setEnabled(false); return; }

    auto count = table->selectionModel()->selectedRows().count();
    auto currentSelection = table->selectionModel()->selectedRows();
    auto currentrow = currentSelection.at(count - 1).row();
    auto record = model->record(currentrow);

    auto uuid = record.field("Uuid").value().toString(); // NOTE: 'Uuid' column
    auto curname = record.field("CurrentName").value().toString(); // NOTE: 'CurrentName' column
    auto comments = record.field("Comments").value().toString(); // NOTE: 'Comments' column

    const QVector<QString> keys = {"01#_Name: ", "02#_ID: ", "03#_Value: "};
    QMap<QString, DialogValue> map = {{keys.at(0), {QVariant::String, curname, "", "", DialogValueMode::Disabled}},
                                      {keys.at(1), {QVariant::String, uuid, "", "", DialogValueMode::Disabled}},
                                      {keys.at(2), {QVariant::String, comments}}};

    auto dvl = new DialogValuesList(this, ":/resources/img/edit.svg", "Edit comments", &map, keys.at(2));

    if(!dvl->exec()) return;

    auto newcomments = map.value(keys.at(2)).value.toString().simplified();
    if(newcomments == comments) return;

    QSqlQuery query(db);
    auto text = getTextFromRes(":/resources/sql/update_profile_comment.sql").arg(uuid, newcomments);

    if(query.exec(text))
    {
        table->clearSelection();
        model->select();
        showTableInfo(model->filter());

        // кэшируем
        while(model->canFetchMore() && model->rowCount() < currentrow)
            model->fetchMore();

        auto index = table->model()->index(currentrow, 0);
        table->scrollTo(index, QAbstractItemView::EnsureVisible);
        table->setCurrentIndex(index);
    }
    else
    {
        auto error = db.lastError().text().simplified();
        qCritical() << "DB error: '" << error << "', query: '" << text.simplified() << "'";
    }
}

void DBBrowser::slotViewProfile()
{
    auto db = database();
    if(!db.isOpen()) { actionView->setEnabled(false); return; }

    auto model = qobject_cast<QSqlTableModel *>(table->model());
    if(!model) { actionView->setEnabled(false); return; }

    QMap<QString, DialogValue> map;
    auto count = getTableSize(model->filter());
    auto countsel = table->selectionModel()->selectedRows().count();
    auto currentSelection = table->selectionModel()->selectedRows();
    auto currentrow = currentSelection.at(countsel - 1).row();
    auto pcurrentrow = &currentrow;
    rowToDialogValueMap(&map, currentrow);

    auto dvl = new DialogValuesList(this, ":/resources/img/eye.svg", "View profile", &map, "", false);

    auto actionPrev = new QAction(QIcon(":/resources/img/up_arrow.svg"), "Previous profile", dvl);
    auto actionNext = new QAction(QIcon(":/resources/img/down_arrow.svg"), "Next profile", dvl);

    if(currentrow == 0) actionPrev->setDisabled(true);
    if(count == currentrow + 1) actionNext->setDisabled(true);

    QObject::connect(actionPrev, &QAction::triggered, [=]()
                     {
                         (*pcurrentrow)--;
                         QMap<QString, DialogValue> map;
                         rowToDialogValueMap(&map, *pcurrentrow);
                         dvl->slotLoadContent(&map);

                         auto index = table->model()->index(*pcurrentrow, 0);
                         table->clearSelection();
                         table->setCurrentIndex(index);

                         if(*pcurrentrow == 0) actionPrev->setDisabled(true);
                         if(count > 0) actionNext->setEnabled(true);
                     });
    QObject::connect(actionNext, &QAction::triggered, [=]()
                     {
                         (*pcurrentrow)++;
                         QMap<QString, DialogValue> map;
                         rowToDialogValueMap(&map, *pcurrentrow);
                         dvl->slotLoadContent(&map);

                         auto index = table->model()->index(*pcurrentrow, 0);
                         table->clearSelection();
                         table->setCurrentIndex(index);

                         if(*pcurrentrow > 0) actionPrev->setEnabled(true);
                         if(count == *pcurrentrow + 1) actionNext->setDisabled(true);
                     });

    dvl->addToolbarButton(actionNext);
    dvl->addToolbarButton(actionPrev);

    dvl->exec();
}

int DBBrowser::getColumnIndex(QSqlTableModel *model, const QString& name)
{
    if(!model) return -1;

    for(int i = 0; i < model->columnCount(); i++)
        if(model->record().fieldName(i) == name) return i;

    return -1;
}

QVariant MySqlTableModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::BackgroundRole && isDirty(index)) return QBrush(QColor(Qt::yellow));

    auto tableskinmode = config->TableSkinMode().toLower();
    if(tableskinmode == "portrait" || tableskinmode == "skin")
    {
        auto columnname = headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();
        auto tablename = tableName().remove('"'); // tableName() возвращает значение в кавычках
        if(tablename == "Profiles" && columnname == "Skin") // NOTE: 'Profiles.Skin' column
        {
            QString s_img = QSqlTableModel::data(index, Qt::DisplayRole).toString();
            QPixmap pixmap;
            if(tableskinmode == "portrait")
                pixmap = getProfilePortrait(s_img, config->TablePortraitSize());
            else if(tableskinmode == "skin")
                pixmap = getPixmapFromBase64(s_img, nullptr, config->TableSkinSize());

            if(role == Qt::DisplayRole) return QString();
            if(role == Qt::DecorationRole) return pixmap;
            if(role == Qt::SizeHintRole) return pixmap.size();
        }
    }

    return QSqlTableModel::data(index, role);
}
