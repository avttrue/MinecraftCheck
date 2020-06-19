#include "mainwindow.h"
#include "properties.h"
#include "controls.h"
#include "mojangapi.h"
#include "helper.h"
#include "helpergraphics.h"
#include "dbbrowser.h"
#include "splashscreen.h"
#include "textlog.h"
#include <dialogs/dialogvalueslist.h>

#include <QDebug>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QStatusBar>
#include <QTextBrowser>
#include <QToolBar>
#include <QFileDialog>
#include <QDesktopServices>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QTimer>
#include <QPointer>
#include <QSqlDatabase>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/resources/img/mainicon512.png"));
    setWindowTitle(QString("%1 %2").arg(APP_NAME, APP_VERS));
    loadGui();
    setWidgetToScreenCenter(this);
    openDataBase();    
    slotAbout();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // сплеш на завершающие процедуры - Vacuum и пр.
    SplashScreen splash;
    splash.show();
    QEventLoop loop;
    QTimer::singleShot(config->SplashServiceTime(), &loop, &QEventLoop::quit); // без этого не отображается
    loop.exec();

    auto db = QSqlDatabase::database();

    if(db.isOpen())
    {
        if(config->AutoVacuum())
        {
            QString text = getTextFromRes(":/resources/sql/vacuum.sql");
            setQueryDataBase(text);
        }
        db.close();
    }

    QSettings settings(config->PathAppConfig(), QSettings::IniFormat);
    settings.setValue("MainWindow/Height",height());
    settings.setValue("MainWindow/Width",width());

    splash.close();

    config->deleteLater();

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
    QObject::connect(actionCheckPerson, &QAction::triggered, [=]()
                     { showTextEdit(0);
                         setInformation(getTextFromRes(":/resources/tip_body.html").
                                        arg("Enter below a nickname to search using the Mojang Api"));
                     });

    actionCheckPersonId = new QAction(QIcon(":/resources/img/person_id.svg"), "Check player by id", this);
    actionCheckPersonId->setShortcut(Qt::CTRL + Qt::Key_I);
    QObject::connect(actionCheckPersonId, &QAction::triggered, [=]()
                     {
                         showTextEdit(1);
                         setInformation(getTextFromRes(":/resources/tip_body.html").
                                        arg("Enter below a Id to search using the Mojang Api"));
                     });

    actionSave = new QAction(QIcon(":/resources/img/save.svg"), "Save report", this);
    actionSave->setShortcut(Qt::CTRL + Qt::Key_S);
    QObject::connect(actionSave, &QAction::triggered, this, &MainWindow::saveReport);

    auto actionAbort = new QAction(QIcon(":/resources/img/no.svg"), "Abort query", this);
    actionAbort->setShortcut(Qt::CTRL + Qt::Key_X);
    QObject::connect(actionAbort, &QAction::triggered, [=](){ Q_EMIT signalAbortQuery(); });

    auto actionAbout = new QAction(QIcon(":/resources/img/question.svg"), "About", this);
    QObject::connect(actionAbout, &QAction::triggered, this, &MainWindow::slotAbout);

    auto actionSetup = new QAction(QIcon(":/resources/img/setup.svg"), "Settings", this);
    QObject::connect(actionSetup, &QAction::triggered, this, &MainWindow::slotSetup);

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
    tbMain->addWidget(new WidgetSpacer(this));
    tbMain->addAction(actionSetup);
    tbMain->addSeparator();
    tbMain->addAction(actionAbout);
    tbMain->addAction(actionQt);
    tbMain->addAction(actionExit);

    addToolBar(tbMain);

    // вкладки
    tabWidget = new QTabWidget(this);
    tabWidget->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));

    textBrowser = new QTextBrowser(this);
    textBrowser->setOpenLinks(false);
    textBrowser->setUndoRedoEnabled(false);
    QObject::connect(textBrowser, &QTextBrowser::anchorClicked, [=](const QUrl &link)
                     {
                         if(!config->OpenUrls()) return;
                         if(link.isEmpty() || !link.isValid()) return;
                         if (!QDesktopServices::openUrl(link))
                             qCritical() << ": error at QDesktopServices::openUrl(" << link.toString() << ")";
                     });

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
    QObject::connect(lineEdit, &QLineEdit::returnPressed, this, &MainWindow::setPlayerForSearch);
    vblInfo->addWidget(lineEdit, 0, Qt::AlignBottom);
    tabWidget->addTab(infoWidget, QIcon(":/resources/img/info.svg"), "Information");

    dbBrowser = new DBBrowser(this);
    tabWidget->addTab(dbBrowser, QIcon(":/resources/img/database.svg"), "Database");
    QObject::connect(dbBrowser, &DBBrowser::signalMessage, [=](const QString& text)
                     { setTaskSeparator();
                       textEvents->addText(text); });
    QObject::connect(dbBrowser, &DBBrowser::signalReport, this, &MainWindow::showDBProfiles);
    QObject::connect(dbBrowser, &DBBrowser::signalQuery, [=](const QString& text)
                     { QVector<QVariantList> answer;
                       setQueryDataBase(text, &answer, true);
                       tabWidget->setCurrentIndex(2); });
    QObject::connect(dbBrowser, &DBBrowser::signalUpdateProfile, [=](const QString& text)
                     { showTextEdit(1);
                         lineEdit->setText(text);
                         setInformation(getTextFromRes(":/resources/tip_body.html").
                                        arg("Confirm serch by uuid to continue<br>(press 'Enter')"));
                     });

    textEvents = new TextLog(this);
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
    lineEdit->setProperty(LE_PROPERTY_SEARCH, mode);
    if(mode == 0)
        lineEdit->setPlaceholderText("enter the player's NICKNAME here");
    else if(mode == 1)
        lineEdit->setPlaceholderText("enter the player's ID here");
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
    progressBar->setVisible(true);

    QPointer<ServerStatusReader> reader = new ServerStatusReader(this);

    QObject::connect(reader, &ServerStatusReader::signalSuccess, [=]()
                     { queryDone(true); reader->deleteLater(); });
    QObject::connect(reader, &ServerStatusReader::signalError, [=]()
                     { queryDone(false); reader->deleteLater(); });
    QObject::connect(this, &MainWindow::signalAbortQuery, [=]()
                     { if(reader) reader->abort(true); });
    QObject::connect(reader, &ServerStatusReader::signalMessage, [=](QString text)
                     { textEvents->addText(text);
                     QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &ServerStatusReader::signalStatus, [=](QString text)
                     { labelStatus->setText(text);
                     QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &ServerStatusReader::signalServers, this, &MainWindow::showServers);

    setTaskSeparator();

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    reader->sendQuery(config->QueryServers());
}

void MainWindow::setPlayerForSearch()
{
    if(lineEdit->text().isEmpty()) return;

    lineEdit->setVisible(false);
    tabWidget->setCurrentIndex(2);
    setEnableActions(false);    
    textBrowser->clear();
    progressBar->setVisible(true);

    auto text = lineEdit->text().simplified();
    bool mode = lineEdit->property(LE_PROPERTY_SEARCH).toInt();
    getProfile(text, mode == 0 ? true : false);

    lineEdit->clear();
}

void MainWindow::getProfile(const QString &text, bool mode)
{
    QPointer<PlayerProfileReader> reader = new PlayerProfileReader(this);

    QObject::connect(reader, &PlayerProfileReader::signalSuccess, [=]()
                     { queryDone(true); reader->deleteLater(); });
    QObject::connect(reader, &PlayerProfileReader::signalError, [=]()
                     { queryDone(false); reader->deleteLater(); });
    QObject::connect(this, &MainWindow::signalAbortQuery, [=]()
                     { if(reader) reader->abort(true);
                         setInformation(getTextFromRes(":/resources/tip_body.html").
                                              arg("Failed to get data from Mojang")); });
    QObject::connect(reader, &PlayerProfileReader::signalMessage, [=](QString string)
                     { textEvents->addText(string);
                       QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &PlayerProfileReader::signalStatus, [=](QString string)
                     { labelStatus->setText(string);
                       QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);});
    QObject::connect(reader, &PlayerProfileReader::signalProfile, this, &MainWindow::showProfile);

    setTaskSeparator();

    QString query;
    if(mode)
    {
        query = config->QueryPersonName().arg(text);
        textEvents->addText(QString("[i]\tSearching by player's name '%1'\n").arg(text));
    }
    else
    {
        auto id = text; id.remove('{').remove('}').remove('-');
        query = config->QueryPersonUuid().arg(id);
        reader->setProfileId(id);
        textEvents->addText(QString("[i]\tSearching by player's id '%1'\n").arg(id));
    }

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    reader->sendQuery(query);
}

void MainWindow::saveReport()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save report", config->LastDir(), "HTML files (*.html)");

    if(filename.isNull()) return;
    config->setLastDir(QFileInfo(filename).dir().path());
    if(!filename.endsWith(".html", Qt::CaseInsensitive)) filename.append(".html");

    QFile file(filename);

    QString text = config->UseQtHtmlContent()
                       ? textBrowser->toHtml()
                       : textBrowser->property(TB_PROPERTY_CONTENT).toString();

    if(textToFile(text, filename))
    {
        if(!config->ReportAutoOpen()) return;

        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filename)))
            qCritical() << __func__ << ": error at QDesktopServices::openUrl(" << filename << ")";
        return;
    }

    textEvents->addText(QString("[!]\tERROR at file saving: %1").arg(filename));
    tabWidget->setCurrentIndex(2);
}

