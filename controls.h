#ifndef CONTROLS_H
#define CONTROLS_H

#include <QFrame>
#include <QLineEdit>

class QToolBar;

class WidgetSpacer: public QWidget
{

public:
    explicit WidgetSpacer(QWidget* parent = nullptr);
};

class SeparatorH:  public QFrame
{
public:
    explicit SeparatorH(QWidget* parent = nullptr);
};

class LineEditEventFilter : public QObject
{
public:
    explicit LineEditEventFilter(QLineEdit *parent) : QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *e);
};

/*!
 * поместить виджет по центру экрана
 */
void setWidgetToScreenCenter(QWidget* w);

/*!
 * поместить виджет по центру предка
 */
void setWidgetToParentCenter(QWidget* w);

/*!
 * \brief addToolBarAction - добавить Action на ToolBar с присвоением имени для стилизации через css
 */
void addToolBarAction(QToolBar* bar, QAction *action, const QString& name = "");

#endif // CONTROLS_H
