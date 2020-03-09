#include "mainwindow.h"
#include "properties.h"
#include "controls.h"
#include "mojangapi.h"
#include "helper.h"
#include "helpergraphics.h"
#include "dbbrowser.h"

#include <QDebug>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QProgressBar>
#include <QStatusBar>
#include <QTextBrowser>
#include <QToolBar>
#include <QFileDialog>
#include <QDesktopServices>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/resources/img/mainicon512.png"));
    setWindowTitle(QString("%1 %2").arg(APP_NAME, APP_VERS));
    loadGui();
    setWidgetToScreenCenter(this);
    openDataBase();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(database.isOpen())
    {
        if(config->AutoVacuum())
        {
            QString text = getTextFromRes(":/resources/sql/vacuum.sql");
            setQueryDataBase(text);
        }
        database.close();
    }

    QSettings settings(config->PathAppConfig(), QSettings::IniFormat);
    settings.setValue("MainWindow/Height",height());
    settings.setValue("MainWindow/Width",width());

    event->accept();
}

void MainWindow::loadGui()
{
    // действия
    auto actionQt = new QAction(QIcon(":/resources/img/qt_logo.svg"), "About Qt", this);
    QObject::connect(actionQt, &QAction::triggered, qApp, QApplication::aboutQt);

    auto actionExit = new QAction(QIcon(":/resources/img/exit.svg"), "Exit", this);
    QObject::connect(actionExit, &QAction::triggered, this, &MainWindow::close);

    actionCheckServers = new QAction(QIcon(":/resources/img/check.svg"), "Check servers", this);
    actionCheckServers->setShortcut(Qt::CTRL + Qt::Key_M);    
    QObject::connect(actionCheckServers, &QAction::triggered, this, &MainWindow::getServersStatus);

    actionCheckPerson = new QAction(QIcon(":/resources/img/person.svg"), "Check player by nick", this);
    actionCheckPerson->setShortcut(Qt::CTRL + Qt::Key_N);
    QObject::connect(actionCheckPerson, &QAction::triggered, [this](){ showTextEdit(0); });

    actionCheckPersonId = new QAction(QIcon(":/resources/img/person_id.svg"), "Check player by id", this);
    actionCheckPersonId->setShortcut(Qt::CTRL + Qt::Key_I);
    QObject::connect(actionCheckPersonId, &QAction::triggered, [this](){ showTextEdit(1); });

    actionSave = new QAction(QIcon(":/resources/img/save.svg"), "Save report", this);
    actionSave->setShortcut(Qt::CTRL + Qt::Key_S);
    QObject::connect(actionSave, &QAction::triggered, this, &MainWindow::saveReport);

    auto actionAbort = new QAction(QIcon(":/resources/img/no.svg"), "Abort query", this);
    actionAbort->setShortcut(Qt::CTRL + Qt::Key_X);
    QObject::connect(actionAbort, &QAction::triggered, [=](){ Q_EMIT signalAbortQuery(); });

    //    auto testAction = new QAction(QIcon(":/resources/img/question.svg"), "TEST", this);
    //    QObject::connect(testAction, &QAction::triggered, [=]()
    //                     {});

    // тулбар
    auto tbMain = new QToolBar(this);
    tbMain->setMovable(false);
    tbMain->setOrientation(Qt::Horizontal);
    tbMain->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));

    tbMain->addAction(actionCheckServers);
    tbMain->addAction(actionCheckPerson);
    tbMain->addAction(actionCheckPersonId);
    tbMain->addSeparator();
    tbMain->addAction(actionAbort);
    tbMain->addSeparator();
    tbMain->addAction(actionSave);
    //tbMain->addAction(testAction);
    tbMain->addWidget(new WidgetSpacer(this));
    tbMain->addAction(actionQt);
    tbMain->addAction(actionExit);

    addToolBar(tbMain);

    // вкладки
    tabWidget = new QTabWidget(this);
    tabWidget->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));

    textBrowser = new QTextBrowser(this);
    textBrowser->setOpenLinks(false);
    textBrowser->setUndoRedoEnabled(false);
    auto infoWidget = new QWidget(this);
    auto vblInfo = new QVBoxLayout();
    vblInfo->setSpacing(1);
    vblInfo->setMargin(1);
    infoWidget->setLayout(vblInfo);
    vblInfo->addWidget(textBrowser);

    lineEdit = new QLineEdit(this);
    lineEdit->setClearButtonEnabled(true);
    lineEdit->setVisible(false);
    auto* leef = new LineEditEventFilter(lineEdit);
    lineEdit->installEventFilter(leef);
    QObject::connect(lineEdit, &QLineEdit::returnPressed, this, &MainWindow::getPlayerProfile);
    vblInfo->addWidget(lineEdit, 0, Qt::AlignBottom);
    tabWidget->addTab(infoWidget, QIcon(":/resources/img/info.svg"), "Information");

    dbBrowser = new DBBrowser(this);
    tabWidget->addTab(dbBrowser, QIcon(":/resources/img/database.svg"), "Database");
    QObject::connect(dbBrowser, &DBBrowser::signalMessage, [=](const QString& text)
                     { textEvents->appendPlainText(text);
                       tabWidget->setCurrentIndex(2); });
    QObject::connect(dbBrowser, &DBBrowser::signalReport, this, &MainWindow::showDBProfiles);
    QObject::connect(dbBrowser, &DBBrowser::signalQuery, [=](const QString& text)
                     { QVector<QVariantList> answer;
                       setQueryDataBase(text, &answer, true);
                       tabWidget->setCurrentIndex(2); });

    textEvents = new QPlainTextEdit(this);
    textEvents->setFont(QFont(config->FontNameEvents(), -1, QFont::ExtraBold));
    textEvents->setLineWrapMode(QPlainTextEdit::NoWrap);
    textEvents->setReadOnly(true);
    textEvents->setUndoRedoEnabled(false);
    tabWidget->addTab(textEvents, QIcon(":/resources/img/log.svg"), "Events");

    setCentralWidget(tabWidget);

    // статусбар
    auto statusBar = new QStatusBar(this);
    statusBar->addWidget(new QLabel("Local DB: ", this));
    labelLocalDB = new QLabel(this);
    labelLocalDB->setText("-");
    statusBar->addWidget(labelLocalDB);
    statusBar->addWidget(new QLabel(" | ", this));
    statusBar->addWidget(new QLabel("Current query: ", this));
    labelStatus = new QLabel(this);
    labelStatus->setText("-");
    statusBar->addWidget(labelStatus);

    progressBar = new QProgressBar(this);
    progressBar->setAlignment(Qt::AlignLeft);
    progressBar->setFixedWidth(4 * config->ButtonSize());
    progressBar->setMinimum(0);
    progressBar->setMaximum(0);
    progressBar->setVisible(false);
    statusBar->addPermanentWidget(progressBar);

    setStatusBar(statusBar);

    QSettings settings(config->PathAppConfig(), QSettings::IniFormat);
    resize(settings.value("MainWindow/Width", WINDOW_WIDTH).toInt(),
           settings.value("MainWindow/Height", WINDOW_HEIGHT).toInt());
}

