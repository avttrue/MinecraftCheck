#include "helper.h"

#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QtMath>

QString getTextFromRes(const QString& path)
{
    QFile file(path);
    if(file.open(QFile::ReadOnly))
    {
        QString text(file.readAll());
        return text;
    }
    qCritical() << "Resurce not found:" << path;
    return "";
}

bool textToFile(const QString& text, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "File not open for writing:" << path;
        return false;
    }

    QTextStream stream(&file);
    stream << text;
    file.close();

    if (stream.status() != QTextStream::Ok)
    {
        qCritical() << "Error at file saving:" << path;
        return false;
    }
    return true;
}

QString longTimeToString(long datetime, const QString& format)
{
    QDateTime dt;
    dt.setMSecsSinceEpoch(datetime);
    return dt.toString(format);
}

QString humanReadableByteCount(long long bytes, bool si)
{
    int unit = si ? 1000 : 1024;
    if (bytes < unit)
        return QString("%1 B").arg(QString::number(bytes));

    int exp = static_cast<int>(qLn(bytes) / qLn(unit));
    QString pre = si ? "kMGTPE" : "KMGTPE";

    return QString("%1 %2%3B").
        arg(bytes / qPow(unit, exp), 0, 'f', 1, '0').
        arg(pre[exp - 1]).
        arg(si ? "" : "i");
}

QString fileToText(const QString& path, bool* ok)
{
    QString text;
    QFile file(path);

    if (!file.exists())
    {
        qCritical() << "File not found:" << path;
        if(ok) *ok = false;
        return "";
    }

    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        qCritical() << "File not open for reading:" << path;
        if(ok) *ok = false;
        return "";
    }

    QTextStream stream(&file);
    text = stream.readAll();
    file.close();
    if(ok) *ok = true;
    return text;
}
