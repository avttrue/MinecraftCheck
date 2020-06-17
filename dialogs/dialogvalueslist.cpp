#include "dialogvalueslist.h"
#include "controls.h"
#include "properties.h"
#include "helpergraphics.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QScrollArea>
#include <QSpinBox>
#include <QGridLayout>
#include <QToolBar>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QColorDialog>
#include <QWindowStateChangeEvent>

DialogValuesList::DialogValuesList(QWidget* parent,
                                   const QString& icon,
                                   const QString& caption,
                                   QMap<QString, DialogValue> *values,
                                   const QString &focusedKey,
                                   bool dialogMode) :
      QDialog(parent)
{
    m_Values = values;
    m_DialogMode = dialogMode;
    m_FocusedKey = focusedKey;
    setWindowFlags(Qt::Dialog |
                   Qt::CustomizeWindowHint |
                   Qt::WindowTitleHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(caption);
    setWindowIcon(QIcon(icon));
    setModal(m_DialogMode);

    auto vblForm = new QVBoxLayout();
    vblForm->setAlignment(Qt::AlignAbsolute);
    vblForm->setMargin(0);
    vblForm->setSpacing(0);
    setLayout(vblForm);

    auto saContent = new QScrollArea();
    saContent->setAlignment(Qt::AlignTop);
    saContent->setWidgetResizable(true);

    auto wContent = new QWidget();
    saContent->setWidget(wContent);

    glContent = new QGridLayout();
    wContent->setLayout(glContent);
    glContent->setAlignment(Qt::AlignTop);

    toolBar = new QToolBar();
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));

    toolBar->addWidget(new WidgetSpacer());

    if(m_DialogMode)
    {
        auto actionAccept = new QAction(QIcon(":/resources/img/yes.svg"), "Accept");
        actionAccept->setAutoRepeat(false);
        actionAccept->setShortcut(Qt::CTRL + Qt::Key_Q);
        QObject::connect(actionAccept, &QAction::triggered, [=](){ accept(); });
        toolBar->addAction(actionAccept);
    }
    auto actionCancel = new QAction(QIcon(":/resources/img/no.svg"), "Cancel");
    actionCancel->setAutoRepeat(false);
    QObject::connect(actionCancel, &QAction::triggered, [=](){ reject(); });
    toolBar->addAction(actionCancel);

    vblForm->addWidget(saContent);
    vblForm->addWidget(toolBar);

    slotLoadContent(values);

    resize(WINDOW_SIZE);
}

void DialogValuesList::addWidgetContent(QWidget *widget)
{
    glContent->addWidget(widget, glContent->count(), 0, 1, 1, Qt::AlignTop);
}

void DialogValuesList::addToolbarButton(QAction *action)
{
    toolBar->insertAction(toolBar->actions().first(), action);
}

