#include "dialogdbtablesearch.h"
#include "properties.h"
#include "controls.h"

#include <QGridLayout>
#include <QLabel>
#include <QToolBar>

DialogDBTableSearch::DialogDBTableSearch(QWidget* parent,
                                         const QString& in_string,
                                         QString* out_string)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog |
                   Qt::CustomizeWindowHint |
                   Qt::WindowTitleHint |
                   Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Search");
    setWindowIcon(QIcon(":/resources/img/search.svg"));
    setModal(true);

    m_string = out_string;

    auto layout = new QGridLayout(this);
    layout->setAlignment(Qt::AlignAbsolute);
    layout->setMargin(1);
    layout->setSpacing(1);

    textSearch = new QLineEdit(this);
    textSearch->setMinimumWidth(SEARCH_LINE_WIDTH);
    textSearch->setText(in_string);
    QObject::connect(textSearch, &QLineEdit::returnPressed, this, &DialogDBTableSearch::slotAccept);

    auto labelText = new QLabel(this);
    labelText->setText("Value: ");
    labelText->setBuddy(textSearch);
    labelText->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto toolBar = new QToolBar();
    toolBar->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));
    toolBar->setMovable(false);

    auto actionAccept = new QAction(QIcon(":/resources/img/yes.svg"), "Accept", this);
    QObject::connect(actionAccept, &QAction::triggered, this, &DialogDBTableSearch::slotAccept);

    auto actionCancel = new QAction(QIcon(":/resources/img/no.svg"), "Cancel", this);
    QObject::connect(actionCancel, &QAction::triggered, [=](){ reject(); });

    toolBar->addWidget(new WidgetSpacer());
    toolBar->addAction(actionAccept);
    toolBar->addAction(actionCancel);

    layout->addWidget(labelText, 0, 0);
    layout->addWidget(textSearch, 0, 1);

    layout->addWidget(toolBar, 1, 1);
    setLayout(layout);
}

void DialogDBTableSearch::slotAccept()
{
    // TODO: DialogSearch::slotAccept
    accept();
}

