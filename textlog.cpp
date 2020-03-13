#include "textlog.h"
#include "properties.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QScrollBar>

TextLog::TextLog(QWidget *parent)
    :QPlainTextEdit(parent)
{
    setFont(QFont(config->FontNameEvents(), -1, QFont::ExtraBold));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setReadOnly(true);
    setUndoRedoEnabled(false);

    actionClear = new QAction(QIcon(":/resources/img/delete.svg"), tr("Clear all"), this);
    connect(actionClear, &QAction::triggered, this, &TextLog::slotClear);

    actionAutoScroll = new QAction(tr("To last message"), this);
    actionAutoScroll->setCheckable(true);
    actionAutoScroll->setChecked(isAutoScroll);
    connect(actionAutoScroll, &QAction::triggered, this, &TextLog::slotAutoScroll);
}

void TextLog::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu *menu = createStandardContextMenu();

    menu->addAction(actionClear);

    menu->addAction(actionAutoScroll);
    actionAutoScroll->setChecked(isAutoScroll);

    menu->exec(event->globalPos());
    delete menu;
}

void TextLog::slotClear()
{
    document()->clear();
    messagesCount = 0;
    textChanged();
}

void TextLog::addText(const QString &text)
{
    messagesCount++;
    appendPlainText(text);

    if(isAutoScroll)
    {
        QScrollBar *vScrollBar = verticalScrollBar();
        vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
    }
}

void TextLog::setAutoScroll(bool value)
{
    if(isAutoScroll == value) return;

    isAutoScroll = value;
    Q_EMIT signalAutoScrollChanged(isAutoScroll);
}

void TextLog::slotAutoScroll()
{
    isAutoScroll = !isAutoScroll;
    if(isAutoScroll)
    {
        QScrollBar *vScrollBar = verticalScrollBar();
        vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
    }
    emit signalAutoScrollChanged(isAutoScroll);
}

void TextLog::setLogSize(int count) { document()->setMaximumBlockCount(count);}
bool TextLog::getAutoScroll() { return isAutoScroll; }
int TextLog::getMessagesCount() { return  messagesCount; }
