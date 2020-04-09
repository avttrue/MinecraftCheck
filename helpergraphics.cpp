#include "helpergraphics.h"

#include <QBuffer>
#include <QIcon>
#include <QDebug>
#include <QPainter>

QString getBase64Image(const QString& path, QSize size, bool html)
{
    QIcon icon(path);
    return getBase64Image(icon.pixmap(size), html);
}

QString getBase64Image(QPixmap pixmap, bool html)
{
    QImage image(pixmap.toImage());
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    image.save(&buffer, "PNG");
    QString stringBase64 = html ? "data:image/png;base64," : "";
    stringBase64.append(QString::fromLatin1(byteArray.toBase64().data()));
    return stringBase64;
}

QPixmap getPixmapFromBase64(const QString& img, bool* ok, int defsize)
{
    QPixmap pixmap;
    bool result = true;
    if(!pixmap.loadFromData(QByteArray::fromBase64(img.toLatin1())))
    {
        pixmap = QIcon(":/resources/img/error.svg").pixmap(defsize, defsize);
        result = false;
        qCritical() << __func__ << ": Error loading from Base64";
    }
    if(ok) *ok = result;
    return pixmap;
}

QPixmap getProfilePortrait(const QString &img, int size)
{
    bool ok = false;
    QPixmap pixmap = getPixmapFromBase64(img, &ok);
    if(!ok) return pixmap;

    QImage image = pixmap.toImage();
    auto alfacolor = image.pixelColor(32, 0); // назначение маски прзрачности пока не готово

    QImage face(8, 8, QImage::Format_ARGB32_Premultiplied);
    face.fill(alfacolor);

    QPainter painter(&face);
    painter.drawImage(QPoint(0, 0), image.copy(8, 8, 8, 8));
    painter.drawImage(QPoint(0, 0), image.copy(40, 8, 8, 8));
    painter.end();

    pixmap = QPixmap::fromImage(face);
    if(size > 0)
        pixmap = pixmap.scaled(size, size, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    return pixmap;
}