void MainWindow::setEnableActions(bool value)
{
    actionCheckPerson->setEnabled(value);
    actionCheckPersonId->setEnabled(value);
    actionCheckServers->setEnabled(value);
    actionSave->setEnabled(value);
}

void MainWindow::showTextEdit(int mode)
{
    tabWidget->setCurrentIndex(0);
    lineEdit->clear();
    lineEdit->setVisible(true);
    lineEdit->setFocus();
    lineEdit->setProperty("SearchMode", mode);
    if(mode == 0)
        lineEdit->setPlaceholderText("enter the player's NICK");
    else if(mode == 1)
        lineEdit->setPlaceholderText("enter the player's ID");
}

void MainWindow::queryDone(bool success)
{
    success ? tabWidget->setCurrentIndex(0) : tabWidget->setCurrentIndex(2);
    setEnableActions(true);
    progressBar->setVisible(false);
}

void MainWindow::getServersStatus()
{
    lineEdit->setVisible(false);
    tabWidget->setCurrentIndex(2);
    setEnableActions(false);

    QPointer<ServerStatusReader> reader = new ServerStatusReader(this);
    QObject::connect(this, &MainWindow::signalAbortQuery, [=]()
                     { if(reader) reader->abort(true); });
    QObject::connect(reader, &ServerStatusReader::signalSuccess, [=]()
                     { queryDone(true); reader->deleteLater(); });
    QObject::connect(reader, &ServerStatusReader::signalError, [=]()
                     { queryDone(false); reader->deleteLater(); });
    QObject::connect(reader, &ServerStatusReader::signalMessage, [=](QString text)
                     { textEvents->appendPlainText(text);
                     QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &ServerStatusReader::signalStatus, [=](QString text)
                     { labelStatus->setText(text);
                     QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &ServerStatusReader::signalServers, this, &MainWindow::showServers);

    progressBar->setVisible(true);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    newTaskMarker();
    reader->sendQuery(config->QueryServers());
}

void MainWindow::getPlayerProfile()
{
    if(lineEdit->text().isEmpty()) return;
    lineEdit->setVisible(false);
    tabWidget->setCurrentIndex(2);
    setEnableActions(false);    
    textBrowser->clear();

    QPointer<PlayerProfileReader> reader = new PlayerProfileReader(this);
    QObject::connect(this, &MainWindow::signalAbortQuery, [=]()
                     { if(reader) reader->abort(true); });
    QObject::connect(reader, &PlayerProfileReader::signalSuccess, [=]()
                     { queryDone(true); reader->deleteLater(); });
    QObject::connect(reader, &PlayerProfileReader::signalError, [=]()
                     { queryDone(false); reader->deleteLater();
                       textBrowser->setText("Failed to get data from Mojang"); });
    QObject::connect(reader, &PlayerProfileReader::signalMessage, [=](QString text)
                     { textEvents->appendPlainText(text);
                       QScrollBar *vScrollBar = textEvents->verticalScrollBar();
                       vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
                       QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &PlayerProfileReader::signalStatus, [=](QString text)
                     { labelStatus->setText(text);
                       QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &PlayerProfileReader::signalProfile, this, &MainWindow::writeProfile);

    progressBar->setVisible(true);

    newTaskMarker();
    if(lineEdit->property("SearchMode").toInt() == 0)
    {
        auto nick = lineEdit->text().simplified();
        auto query = config->QueryPersonName().arg(nick);
        textEvents->appendPlainText(QString("[i]\tSearching by player's name '%1'\n").arg(nick));
        reader->sendQuery(query);
    }
    else if(lineEdit->property("SearchMode").toInt() == 1)
    {
        auto id = lineEdit->text().simplified().remove('{').remove('}').remove('-');
        auto query = config->QueryPersonUuid().arg(id);
        reader->setProfileId(id);
        textEvents->appendPlainText(QString("[i]\tSearching by player's id '%1'\n").arg(id));
        reader->sendQuery(query);
    }
    else // error
    {
        labelStatus->setText("FATAL ERROR");
        qCritical() << __func__ << ": wrong 'SearchMode' (" << lineEdit->property("SearchMode") << ")";
        progressBar->setVisible(false);
    }

    lineEdit->clear();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindow::saveReport()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save report",
                                                    config->PathApp(), "HTML files (*.html)");

    if(filename.isNull()) return;
    if(!filename.endsWith(".html")) filename.append(".html");

    QFile file(filename);

    if(textToFile(textBrowser->toHtml(), filename))
    {
        if(!config->ReportAutoOpen()) return;

        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filename)))
            qCritical() << __func__ << ": error at QDesktopServices::openUrl(" << filename << ")";
        return;
    }

    textEvents->appendPlainText(QString("[!]\tError at file saving: %1").arg(filename));
    tabWidget->setCurrentIndex(2);
}