void DialogValuesList::slotLoadContent(QMap<QString, DialogValue>* values)
{
    QLayoutItem *child;
    while ((child = glContent->takeAt(0)) != nullptr) delete child->widget();

    if(!values) { qCritical() << __func__ << ": Values is empty"; return; }

    for (auto key: values->keys())
    {
        QVariant::Type t = values->value(key).type;
        QVariant v = values->value(key).value;
        QVariant minv = values->value(key).minValue;
        QVariant maxv = values->value(key).maxValue;
        QString text = key; text.remove(QRegExp(RE_NUM_MARK));

        if(t == QVariant::Invalid ||
            values->value(key).mode == DialogValueMode::Caption)
        {
            auto widget = new QFrame();
            widget->setFrameStyle(QFrame::Raised | QFrame::Panel);
            auto bl = new QHBoxLayout();
            bl->setMargin(0);

            auto label = new QLabel(widget);
            label->setText(QString("<b>%1</b>").arg(text));
            label->setWordWrap(true);

            auto effect = new QGraphicsDropShadowEffect(widget);
            effect->setOffset(CAPTION_EFFECT_OFFSET, CAPTION_EFFECT_OFFSET);
            effect->setColor(widget->palette().color(QPalette::Base));
            label->setGraphicsEffect(effect);

            QFont font = label->font();
            font.setPointSizeF(font.pointSizeF() + CAPTION_FONT_UP);
            //font.setUnderline(true);
            label->setFont(font);

            bl->addWidget(label, 0);
            widget->setLayout(bl);
            addWidgetContent(widget);
            continue;
        }

        if(values->value(key).mode == DialogValueMode::Disabled)
        {
            auto widget = new QWidget();
            auto bl = new QHBoxLayout();
            bl->setMargin(0);
            bl->setSpacing(1);
            auto label = new QLabel(widget);
            label->setText(QString("<b>%1</b>").arg(text));
            label->setWordWrap(true);
            bl->addWidget(label, 0);
            auto le = new QLineEdit(v.toString(), widget);
            le->setReadOnly(true);
            le->setCursorPosition(0);
            QPalette pal;
            pal.setColor(QPalette::Base, palette().color(QPalette::Button));
            pal.setColor(QPalette::Text, palette().color(QPalette::ButtonText));
            le->setPalette(pal);
            bl->addWidget(le, 1);
            widget->setLayout(bl);
            addWidgetContent(widget);
            continue;
        }

        if(values->value(key).mode == DialogValueMode::Color)
        {
            auto widget = new QWidget();
            auto bl = new QVBoxLayout();
            bl->setMargin(0);
            bl->setSpacing(1);
            auto label = new QLabel(widget);
            label->setText(QString("<b>%1</b>").arg(text));
            label->setWordWrap(true);
            bl->addWidget(label, 0);

            auto btn = new QPushButton(v.toString(), widget);
            btn->setFixedHeight(config->ButtonSize());
            btn->setAutoFillBackground(true);
            btn->setProperty("ValueName", key);
            btn->setStyleSheet(BTN_COLOR_STYLE.arg(v.toString(), GetContrastColor(QColor(v.toString())).name()));
            QObject::connect(btn, &QPushButton::pressed, [=]() { selectColor(btn->text(), btn); });
            bl->addWidget(btn, 1);

            widget->setLayout(bl);
            addWidgetContent(widget);
            continue;
        }

        if(values->value(key).mode == DialogValueMode::Base64Image)
        {
            auto widget = new QWidget();
            auto bl = new QVBoxLayout();
            bl->setMargin(0);
            bl->setSpacing(1);
            auto label = new QLabel(widget);
            label->setText(QString("<b>%1</b>").arg(text));
            label->setWordWrap(true);
            bl->addWidget(label, 0);

            QPixmap pixmap = getPixmapFromBase64(v.toString(), nullptr, config->ButtonSize());

            auto realw = pixmap.width();
            auto realh = pixmap.height();
            if(realw > 0 && realh)
            {
                auto limg = new QLabel();
                int border = 5;
                QPixmap p(pixmap);
                limg->setBackgroundRole(QPalette::Base);
                limg->setStyleSheet(IMG_STYLE.arg(QString::number(border)));

                auto w = minv.toInt();
                auto h = maxv.toInt();
                if(w > 0 && h > 0)
                    p = p.scaled(realw * w, realh * h, Qt::KeepAspectRatio, Qt::FastTransformation);

                limg->setPixmap(p);
                limg->setFixedSize(p.width() + 2 * border, p.height() + 2 * border);
                bl->addWidget(limg, 0);
            }            
            auto tbimginfo = new QToolBar();
            tbimginfo->setMovable(false);
            tbimginfo->setIconSize(QSize(config->ButtonSize(), config->ButtonSize()));
            tbimginfo->setOrientation(Qt::Horizontal);

            auto actionSave = new QAction(QIcon(":/resources/img/save.svg"), tr("Save image"), widget);
            actionSave->setAutoRepeat(false);
            QObject::connect(actionSave, &QAction::triggered, [=](){ saveImage(pixmap); });
            tbimginfo->addAction(actionSave);

            auto limgsize = new QLabel(QString("Size: %1X%2 px").
                                       arg(QString::number(realw), QString::number(realh)), widget);
            tbimginfo->addWidget(limgsize);
            bl->addWidget(tbimginfo, 0);

            widget->setLayout(bl);
            addWidgetContent(widget);
            continue;
        }

        if(t == QVariant::String)
        {
            auto widget = new QWidget();
            auto bl = new QHBoxLayout();
            bl->setMargin(0);
            bl->setSpacing(1);
            auto label = new QLabel(widget);
            label->setWordWrap(true);
            label->setText(QString("<b>%1</b>").arg(text));
            bl->addWidget(label, 0);
            auto le = new QLineEdit(v.toString(), widget);
            le->setCursorPosition(0);
            le->setProperty("ValueName", key);
            QObject::connect(le, &QLineEdit::textEdited,
                             this, &DialogValuesList::slotStringValueChanged);
            bl->addWidget(le, 1);
            widget->setLayout(bl);

            if(key == m_FocusedKey) le->setFocus();
            addWidgetContent(widget);
            continue;
        }

        if(t == QVariant::Bool)
        {
            auto cbox = new QCheckBox(text);
            cbox->setChecked(v.toBool());
            cbox->setProperty("ValueName", key);
            QObject::connect(cbox, &QCheckBox::stateChanged,
                             this, &DialogValuesList::slotBoolValueChanged);
            addWidgetContent(cbox);
            if(key == m_FocusedKey) cbox->setFocus();
            continue;
        }

        if(t == QVariant::Int)
        {
            auto spinbox = new QSpinBox();
            spinbox->setPrefix(QString("%1: ").arg(text));
            spinbox->setRange(minv.toInt(),
                              maxv.toInt() == minv.toInt()
                                  ? std::numeric_limits<int>::max()
                                  : maxv.toInt());
            spinbox->setSingleStep(1);
            spinbox->setValue(v.toInt());
            spinbox->installEventFilter(this);
            spinbox->setProperty("ValueName", key);
            QObject::connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                             this, &DialogValuesList::slotIntValueChanged);
            addWidgetContent(spinbox);
            if(key == m_FocusedKey) spinbox->setFocus();
            continue;
        }
        if(t == QVariant::Double)
        {
            auto spinbox = new QDoubleSpinBox();
            spinbox->setPrefix(QString("%1: ").arg(text));
            spinbox->setRange(minv.toDouble(),
                              maxv.toDouble() - minv.toDouble() == 0.0
                                  ? std::numeric_limits<double>::max()
                                  : maxv.toDouble());
            spinbox->setDecimals(DOUBLE_SPINBOX_DECIMALS);
            spinbox->setSingleStep(0.1);// TODO: QDoubleSpinBox->setSingleStep
            spinbox->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
            spinbox->setValue(v.toDouble());
            spinbox->installEventFilter(this);
            spinbox->setProperty("ValueName", key);
            QObject::connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                             this, &DialogValuesList::slotDoubleValueChanged);
            addWidgetContent(spinbox);
            if(key == m_FocusedKey) spinbox->setFocus();
            continue;
        }
        if(t == QVariant::StringList)
        {
            auto widget = new QWidget();
            auto label = new QLabel();
            label->setText(QString("<b>%1</b>").arg(text));
            label->setWordWrap(true);
            auto bl = new QHBoxLayout();
            bl->setMargin(0);
            bl->setSpacing(2);
            bl->addWidget(label, 0);
            if(values->value(key).mode == DialogValueMode::Default)
            { 
                auto le = new QLineEdit(v.toStringList().join(','), widget);
                le->setCursorPosition(0);
                le->setProperty("ValueName", key);
                le->setToolTip("Enter");
                // значение присваивается по нажатию Enter
                QObject::connect(le, &QLineEdit::returnPressed,
                                 this, &DialogValuesList::slotStringListValueChanged);
                bl->addWidget(le, 1);
                if(key == m_FocusedKey) le->setFocus();
                widget->setLayout(bl);
            }
            else if(values->value(key).mode == DialogValueMode::OneFromList)
            {
                auto cb = new QComboBox(widget);
                cb->installEventFilter(this);
                auto list = maxv.toStringList();
                if(!list.isEmpty())
                {
                    list.sort(Qt::CaseInsensitive);
                    cb->addItems(list);
                    cb->setProperty("ValueName", key);
                    auto index = list.indexOf(v.toString());
                    if(index != -1) cb->setCurrentIndex(index);
                    QObject::connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                     this, &DialogValuesList::slotOneOfStringListValueChanged);
                }
                else cb->setDisabled(true);
                bl->addWidget(cb, 1);
                if(key == m_FocusedKey) cb->setFocus();
                widget->setLayout(bl);
            }
            else if(values->value(key).mode == DialogValueMode::ManyFromList)
            {
                auto lv = new QListView(widget);
                lv->setSelectionMode(QAbstractItemView::NoSelection);
                lv->setEditTriggers(QAbstractItemView::NoEditTriggers);
                lv->setSelectionRectVisible(false);                
                QFontMetrics fm(lv->font());
                lv->setFixedHeight(fm.height() * 4);
                auto model = new QStandardItemModel(lv);
                model->setProperty("ValueName", key);
                auto list = maxv.toStringList();
                if(!list.isEmpty())
                {
                    list.sort(Qt::CaseInsensitive);
                    for(QString s: list)
                    {
                        auto i = new QStandardItem(s);
                        i->setCheckable(true);
                        i->setCheckState(v.toStringList().contains(s) ? Qt::Checked : Qt::Unchecked);
                        model->appendRow(i);
                    }

                    lv->setModel(model);
                    connect(model, &QStandardItemModel::itemChanged,
                            this, &DialogValuesList::slotManyOfStringListValueChanged);
                }
                else lv->setDisabled(true);
                bl->addWidget(lv, 1);
                if(key == m_FocusedKey) lv->setFocus();
                widget->setLayout(bl);
            }
            addWidgetContent(widget);
            continue;
        }

        qCritical() << __func__ << ": Value" << key << "Unsupported type" << t;
    }
}

