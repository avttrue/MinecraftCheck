#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

class QLabel;
class QProgressBar;
class QPlainTextEdit;
class QTextBrowser;
class QLineEdit;
class DBBrowser;
struct MojangApiProfile;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event);
    void loadGui();
    void setEnableActions(bool value);
    void showTextEdit(int mode); // mode = 0 - by nick, 1- by id
    void queryDone(bool success);
    void getServersStatus();
    void getPlayerProfile();
    void saveReport();
    void openDataBase();
    int setQueryDataBase(const QString &text, QVector<QVariantList> *answer = nullptr, bool log = false);
    void showServers(QMap<QString, QString> servers);
    void writeProfileToDB(const MojangApiProfile& profile);
    QString createTableProfile(const MojangApiProfile& profile);
    void showProfile(const QString &caption, const QString &profiletable);
    void showDBProfiles(QStringList uuids);
    void writeProfile(const MojangApiProfile& profile);
    void getDBInfo();
    void newTaskMarker();

private:
    QSqlDatabase database;
    QProgressBar* progressBar;
    QLabel* labelLocalDB;
    QLabel* labelStatus;
    QTabWidget* tabWidget;
    QPlainTextEdit* textEvents;
    QTextBrowser* textBrowser;
    QLineEdit* lineEdit;
    QAction* actionCheckServers;
    QAction* actionCheckPerson;
    QAction* actionCheckPersonId;
    QAction* actionSave;
    DBBrowser* dbBrowser;

Q_SIGNALS:
    void signalAbortQuery();

};
#endif // MAINWINDOW_H
