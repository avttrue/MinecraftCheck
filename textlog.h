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
    int getMessagesCount();
    void addText(const QString& text);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QAction* actionClear;
    QAction* actionAutoScroll;

    int messagesCount = 0;
    bool isAutoScroll = true;

public Q_SLOTS:
    void slotClear();
    void slotAutoScroll();

Q_SIGNALS:
    void signalAutoScrollChanged(bool value);
};

#endif // TEXTLOG_H