bool DialogValuesList::eventFilter(QObject* object, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Wheel:
    { return true; }
    case QEvent::WindowStateChange:
    {
        if(!isMinimized()) return false;

        // если всё же свернули
        setWindowState(static_cast<QWindowStateChangeEvent *>(event)->oldState());
        return true;

    }
    case QEvent::Close:
    {
        if(object != this || isMinimized() || isMaximized()) return false;

        // сохранение размеров окна

        return true;
    }

    default: { return false; }
    }
}

void DialogValuesList::saveImage(QPixmap pixmap)
{
    auto filename = QFileDialog::getSaveFileName(this, "Save image", config->LastDir(), "PNG files (*.png)");

    if(filename.isNull()) return;
    config->setLastDir(QFileInfo(filename).dir().path());

    if(!filename.endsWith(".png", Qt::CaseInsensitive)) filename.append(".png");
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly) || !pixmap.save(&file, "PNG"))
        qCritical() << __func__ << ": Error at file saving:" << filename;
}

void DialogValuesList::setMapValue(const QString &key, const QVariant &value)
{
    DialogValue dv = m_Values->value(key);
    dv.value = value;
    m_Values->insert(key, dv);
}

void DialogValuesList::slotStringValueChanged(const QString &value)
{
    auto ledit = qobject_cast<QLineEdit*>(sender());
    if(!ledit) { qCritical() << __func__ << ": Signal sender not found."; return; }
    auto key = ledit->property("ValueName").toString();
    setMapValue(key, value);
}

