#ifndef TEXTLOG_H
#define TEXTLOG_H

#include <QPlainTextEdit>

class TextLog : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextLog(QWidget *parent = nullptr);
    void setLogSize(int count = 0);
    bool getAutoScroll();
    void setAutoScroll(bool value);
    void addText(const QString& text, bool ishtml = false);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QAction* actionClear;
    QAction* actionAutoScroll;
    bool m_AutoScroll;

public Q_SLOTS:
    void slotClear();
    void slotAutoScroll();

Q_SIGNALS:
    void signalAutoScrollChanged(bool value);
};

#endif // TEXTLOG_H