void MainWindow::openDataBase()
{
    database = QSqlDatabase::addDatabase(DATABASE_TYPE);
    database.setDatabaseName(config->PathLocalDB());

    if(database.open())
    {
        if(database.isValid())
        {
            setQueryDataBase(getTextFromRes(":/resources/sql/create_table_profiles.sql"));
            setQueryDataBase(getTextFromRes(":/resources/sql/create_table_history.sql"));
            dbBrowser->slotRefresh();
            getDBInfo();
        }
        else
        {
            textEvents->appendPlainText("[!]\tValidation: false\n");
            tabWidget->setCurrentIndex(2);
        }
        return;
    }
    textEvents->appendPlainText(QString("[!]\tError connecting to local database '%1': %2\n").
                                arg(config->PathLocalDB(), database.lastError().text()));
    tabWidget->setCurrentIndex(2);
}

int MainWindow::setQueryDataBase(const QString& text, QVector<QVariantList>* answer, bool log)
{
    if(!database.isOpen()) return -1;

    QSqlQuery query(database);
    if( !query.exec(text))
    {
        auto error = database.lastError().text().simplified();
        if(error.isEmpty()) error = "Incorrect query syntax";
        textEvents->appendPlainText(QString("[!]\tFatal error at into local database query: %1\n").
                                    arg(error));
        tabWidget->setCurrentIndex(2);
        return -1;
    } else if (query.isSelect())
    {
        int rows = 0;
        if(query.last())
        {
            rows = query.at() + 1;
            query.first();
        }

        if(answer == nullptr || rows <= 0) return rows;

        // подготовка ответа
        query.first();
        QString sanswer;
        do
        {
            QVariantList row;
            QString srow;
            for(int i = 0; ;i++)
            {
                auto value = query.value(i);
                if(!value.isValid()) break;
                row.append(value);

                if(!log) continue;
                srow.isEmpty()
                    ? srow.append(value.toString())
                    : srow.append(QString(";%1").arg(value.toString()));
            }
            if(row.isEmpty()) continue;

            answer->append(row);

            if(!log) continue;
            sanswer.append(QString("\n\t%1").arg(srow));
        }
        while(query.next());

        if(log) textEvents->appendPlainText(QString("[i]\tDatabase query answer: %1").arg(sanswer));
        return rows;
    }
    else
    {
        auto raffected =query.numRowsAffected();

        if(raffected > 0)
            textEvents->appendPlainText(QString("[i]\tDatabase rows affected: %1").
                                        arg(QString::number(raffected)));
        return raffected;
    }
}