void MainWindow::openDataBase()
{
    auto db = QSqlDatabase::addDatabase(DATABASE_TYPE);
    db.setDatabaseName(config->PathLocalDB());

    if(db.open())
    {
        if(db.isValid())
        {
            setQueryDataBase("BEGIN TRANSACTION;");
            setQueryDataBase(getTextFromRes(":/resources/sql/create_table_profiles.sql"));
            setQueryDataBase(getTextFromRes(":/resources/sql/create_table_history.sql"));
            setQueryDataBase(getTextFromRes(":/resources/sql/create_table_capes.sql"));
            setQueryDataBase("COMMIT;");
            dbBrowser->slotRefresh();
            showDBInfo();
        }
        else
        {
            textEvents->addText("[!]\tValidation: false\n");
            tabWidget->setCurrentIndex(2);
        }
        return;
    }
    textEvents->addText(QString("[!]\tFATAL ERROR connecting to local database '%1': %2\n").
                        arg(config->PathLocalDB(), db.lastError().text()));
    tabWidget->setCurrentIndex(2);
}

int MainWindow::setQueryDataBase(const QString& text, QVector<QVariantList>* answer, bool log)
{
    auto db = QSqlDatabase::database();

    if(!db.isOpen()) return -1;

    if(log)
    {
        setTaskSeparator();
        textEvents->addText(QString("[i]\tLocal database query: '%1'").arg(text.simplified()));
    }

    auto time = QDateTime::currentMSecsSinceEpoch();

    QSqlQuery query(db);
    if(!query.exec(text))
    {
        auto error = db.lastError().text().simplified();
        if(error.isEmpty()) error = "Incorrect query syntax";
        textEvents->addText(QString("[!]\tFATAL ERROR at into local database query: %1\n").
                            arg(error));
        tabWidget->setCurrentIndex(2);
        return -1;
    }
    else if (query.isSelect())
    {
        int rows = 0;
        if(query.last())
        {
            rows = query.at() + 1;
            query.first();
        }

        if(answer == nullptr || rows <= 0)
        {
            if(log)
            {
                textEvents->addText("[i]\tQuery: answer is empty");
                textEvents->addText(QString("[i]\tQuery was completed in %1 ms").
                                    arg(QString::number(QDateTime::currentMSecsSinceEpoch() - time)));
            }
            return rows;
        }

        answer->clear();

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
        } while(query.next());

        if(log)
        {
            textEvents->addText(QString("[i]\tAnswer: %1").arg(sanswer));
            setTaskSeparator();
            textEvents->addText(QString("[i]\tAnswer len: %1 rows").arg(QString::number(rows)));
            textEvents->addText(QString("[i]\tQuery was completed in %1 ms").
                                arg(QString::number(QDateTime::currentMSecsSinceEpoch() - time)));
        }
        return rows;
    }
    else
    {
        auto raffected =query.numRowsAffected();
        if(raffected > 0 || log)
            textEvents->addText(QString("[i]\tDatabase rows affected: %1").
                                arg(QString::number(raffected)));
        if(log)
        {
            textEvents->addText(QString("[i]\tQuery was completed in %1 ms").
                                arg(QString::number(QDateTime::currentMSecsSinceEpoch() - time)));
        }
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
    auto caption = QString("Status of the Mojang services %1").
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
    table.prepend("<tr><td>");
    table.append("</td></tr>");
    auto html = getTextFromRes(":/resources/report_body.html").
                arg(caption, table, QString::number(config->ReportMargins()));
    setInformation(html);
}

