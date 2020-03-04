#ifndef HELPERGRAPHICS_H
#define HELPERGRAPHICS_H

#include <QSize>
#include <QString>

/*!
 * \brief getBase64Image - получить изображение в формате Base64
 * \param path - путь до ресурса
 * \param size - размер изображения
 * \param html - добавить префикс для использования в html документе
 */
QString getBase64Image(const QString& path, QSize size, bool html = false);

#endif // HELPERGRAPHICS_H
