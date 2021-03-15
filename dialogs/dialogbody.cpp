#include "dialogbody.h"

#include <QCoreApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QSizeGrip>
#include <QToolBar>

DialogBody::DialogBody(QWidget* parent,
                       const QString& text,
                       const QString& icon,
                       bool closable,
                       bool modal)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog |
                   Qt::WindowStaysOnTopHint |
                   Qt::FramelessWindowHint);

    setAttribute(Qt::WA_DeleteOnClose, true);
    setModal(modal);
    setStyleSheet(DB_WINDOW_STYLE);

    // основной контейнер
    auto formGridLayout = new QGridLayout(this);
    formGridLayout->setAlignment(Qt::AlignCenter);
    formGridLayout->setMargin(2);
    formGridLayout->setSpacing(0);

    // заголовок
    auto frameCaption = new QFrame();
    frameCaption->setFrameStyle(QFrame::Raised | QFrame::Panel);
    frameCaption->setLineWidth(1);
    auto layoutCaption = new QHBoxLayout();
    layoutCaption->setMargin(1);
    layoutCaption->setSpacing(2);

    m_Caption = new DialogCaption(text);
    auto font_height = QSize(QFontMetrics(m_Caption->font()).height(), QFontMetrics(m_Caption->font()).height());
    setWindowTitle(text);

    if(!icon.isEmpty())
    {
        auto labelIcon = new QLabel();
        labelIcon->setStyleSheet(DB_CAPTION_ICON_STYLE);
        labelIcon->setPixmap(QPixmap(icon).scaled(font_height + DB_CAPTION_DELTA_SIZE,
                                                  Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        layoutCaption->addWidget(labelIcon);
        setWindowIcon(QIcon(icon));
    }

    layoutCaption->addWidget(m_Caption, 1);

    if(closable)
    {
        auto buttonCaption = new QPushButton();
        buttonCaption->setStyleSheet(DB_CAPTION_BUTTON_STYLE);
        buttonCaption->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        buttonCaption->setIconSize(font_height + DB_CAPTION_DELTA_SIZE);
        buttonCaption->setFixedSize(font_height + DB_CAPTION_DELTA_SIZE + QSize(4, 4));
        buttonCaption->setIcon(QIcon(":/resources/img/exit.svg"));
        buttonCaption->setToolTip(tr("Close"));
        buttonCaption->setFlat(true);
        QObject::connect(buttonCaption, &QPushButton::released, this, &QDialog::close);
        layoutCaption->addWidget(buttonCaption);
    }
    frameCaption->setLayout(layoutCaption);

    // содержание
    auto frameContent = new QFrame();
    frameContent->setFrameStyle(QFrame::NoFrame);

    m_ContentGridLayout = new QGridLayout(frameContent);
    m_ContentGridLayout->setAlignment(Qt::AlignCenter);
    m_ContentGridLayout->setMargin(0);
    m_ContentGridLayout->setSpacing(0);
    frameContent->setLayout(m_ContentGridLayout);

    // панель кнопок
    m_ToolBar = new QToolBar();
    m_ToolBar->setMovable(false);
    m_ToolBar->setOrientation(Qt::Horizontal);

    formGridLayout->addWidget(frameCaption, 0, 0, 1, -1);
    formGridLayout->addWidget(frameContent, 1, 0, 1, -1);
    formGridLayout->addWidget(m_ToolBar, 2, 0, 1, 1);
    formGridLayout->addWidget(new QSizeGrip(this), 2, 1, 1, -1, Qt::AlignBottom | Qt::AlignRight);
    setLayout(formGridLayout);

    installEventFilter(this);
}

bool DialogBody::eventFilter(QObject* object, QEvent *event)
{
    auto o = qobject_cast<DialogBody*>(object);
    if(o)
    {
        switch (event->type())
        {
        case QEvent::WindowStateChange:
        {
            if(windowState() == Qt::WindowMinimized ||
                    windowState() == Qt::WindowMaximized)
            {
                setWindowState(static_cast<QWindowStateChangeEvent *>(event)->oldState());
                return true;
            }
            return false;
        }
        case QEvent::Hide:
        case QEvent::Close:
        {
            if(object != this || isMinimized() || isMaximized()) return false;
            Q_EMIT signalSizeChanged(size());
            return true;
        }
        default: { return false; }
        }
    }
    else
    {
        switch (event->type())
        {
        case QEvent::Wheel:
        { return true; }
        default: { return false; }
        }
    }
}

void DialogBody::addDialogContent(QWidget *widget) { m_ContentGridLayout->addWidget(widget); }
QToolBar *DialogBody::ToolBar() const { return m_ToolBar; }

///////////////////////////////////////////////

DialogCaption::DialogCaption(const QString &text, QWidget* parent)
    : QLabel(text, parent)
{
    setStyleSheet(DB_CAPTION_STYLE);
    setText(text);
}

void DialogCaption::setText(const QString& text)
{
    QLabel::setText(QString("<center><h3> %1 </h3></center>").arg(text));
}

void DialogCaption::mousePressEvent(QMouseEvent* event)
{
    m_MouseClick_X = event->x();
    m_MouseClick_Y = event->y();
}

void DialogCaption::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;

    window()->move(event->globalX() - m_MouseClick_X - pos().x(),
                   event->globalY() - m_MouseClick_Y - pos().y());
}