void MainWindow::writeProfileToDB(const MojangApiProfile &profile, bool* updated)
{
    auto db = QSqlDatabase::database();

    if(!db.isOpen()) return;

    bool rezult = false;
    QString comments;
    QString hasCapes = "0";
    QString text = getTextFromRes(":/resources/sql/is_profile_exists_uuid.sql").arg(profile.Id);
    QVector<QVariantList> answer;

    setQueryDataBase(text, &answer);
    if(checkAnswerDB(answer, 1, 1)) // если профиль уже есть
    {
        if(answer.at(0).at(0).toInt() > 0)
        {
            rezult = true;
            textEvents->addText(QString("[i]\tProfile '%1' already EXISTS, rewrites").
                                arg(profile.Id));

            if(config->KeepCommentsAtUpd())
            {
                text = getTextFromRes(":/resources/sql/get_profile_comment.sql").arg(profile.Id);
                setQueryDataBase(text, &answer);
                if(checkAnswerDB(answer, 1, 1)) comments = answer.at(0).at(0).toString();
                if(!comments.isEmpty())
                    textEvents->addText(QString("[i]\tProfile comment will be saved"));
            }

            text = getTextFromRes(":/resources/sql/is_profile_has_capes.sql").arg(profile.Id);
            setQueryDataBase(text, &answer);
            if(checkAnswerDB(answer, 1, 1))
                hasCapes = answer.at(0).at(0).toString();

            if(hasCapes == "1")
                textEvents->addText(QString("[i]\tProfile already had CAPES"));

            setQueryDataBase("BEGIN TRANSACTION;");
            text = getTextFromRes(":/resources/sql/del_record_profile.sql").arg(profile.Id);
            setQueryDataBase(text);
            text = getTextFromRes(":/resources/sql/del_record_history.sql").arg(profile.Id);
            setQueryDataBase(text);

            if(!profile.Capes.isEmpty())
            {
                text = getTextFromRes(":/resources/sql/del_record_cape.sql").
                       arg(profile.Id, profile.Capes.keys().at(0));
                setQueryDataBase(text);
            }
            setQueryDataBase("COMMIT;");
        }
        else
        {
            textEvents->addText(QString("[i]\tProfile '%1' is NEW, will be added").
                                arg(profile.Id));
            rezult = false;
        }
    }

    // внесение данных по профилю в БД
    setQueryDataBase("BEGIN TRANSACTION;");

    if(!profile.Capes.isEmpty())
    {
        textEvents->addText(QString("[i]\tProfile has a CAPE"));
        text = getTextFromRes(":/resources/sql/add_record_cape.sql").
               arg(profile.Id,
                   profile.Capes.keys().at(0),
                   profile.Capes.value(profile.Capes.keys().at(0)));
        setQueryDataBase(text);
        hasCapes = "1";
    }

    if(!profile.NameHistory.isEmpty())
    {
        textEvents->addText(QString("[i]\tProfile has a NAME HISTORY"));
        text = getTextFromRes(":/resources/sql/add_record_history.sql");
        QStringList query;

        for(auto key: profile.NameHistory.keys())
            query.append(QString("('%1', '%2', '%3')").
                         arg(profile.Id, QString::number(key), profile.NameHistory.value(key)));
        setQueryDataBase(text.arg(query.join(",\n")));
    }

    text = getTextFromRes(":/resources/sql/add_record_profile.sql").
           arg(profile.Id, QString::number(profile.DateTime),
               profile.FirstName, profile.CurrentName,
               profile.SkinUrl, profile.Skin, profile.SkinModel,
               QString::number(profile.Legacy),
               QString::number(profile.Demo),
               comments,
               profile.NameHistory.isEmpty() ? "0" : "1",
               hasCapes);
    setQueryDataBase(text);

    setQueryDataBase("COMMIT;");

    dbBrowser->slotRefresh();
    showDBInfo();
    if(updated) *updated = rezult;
}

