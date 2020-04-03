#ifndef HELPERGRAPHICS_H
#define HELPERGRAPHICS_H

#include <QPixmap>
#include <QSize>
#include <QString>

/*!
 * \brief getBase64Image - получить изображение в формате Base64
 * \param path - путь до ресурса
 * \param size - размер изображения
 * \param html - добавить префикс для использования в html документе
 */
QString getBase64Image(const QString& path, QSize size, bool html = false);

QString getBase64Image(QPixmap pixmap, bool html = false);

/*!
 * \brief getPixmapFromBase64 - получить Pixmap из Base64
 * \param img - строка Base64
 * \param ok - наличие ошибок
 * \param defsize - размер картинки-заглушки в случае ошибки
 */
QPixmap getPixmapFromBase64(const QString& img, bool *ok = nullptr, int defsize = 64);

/*!
 * \brief getProfilePortrait - получить портрет профиля (для Mojang API)
 * \param img - строка Base64
 */
QPixmap getProfilePortrait(const QString& img, int size = 0);

#endif // HELPERGRAPHICS_H