void MainWindow::showServers(QMap<QString, QString> servers)
{
    auto lsize = config->ReportLedSize();
    auto ledgreen = QString("<img src='%1' alt='green'>").
                    arg(getBase64Image(":/resources/img/led_green.svg", QSize(lsize, lsize), true));
    auto ledred = QString("<img src='%1' alt='red'>").
                  arg(getBase64Image(":/resources/img/led_red.svg", QSize(lsize, lsize), true));
    auto ledyellow = QString("<img src='%1' alt='yellow'>").
                     arg(getBase64Image(":/resources/img/led_yellow.svg", QSize(lsize, lsize), true));
    auto ledunknown = QString("<img src='%1' alt='%2'>").
                      arg(getBase64Image(":/resources/img/question.svg", QSize(lsize, lsize), true), "%1");
    auto caption = QString("Status of the Mojang servers<br>%1").
                   arg(QDateTime::currentDateTime().toString(config->DateTimeFormat()));

    QString report_content;
    for(auto key: servers.keys())
    {
        report_content.append(QString("<tr><td class='TDTEXT1'><h3>%1</h3></td>").arg(key));

        auto value = servers.value(key).toLower();

        if(value == "green")
            report_content.append(QString("<td class='TDIMG'>%1</td></tr>").arg(ledgreen));
        else if(value == "red")
            report_content.append(QString("<td class='TDIMG'>%1</td></tr>").arg(ledred));
        else if(value == "yellow")
            report_content.append(QString("<td class='TDIMG'>%1</td></tr>").arg(ledyellow));
        else
            report_content.append(QString("<td class='TDIMG'>%1</td></tr>").arg(ledunknown.arg(value)));
    }

    auto table = getTextFromRes(":/resources/table_body.html").arg(caption, report_content);
    auto html = getTextFromRes(":/resources/report_body.html").arg(caption, table);
    textBrowser->setText(html);
}

void MainWindow::writeProfileToDB(const MojangApiProfile &profile)
{
    if(!database.isOpen()) return;

    QVector<QVariantList> answer;
    QString text = getTextFromRes(":/resources/sql/is_profile_exists_uuid.sql").arg(profile.Id);
    setQueryDataBase(text, &answer);
    if(answer.isEmpty() || answer.at(0).isEmpty())
        textEvents->appendPlainText("[!]\tError: unexpected response from 'Profiles' table");
    else
    {
        if(answer.at(0).at(0).toInt() > 0)
        {
            textEvents->appendPlainText(QString("[i]\tProfile '%1' already exists, rewrites").
                                        arg(profile.Id));

            //TODO: сохранять comments если есть

            text = getTextFromRes(":/resources/sql/del_record_profile.sql").arg(profile.Id);
            setQueryDataBase(text);

            text = getTextFromRes(":/resources/sql/del_record_history.sql").arg(profile.Id);
            setQueryDataBase(text);
        }
        else
            textEvents->appendPlainText(QString("[i]\tProfile '%1' is new, will be added").
                                        arg(profile.Id));
    }

    text = getTextFromRes(":/resources/sql/add_record_profile.sql").
           arg(profile.Id, QString::number(profile.DateTime),
               profile.FirstName, profile.CurrentName,
               profile.SkinUrl, profile.Skin, profile.SkinModel,
               profile.CapeUrl, profile.Cape,
               QString::number(profile.Legacy),
               QString::number(profile.Demo),
               "", // comments
               profile.NameHistory.isEmpty() ? "0" : "1");
    setQueryDataBase(text);

    if(!profile.NameHistory.isEmpty())
    {
        text = getTextFromRes(":/resources/sql/add_record_history.sql");

        for(auto key: profile.NameHistory.keys())
            setQueryDataBase(text.arg(profile.Id, QString::number(key), profile.NameHistory.value(key)));
    }

    dbBrowser->slotRefresh();
    getDBInfo();
}