QString MainWindow::createTableProfile(const MojangApiProfile &profile, bool updated)
{
    auto dts = longTimeToString(profile.DateTime, config->DateTimeFormat()).replace(' ', "&#160;");
    auto caption = QString("Player profile '%1'<br>%2%3").
                   arg(profile.CurrentName,
                       dts,
                       updated ? "<br>(updated)" : "");

    QString report_content;
    report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                  "<h3>ID:&#160;%1</h3></td></tr>").arg(profile.Id));
    report_content.append(QString("<tr><td class='TDTEXT1'>"
                                  "<h3>First name</h3></td>"
                                  "<td class='TDTEXT1'><h3>%1</h3></td></tr>").arg(profile.FirstName));

    if(config->ReportAddPortrait())
    {
        QPixmap portrait = getProfilePortrait(profile.Skin, config->ReportPortraitSize());
        report_content.append(QString("<tr><td class='TDIMG' colspan='2'>"
                                      "<img src='data:image/png;base64,%1' "
                                      "alt='portrait'></td></tr>").
                              arg(getBase64Image(portrait)));
    }

    if(!profile.NameHistory.isEmpty())
    {
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "<h2>Name&#160;history&#160;[%1]</h2></td></tr>").
                              arg(QString::number(profile.NameHistory.keys().count())));
        for(auto key: profile.NameHistory.keys())
        {

            auto dts = longTimeToString(key, config->DateTimeFormat()).replace(' ', "&#160;");
            report_content.append(QString("<tr><td class='TDTEXT1'><h3>%1</h3></td>"
                                          "<td class='TDTEXT1'><h3>%2</h3></td></tr>").
                                  arg(dts, profile.NameHistory.value(key)));
        }
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "&#8212;&#160;&#8212;&#160;&#8212;</td></tr>"));
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

    report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                  "<h2>Skin</h2></td></tr>"));

    QPixmap image = getPixmapFromBase64(profile.Skin);
    report_content.append(QString("<tr><td class='TDIMG' colspan='2'><br>"
                                  "<img src='data:image/png;base64,%1' "
                                  "alt='%2' width='%3' height='%4'><br>").
                          arg(profile.Skin,
                              profile.SkinUrl,
                              QString::number(image.width() * config->ReportImgScale()),
                              QString::number(image.height() * config->ReportImgScale())));
    report_content.append(QString("<a href='%1' title='%1'>link</a><br>&#160;</td></tr>").
                          arg(profile.SkinUrl));

    if(!profile.Capes.isEmpty())
    {
        report_content.append(QString("<tr><td class='TDTEXT2' colspan='2'>"
                                      "<h2>Capes</h2></td></tr>"));

        for(auto key: profile.Capes.keys())
        {
            auto cape = profile.Capes.value(key);
            QPixmap image = getPixmapFromBase64(cape);

            report_content.append(QString("<tr><td class='TDIMG' colspan='2'><br>"
                                          "<img src='data:image/png;base64,%1' "
                                          "alt='%2' width='%3' height='%4'><br>").
                                  arg(cape,
                                      key,
                                      QString::number(image.width() * config->ReportImgScale()),
                                      QString::number(image.height() * config->ReportImgScale())));
            report_content.append(QString("<a href='%1' title='%1'>link</a><br>&#160;</td></tr>").
                                  arg(key));
        }
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

void MainWindow::buildProfileTable(const QString &caption, const QString &profiletable)
{
    auto html = getTextFromRes(":/resources/report_body.html").
                arg(caption, profiletable, QString::number(config->ReportMargins()));
    setInformation(html);
}

void MainWindow::showDBProfiles(QStringList uuids)
{
    setTaskSeparator();
    auto time = QDateTime::currentMSecsSinceEpoch();
    tabWidget->setCurrentIndex(0);
    setEnableActions(false);
    progressBar->setVisible(true);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QStringList list(uuids);
    if(list.isEmpty())
    {
        QVector<QVariantList> answer;
        auto where = dbBrowser->getModelFilter();
        qDebug() << __func__ << ": where =" << where;
        if(where.isEmpty())
            setQueryDataBase(getTextFromRes(":/resources/sql/select_uuid_profiles.sql"), &answer);
        else
            setQueryDataBase(QString(getTextFromRes(":/resources/sql/select_uuid_profiles_where.sql").
                                     arg(where)), &answer);
        for(auto v: answer)
        {
            if(v.isEmpty()) continue;
            list.append(v.at(0).toString());
        }
    }

    if(list.isEmpty())
    {
        qDebug() << __func__ << "profile list is empty";
        progressBar->setVisible(false);
        return;
    }

    QString content;
    int prof_count = 1;
    for(auto uuid: list)
    {
        MojangApiProfile profile;
        QVector<QVariantList> answer_pofile;
        QVector<QVariantList> answer_history;
        QVector<QVariantList> answer_capes;

        setQueryDataBase(getTextFromRes(":/resources/sql/select_profile_uuid.sql").
                         arg(uuid), &answer_pofile);
        if(!checkAnswerDB(answer_pofile, 1, 12))
        {
            textEvents->addText(QString("[!]\tERROR: Incorrect profile '%1' in local DB").
                                arg(uuid));
            continue;
        }

        profile.Id = answer_pofile.at(0).at(0).toString();
        profile.DateTime = answer_pofile.at(0).at(1).toLongLong();
        profile.FirstName = answer_pofile.at(0).at(2).toString();
        profile.CurrentName = answer_pofile.at(0).at(3).toString();
        profile.SkinUrl = answer_pofile.at(0).at(4).toString();
        profile.Skin = answer_pofile.at(0).at(5).toString();
        profile.SkinModel = answer_pofile.at(0).at(6).toString();
        profile.Legacy = answer_pofile.at(0).at(7).toInt();
        profile.Demo = answer_pofile.at(0).at(8).toInt();
        profile.Comment = answer_pofile.at(0).at(9).toString();
        auto hasHistory = answer_pofile.at(0).at(10).toBool();
        auto hasCape = answer_pofile.at(0).at(11).toBool();

        if(hasHistory)
        {
            setQueryDataBase(getTextFromRes(":/resources/sql/select_history_uuid.sql").
                             arg(uuid), &answer_history);
            if(!checkAnswerDB(answer_history, 1, 2))
                textEvents->addText(QString("[!]\tERROR: Name history not found in local DB, uuid: %1").
                                    arg(uuid));

            for(auto list: answer_history)
                profile.NameHistory.insert(list.at(0).toLongLong(), list.at(1).toString());
        }

        if(hasCape)
        {
            setQueryDataBase(getTextFromRes(":/resources/sql/select_capes_uuid.sql").
                             arg(uuid), &answer_capes);
            if(!checkAnswerDB(answer_capes, 1, 2))
                textEvents->addText(QString("[!]\tERROR: Capes not found in local DB, uuid: %1").
                                    arg(uuid));

            for(auto list: answer_capes)
                profile.Capes.insert(list.at(0).toString(), list.at(1).toString());
        }

        auto profiletable = createTableProfile(profile);

        content.append(QString("<tr><td class='TDTEXT3'><h2>%1</h2></td></tr>"
                               "<tr><td class='TDTEXT3'>%2</td></tr>"
                               "<tr><td>&#160;</td></tr>").
                       arg(QString::number(prof_count), profiletable));

        prof_count++;
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    auto dts = longTimeToString(QDateTime::currentMSecsSinceEpoch(),
                                config->DateTimeFormat()).replace(' ', "&#160;");
    auto caption = QString("Profiles report %1").arg(dts);

    buildProfileTable(caption, content);
    setEnableActions(true);
    progressBar->setVisible(false);

    auto currenttime = QDateTime::currentMSecsSinceEpoch();
    textEvents->addText(QString("[i]\tReport was completed in %1 ms, %2 profiles").
                        arg(QString::number(currenttime - time), QString::number(prof_count - 1)));
}

void MainWindow::showProfile(const MojangApiProfile &profile)
{
    bool updated = false;
    if(config->AutoCollectProfiles()) writeProfileToDB(profile, &updated);

    auto dts = longTimeToString(profile.DateTime, config->DateTimeFormat()).replace(' ', "&#160;");
    auto caption = QString("Player profile '%1' %2").
                   arg(profile.CurrentName, dts);

    auto profiletable = createTableProfile(profile, updated);

    profiletable.prepend("<tr><td>");
    profiletable.append("</td></tr>");
    buildProfileTable(caption, profiletable);
}

void MainWindow::showDBInfo()
{
    auto dbsize = humanReadableByteCount(QFileInfo(config->PathLocalDB()).size(), config->SIMetric());
    QVector<QVariantList> rowsinfo;
    setQueryDataBase(getTextFromRes(":/resources/sql/get_table_rows_count.sql").
                     arg("Profiles"), &rowsinfo);

    if(rowsinfo.isEmpty()) return;

    auto dbcount = QString::number(rowsinfo.at(0).at(0).toInt());
    textEvents->addText(QString("[i]\tLocal database: %1 size, %2 profiles count").
                        arg(dbsize, dbcount));
    labelLocalDB->setText(QString("%1, %2 profiles").arg(dbsize, dbcount));
}

void MainWindow::setTaskSeparator()
{
    QString s;
    textEvents->addText(s.fill('-', TASK_SEPARATOR_LEN));
}

bool MainWindow::checkAnswerDB(QVector<QVariantList> answer, int row, int col)
{
    if(answer.isEmpty() || answer.at(0).isEmpty())
    {
        textEvents->addText("[!]\tERROR: empty answer from database");
        return false;
    }

    if(answer.count() < row || answer.at(0).count() < col)
    {
        textEvents->addText(QString("[!]\tERROR: incomplete answer from database: "
                                    "%1 rows < %2, %3 columns < %4").
                            arg(QString::number(answer.count()),
                                QString::number(row),
                                QString::number(answer.at(0).count()),
                                QString::number(col)));
        return false;
    }
    return true;
}

void MainWindow::setInformation(const QString &text)
{
    textBrowser->setText(text);
    if(!config->UseQtHtmlContent())
        textBrowser->setProperty(TB_PROPERTY_CONTENT, text);
}

void MainWindow::slotAbout()
{
    auto html = getTextFromRes(":/resources/about_body.html").
                arg(APP_NAME, APP_VERS, GIT_VERS, BUILD_DATE, getSystemInfo(), QT_VERSION_STR);
    setInformation(html);
    tabWidget->setCurrentIndex(0);
}

void MainWindow::slotSetup()
{
    const QVector<QString> keys = {"00#_DataBase options (after reload application)",
                                   "01#_Advanced mode",
                                   "02#_Skin mode",
                                   "03#_Skin size ('skin' mode)",
                                   "04#_Portrait size ('portrait' mode)",
                                   "05#_Report options (at next report)",
                                   "06#_Show portrait",
                                   "07#_Portrait size",
                                   "08#_Skin scale",
                                   "09#_Show cape",
                                   "10#_Cape size",
                                   "11#_Led size (servise status)",
                                   "12#_Margin (%)",
                                   "13#_Open in system handler at saving",
                                   "14#_Use Qt html",
                                   "15#_Common options",
                                   "16#_Event log size (0 = maximum)",
                                   "17#_Use SI metric (at next recalc)",
                                   "18#_Date and time format"};
    QMap<QString, DialogValue> map =
        {{keys.at(0), {}},
         {keys.at(1), {QVariant::Bool, config->AdvancedDBMode(), 0, 0}},
         {keys.at(2), {QVariant::StringList, config->TableSkinMode(), 0, QStringList({"none", "portrait", "skin"}), DialogValueMode::OneFromList}},
         {keys.at(3), {QVariant::Int, config->TableSkinSize(), 8, 256}},
         {keys.at(4), {QVariant::Int, config->TablePortraitSize(), 8, 256}},
         {keys.at(5), {}},
         {keys.at(6), {QVariant::Bool, config->ReportAddPortrait(), 0, 0}},
         {keys.at(7), {QVariant::Int, config->ReportPortraitSize(), 8, 256}},
         {keys.at(8), {QVariant::Int, config->ReportImgScale(), 1, 10}},
         {keys.at(9), {QVariant::Bool, config->ShowCapeImage(), 0, 0}},
         {keys.at(10), {QVariant::Int, config->TableCapeSize(), 8, 256}},
         {keys.at(11), {QVariant::Int, config->ReportLedSize(), 8, 256}},
         {keys.at(12), {QVariant::Int, config->ReportMargins(), 0, 30}},
         {keys.at(13), {QVariant::Bool, config->ReportAutoOpen(), 0, 0}},
         {keys.at(14), {QVariant::Bool, config->UseQtHtmlContent(), 0, 0}},
         {keys.at(15), {}},
         {keys.at(16), {QVariant::Int, config->LogSize(), 0, 0}},
         {keys.at(17), {QVariant::Bool, config->SIMetric(), 0, 0}},
         {keys.at(18), {QVariant::String, config->DateTimeFormat(), 0, 0}}
        };

    auto dvl = new DialogValuesList(this, ":/resources/img/setup.svg", "Settings", &map);

    if(!dvl->exec()) return;

    config->setAdvancedDBMode(map.value(keys.at(1)).value.toBool());
    config->setTableSkinMode(map.value(keys.at(2)).value.toString());
    config->setTableSkinSize(map.value(keys.at(3)).value.toInt());
    config->setTablePortraitSize(map.value(keys.at(4)).value.toInt());

    config->setReportAddPortrait(map.value(keys.at(6)).value.toBool());
    config->setReportPortraitSize(map.value(keys.at(7)).value.toInt());
    config->setReportImgScale(map.value(keys.at(8)).value.toInt());
    config->setShowCapeImage(map.value(keys.at(9)).value.toBool());
    config->setTableCapeSize(map.value(keys.at(10)).value.toInt());
    config->setReportLedSize(map.value(keys.at(11)).value.toInt());
    config->setReportMargins(map.value(keys.at(12)).value.toInt());
    config->setReportAutoOpen(map.value(keys.at(13)).value.toBool());
    config->setUseQtHtmlContent(map.value(keys.at(14)).value.toBool());

    config->setLogSize(map.value(keys.at(16)).value.toInt());
    config->setSIMetric(map.value(keys.at(17)).value.toBool());
    config->setDateTimeFormat(map.value(keys.at(18)).value.toString());
}
