#include "controls.h"

#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>

WidgetSpacer::WidgetSpacer(QWidget *parent): QWidget(parent)
{ setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); }

SeparatorH::SeparatorH(QWidget *parent):
    QFrame(parent)
{
    setFrameStyle(QFrame::Sunken | QFrame::HLine);
    setMidLineWidth(2);
    setLineWidth(1);
    setFocusPolicy(Qt::NoFocus);
}

void setWidgetToScreenCenter(QWidget* w)
{
    QGuiApplication::screens();
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect rect = screen->availableGeometry();
    QPoint center = rect.center();

    center.setX(center.x() - (w->width()/2));
    center.setY(center.y() - (w->height()/2));
    w->move(center);
}

void setWidgetToParentCenter(QWidget* w)
{
    QWidget* p = w->parentWidget();
    if(p == nullptr) return;
    QRect rect = p->geometry();
    QPoint center = rect.center();
    center.setX(center.x() - (w->width()/2));
    center.setY(center.y() - (w->height()/2));
    w->move(center);
}

bool LineEditEventFilter::eventFilter(QObject *obj, QEvent *e)
{
    switch(e->type())
    {
    case QEvent::KeyPress:
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            auto lineEdit = reinterpret_cast<QLineEdit *>(parent());
            lineEdit->clear();
            lineEdit->setVisible(false);
        }
        break;
    }
    default: break;
    }

    return QObject::eventFilter(obj, e);
}