QString MainWindow::createTableProfile(const MojangApiProfile &profile)
{
    auto dts = longTimeToString(profile.DateTime, config->DateTimeFormat()).replace(' ', "&#160;");
    auto caption = QString("Player profile '%1'<br>%2").arg(profile.CurrentName, dts);

    QString report_content;
    report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                  "<h3>ID:&#160;%1</h3></td></tr>").arg(profile.Id));
    report_content.append(QString("<tr><td class='TDTEXT1'>"
                                  "<h3>First name</h3></td>"
                                  "<td class='TDTEXT1'><h3>%1</h3></td></tr>").arg(profile.FirstName));
    if(!profile.NameHistory.isEmpty())
    {
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'><h2>Name&#160;history</h2></td></tr>"));
        for(auto key: profile.NameHistory.keys())
        {

            auto dts = longTimeToString(key, config->DateTimeFormat()).replace(' ', "&#160;");
            report_content.append(QString("<tr><td class='TDTEXT1'><h3>%1</h3></td>"
                                          "<td class='TDTEXT1'><h3>%2</h3></td></tr>").
                                  arg(dts, profile.NameHistory.value(key)));
        }
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>&#8212;&#160;&#8212;&#160;&#8212;</td></tr>"));
    }

    auto legacy = profile.Legacy ? "not migrated to mojang.com" : "Ok";
    report_content.append(QString("<tr><td class='TDTEXT1'><h3>Legacy</h3></td>"
                                  "<td class='TDTEXT1'><h3>%1</h3></td></tr>").
                          arg(legacy));

    auto demo = profile.Demo ? "account unpaid" : "Ok";
    report_content.append(QString("<tr><td class='TDTEXT1'><h3>Demo</h3></td>"
                                  "<td class='TDTEXT1'><h3>%1</h3></td></tr>").
                          arg(demo));

    report_content.append(QString("<tr><td class='TDTEXT1'><h3>Model</h3></td>"
                                  "<td class='TDTEXT1'><h3>%1</h3></td></tr>").
                          arg(profile.SkinModel));

    if(!profile.Skin.isEmpty())
    {
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "<h2>Skin</h2></td></tr>"));

        QPixmap image;
        image.loadFromData(QByteArray::fromBase64(profile.Skin.toLatin1()));

        report_content.append(QString("<tr><td class='TDIMG' colspan='2'>"
                                      "<img src='data:image/png;base64,%1' width='%2' height='%3'></td></tr>").
                              arg(profile.Skin,
                                  QString::number(image.width() * config->ReportImgScale()),
                                  QString::number(image.height() * config->ReportImgScale())));
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "<a href='%1' title='%1'>link</a></td></tr>").arg(profile.SkinUrl));
    }

    if(!profile.Cape.isEmpty())
    {
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "<h2>Cape</h2></td></tr>"));

        QPixmap image;
        image.loadFromData(QByteArray::fromBase64(profile.Cape.toLatin1()));

        report_content.append(QString("<tr><td class='TDIMG' colspan='2'>"
                                      "<img src='data:image/png;base64,%1' width='%2' height='%3'></td></tr>").
                              arg(profile.Cape,
                                  QString::number(image.width() * config->ReportImgScale()),
                                  QString::number(image.height() * config->ReportImgScale())));
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "<a href='%1' title='%1'>link</a></td></tr>").arg(profile.CapeUrl));
    }
    if(!profile.Comment.isEmpty())
    {
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "<h2>Comment</h2></td></tr>"));
        report_content.append(QString("<tr><td class='TDTEXT1' colspan='2'>"
                                      "<h3>%1</h3></td></tr>").arg(profile.Comment));
    }

    return getTextFromRes(":/resources/table_body.html").arg(caption, report_content);
}

void MainWindow::showProfile(const QString &caption, const QString &profiletable)
{
    auto html = getTextFromRes(":/resources/report_body.html").arg(caption, profiletable);
    textBrowser->setText(html);
}

