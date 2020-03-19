#ifndef DIALOGVALUESLIST_H
#define DIALOGVALUESLIST_H

#include <QDialog>
#include <QVariant>

const QString RE_FIRST_LAST_SPACES = "^\\s+|\\s+$"; // регексп на наличие стартовых и финальных пробелов
const QString RE_NUM_MARK = "(^.*)(#_)"; // регексп удаления символов до строки '#_' включительно
const QString IMG_STYLE = "border: 1px solid darkgray; border-radius: 9px; padding: %1px;";
const QSize WINDOW_SIZE(400, 300);

class QGridLayout;
class QToolBar;

/*!
 * \brief DialogValueMode - режим отображения значений
 */
enum DialogValueMode
{
    Default = 0,  // поумолчанию, для StringList - перечисление через запятую
    Disabled,     // просто текстовое отображение значения
    OneFromList,  // для StringList - один из списка
    ManyFromList, // для StringList - несколько из списка
    Base64Image   // для String - изображение в формате Base64, только отображение; minValue = width, maxValue = height
};

/*!
 * \brief DialogValue - передаваемая для редактирования структура;
 * type: тип результата;
 * value: результат и значение поумолчанию
 * minValue: минимальное значение, если доступно;
 * maxValue: максимальнеа значение, для StringList вида OneFromList, ManyFromList - все варианты выбора;
 * mode: способ отображения значений
 */
struct DialogValue
{
    QVariant::Type type = QVariant::Invalid;
    QVariant value = QVariant();
    QVariant minValue = QVariant();
    QVariant maxValue = QVariant();
    DialogValueMode mode = DialogValueMode::Default;
};

/*!
 * \brief DialogValuesList - универсальный диалог
 */
class DialogValuesList : public QDialog
{
public:
    /*!
     * \brief DialogValuesList
     * \param icon - иконка окна;
     * \param caption - заголовок окна;
     * \param values - значения для отображения, для сортировки использовать нумерацию вида '00#_' (RE_NUM_MARK);
     * \param focusedKey - контрол для фокуса по ключу values
     * \param dialogMode - вариант диалога: true - ДА/ОК, false - OK
     */
    DialogValuesList(QWidget* parent,
                     const QString &icon,
                     const QString &caption,
                     QMap<QString, DialogValue>* values,
                     const QString &focusedKey = "",
                     bool dialogMode = true);
    void addToolbarButton(QAction* action);

protected:
    void addWidgetContent(QWidget* widget);
    bool eventFilter(QObject *, QEvent *event);
    void saveImage(QPixmap pixmap);

private:    
    QGridLayout* glContent;
    QToolBar* toolBar;
    QMap<QString, DialogValue>* m_Values;
    QString m_FocusedKey;
    bool m_DialogMode;

public Q_SLOTS:
    void slotLoadContent(QMap<QString, DialogValue> *values);

private Q_SLOTS:    
    void slotStringValueChanged(const QString& value);
    void slotStringListValueChanged();
    void slotOneOfStringListValueChanged(const QString& value);
    void slotManyOfStringListValueChanged();
    void slotBoolValueChanged(bool value);
    void slotIntValueChanged(int value);
};

#endif // DIALOGVALUESLIST_H
