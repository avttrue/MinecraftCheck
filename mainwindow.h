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
class TextLog;
struct MojangApiProfile;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void openDataBase();

protected:
    void closeEvent(QCloseEvent *event);
    void loadGui();
    void setEnableActions(bool value);
    void showTextEdit(int mode); // mode = 0 - by nick, 1- by id
    void queryDone(bool success);
    void getServersStatus();
    void getPlayerProfile();
    void saveReport();
    int setQueryDataBase(const QString &text, QVector<QVariantList> *answer = nullptr, bool log = false);
    void showServers(QMap<QString, QString> servers);
    void writeProfileToDB(const MojangApiProfile& profile, bool *updated = nullptr);
    QString createTableProfile(const MojangApiProfile& profile, bool updated = false);
    void buildProfileTable(const QString &caption, const QString &profiletable);
    void showDBProfiles(QStringList uuids);
    void showProfile(const MojangApiProfile& profile);
    void showDBInfo();
    void taskSeparator();
    bool checkAnswerDB(QVector<QVariantList> answer, int row, int col);
    void setInformation(const QString &text);

private:
    QSqlDatabase database;
    QProgressBar* progressBar;
    QLabel* labelLocalDB;
    QLabel* labelStatus;
    QTabWidget* tabWidget;
    TextLog* textEvents;
    QTextBrowser* textBrowser;
    QLineEdit* lineEdit;
    QAction* actionCheckServers;
    QAction* actionCheckPerson;
    QAction* actionCheckPersonId;
    QAction* actionSave;
    DBBrowser* dbBrowser;

Q_SIGNALS:
    void signalAbortQuery();

private Q_SLOTS:
    void slotAbout();

};
#endif // MAINWINDOW_H
