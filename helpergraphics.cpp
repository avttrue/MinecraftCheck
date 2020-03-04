#include "helpergraphics.h"

#include <QBuffer>
#include <QIcon>

QString getBase64Image(const QString& path, QSize size, bool html)
{
    QIcon icon(path);
    QImage image(icon.pixmap(size).toImage());
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    image.save(&buffer, "PNG");
    QString iconBase64 = html ? "data:image/png;base64," : "";
    iconBase64.append(QString::fromLatin1(byteArray.toBase64().data()));
    return iconBase64;
}