void DialogValuesList::slotStringListValueChanged()
{
    auto ledit = qobject_cast<QLineEdit*>(sender());
    if(!ledit) { qCritical() << __func__ << ": Signal sender not found."; return; }
    auto key = ledit->property("ValueName").toString();
    setMapValue(key, ledit->text().split(',', Qt::SkipEmptyParts).replaceInStrings(QRegExp(RE_FIRST_LAST_SPACES), ""));
}

void DialogValuesList::slotOneOfStringListValueChanged()
{
    auto cb = qobject_cast<QComboBox*>(sender());
    if(!cb) { qCritical() << __func__ << ": Signal sender not found."; return; }
    auto key = cb->property("ValueName").toString();
    setMapValue(key, cb->currentText());
}

void DialogValuesList::slotManyOfStringListValueChanged()
{
    auto sim = qobject_cast<QStandardItemModel*>(sender());
    if(!sim) { qCritical() << __func__ << ": Signal sender not found."; return; }
    auto key = sim->property("ValueName").toString();
    auto sl = QStringList();
    for(int i = 0; i < sim->rowCount(); i++)
        if(sim->item(i)->checkState() == Qt::Checked) sl.append(sim->item(i)->text());
    setMapValue(key, sl);
}

void DialogValuesList::slotBoolValueChanged(bool value)
{
    auto cbox = qobject_cast<QCheckBox*>(sender());
    if(!cbox) { qCritical() << __func__ << ": Signal sender not found."; return; }
    auto key = cbox->property("ValueName").toString();
    setMapValue(key, value);
}

void DialogValuesList::slotIntValueChanged(int value)
{
    auto spinbox = qobject_cast<QSpinBox*>(sender());
    if(!spinbox) { qCritical() << __func__ << ": Signal sender not found."; return; }
    auto key = spinbox->property("ValueName").toString();
    setMapValue(key, value);
}

void DialogValuesList::slotDoubleValueChanged(double value)
{
    auto spinbox = qobject_cast<QDoubleSpinBox*>(sender());
    if(!spinbox) { qCritical() << __func__ << ": Signal sender not found."; return; }
    auto key = spinbox->property("ValueName").toString();
    setMapValue(key, value);
}

void DialogValuesList::selectColor(const QString &value, QPushButton* btn)
{
    const auto color = QColorDialog::getColor(QColor(value), this, tr("Select color"));
    if(color.isValid())
    {
        auto colortxt = color.name().toUpper();
        btn->setText(colortxt);
        btn->setStyleSheet(BTN_COLOR_STYLE.arg(colortxt, GetContrastColor(QColor(colortxt)).name()));
        auto key = btn->property("ValueName").toString();
        setMapValue(key, colortxt);
    }
}
