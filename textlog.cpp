#include "textlog.h"
#include "properties.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QScrollBar>

TextLog::TextLog(QWidget *parent)
    :QPlainTextEdit(parent),
    m_AutoScroll(true)
{
    setFont(QFont(config->FontNameEvents(), -1, QFont::ExtraBold));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setReadOnly(true);
    setUndoRedoEnabled(false);

    setLogSize(config->LogSize());

    actionClear = new QAction(QIcon(":/resources/img/delete.svg"), tr("Clear all"), this);
    QObject::connect(actionClear, &QAction::triggered, this, &TextLog::slotClear);

    actionAutoScroll = new QAction(tr("To last message"), this);
    actionAutoScroll->setCheckable(true);
    actionAutoScroll->setChecked(m_AutoScroll);
    QObject::connect(actionAutoScroll, &QAction::triggered, this, &TextLog::slotAutoScroll);
}

void TextLog::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu *menu = createStandardContextMenu();

    menu->addAction(actionClear);

    menu->addAction(actionAutoScroll);
    actionAutoScroll->setChecked(m_AutoScroll);

    menu->exec(event->globalPos());
    delete menu;
}

void TextLog::slotClear()
{
    document()->clear();
    textChanged();
}

void TextLog::addText(const QString &text, bool ishtml)
{
    if(ishtml) appendHtml(text);
    else appendPlainText(text);

    if(m_AutoScroll)
    {
        QScrollBar *vScrollBar = verticalScrollBar();
        vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
    }
}

void TextLog::setAutoScroll(bool value)
{
    if(m_AutoScroll == value) return;

    m_AutoScroll = value;
    Q_EMIT signalAutoScrollChanged(m_AutoScroll);
}

void TextLog::slotAutoScroll()
{
    m_AutoScroll = !m_AutoScroll;
    if(m_AutoScroll)
    {
        QScrollBar *vScrollBar = verticalScrollBar();
        vScrollBar->triggerAction(QScrollBar::SliderToMaximum);
    }
    Q_EMIT signalAutoScrollChanged(m_AutoScroll);
}

void TextLog::setLogSize(int count) { document()->setMaximumBlockCount(count);}
bool TextLog::getAutoScroll() { return m_AutoScroll; }