void MainWindow::showDBProfiles(QStringList uuids)
{
    newTaskMarker();
    auto time = QDateTime::currentMSecsSinceEpoch();
    setEnableActions(false);
    progressBar->setVisible(true);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QString content;
    int number = 1;
    for(auto uuid: uuids)
    {
        MojangApiProfile profile;
        QVector<QVariantList> answer_pofile;
        QVector<QVariantList> answer_history;

        setQueryDataBase(getTextFromRes(":/resources/sql/select_profile_uuid.sql").arg(uuid), &answer_pofile);

        if(answer_pofile.isEmpty()) continue;
        if(answer_pofile.at(0).length() < 13)
        {
            textEvents->appendPlainText("[!]\tError: unexpected response from 'Profiles' table");
            continue;
        }

        profile.Id = answer_pofile.at(0).at(0).toString();
        profile.DateTime = answer_pofile.at(0).at(1).toLongLong();
        profile.FirstName = answer_pofile.at(0).at(2).toString();
        profile.CurrentName = answer_pofile.at(0).at(3).toString();
        profile.SkinUrl = answer_pofile.at(0).at(4).toString();
        profile.Skin = answer_pofile.at(0).at(5).toString();
        profile.SkinModel = answer_pofile.at(0).at(6).toString();
        profile.CapeUrl = answer_pofile.at(0).at(7).toString();
        profile.Cape = answer_pofile.at(0).at(8).toString();
        profile.Legacy = answer_pofile.at(0).at(9).toInt();
        profile.Demo = answer_pofile.at(0).at(10).toInt();
        profile.Comment = answer_pofile.at(0).at(11).toString();
        auto hasHistory = answer_pofile.at(0).at(12).toBool();

        if(hasHistory)
        {
            setQueryDataBase(getTextFromRes(":/resources/sql/select_history_uuid.sql").arg(uuid), &answer_history);
            for(auto list: answer_history)
            {
                if(list.length() < 2)
                    textEvents->appendPlainText("[!]\tError: unexpected response  from 'NameHistory' table");
                else
                    profile.NameHistory.insert(list.at(0).toLongLong(), list.at(1).toString());
            }
        }

        auto profiletable = createTableProfile(profile);
        content.isEmpty()
            ? content.append(QString("<ul><li><h2>%1</h2>").
                             arg(QString::number(number))).append(profiletable).append("</li>")
            : content.append(QString("<br><li><h2>%1</h2>").
                             arg(QString::number(number))).append(profiletable).append("</li>");

        number++;
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    content.append("</ul>");

    auto dts = longTimeToString(QDateTime::currentMSecsSinceEpoch(),
                                config->DateTimeFormat()).replace(' ', "&#160;");
    auto caption = QString("Profiles report %1").arg(dts);

    showProfile(caption, content);
    tabWidget->setCurrentIndex(0);
    setEnableActions(true);
    progressBar->setVisible(false);

    auto currenttime = QDateTime::currentMSecsSinceEpoch();
    textEvents->appendPlainText(QString("[i]\tReport was completed in %1 ms").
                                arg(QString::number(currenttime - time)));
}

void MainWindow::writeProfile(const MojangApiProfile &profile)
{
    if(config->AutoCollectProfiles()) writeProfileToDB(profile);

    auto dts = longTimeToString(profile.DateTime, config->DateTimeFormat()).replace(' ', "&#160;");
    auto caption = QString("Player profile '%1'<br>%2").arg(profile.CurrentName, dts);

    auto profiletable = createTableProfile(profile);
    showProfile(caption, profiletable);
}

void MainWindow::getDBInfo()
{
    auto dbsize = humanReadableByteCount(QFileInfo(config->PathLocalDB()).size());
    QVector<QVariantList> rowsinfo;
    setQueryDataBase(getTextFromRes(":/resources/sql/get_table_rows_count.sql").arg("Profiles"), &rowsinfo);

    if(rowsinfo.isEmpty()) return;

    auto dbcount = QString::number(rowsinfo.at(0).at(0).toInt());
    textEvents->appendPlainText(QString("[i]\tLocal database: %1 bytes size, %2 profiles count").
                                arg(dbsize, dbcount));
    labelLocalDB->setText(QString("%1, %2 profiles").arg(dbsize, dbcount));
}

void MainWindow::newTaskMarker()
{
    textEvents->appendPlainText("\n--------------------\n");
}
