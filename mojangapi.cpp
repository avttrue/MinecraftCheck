#include "mojangapi.h"
#include "properties.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QPixmap>

MojangApi::MojangApi(QObject *parent) :
    QObject(parent),
    m_NetworkReply(nullptr)
{
    m_NetworkManager = new QNetworkAccessManager(this);
    m_Time = QDateTime::currentMSecsSinceEpoch();

    QObject::connect(this, &QObject::destroyed, [=]()
                     {
                         abort();
                         qInfo() << objectName() << "destroyed";
                         qInfo() << "Lifetime MojangApi" << QDateTime::currentMSecsSinceEpoch() - m_Time << "ms";
                     });
}

void MojangApi::sendQuery(const QString &query)
{
    Q_EMIT signalStatus("Receiving data...");
    Q_EMIT signalMessage(QString("**********\n[Q]\t%1").arg(query));

    auto func_nmfinished = [=](QNetworkReply* reply)
    {
        QObject::disconnect(m_NetworkManager, &QNetworkAccessManager::finished, nullptr, nullptr);
        QObject::disconnect(m_NetworkReply, &QNetworkReply::downloadProgress, nullptr, nullptr);
        auto time = QDateTime::currentMSecsSinceEpoch();
        Q_EMIT signalMessage(QString("[i]\tQuery was completed in %1 ms").arg(QString::number(time - m_Time)));
        readQueryReply(reply);
    };
    QObject::connect(m_NetworkManager, &QNetworkAccessManager::finished, func_nmfinished);

    m_NetworkReply = m_NetworkManager->get(QNetworkRequest(QUrl(query)));
    QObject::connect(m_NetworkReply, &QNetworkReply::downloadProgress, this, &MojangApi::slotProgress, Qt::DirectConnection);
}

void MojangApi::downloadData(const QString &query)
{
    Q_EMIT signalStatus("Downloading...");
    Q_EMIT signalMessage(QString("**********\n[Q]\t%1").arg(query));

    auto func_nmfinished = [=](QNetworkReply* data)
    {
        QObject::disconnect(m_NetworkManager, &QNetworkAccessManager::finished, nullptr, nullptr);
        QObject::disconnect(m_NetworkReply, &QNetworkReply::downloadProgress, nullptr, nullptr);
        auto time = QDateTime::currentMSecsSinceEpoch();
        Q_EMIT signalMessage(QString("[i]\tDownloading was completed in %1 ms").arg(QString::number(time - m_Time)));
        readData(data);
    };
    QObject::connect(m_NetworkManager, &QNetworkAccessManager::finished, func_nmfinished);

    m_NetworkReply = m_NetworkManager->get(QNetworkRequest(QUrl(query)));
    QObject::connect(m_NetworkReply, &QNetworkReply::downloadProgress, this, &MojangApi::slotProgress, Qt::DirectConnection);
}

void MojangApi::abort(bool forced)
{
    if(!m_NetworkReply) return;

    QObject::disconnect(m_NetworkManager, &QNetworkAccessManager::finished, nullptr, nullptr);
    QObject::disconnect(m_NetworkReply, &QNetworkReply::downloadProgress, nullptr, nullptr);
    m_NetworkReply->abort();
    m_NetworkReply->close();

    if(forced) // вынужденное завершение
    {
        Q_EMIT signalMessage(QString("[X]\tQuery aborted\n^^^^^^^^^^"));
        Q_EMIT signalStatus("ABORTED");
        Q_EMIT signalError();
    }
}

void MojangApi::readQueryReply(QNetworkReply *reply)
{
    if(reply->error())
    {
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalMessage(QString("[!]\tERROR. %1").arg(reply->errorString()));
        Q_EMIT signalError();
        return;
    }

    QByteArray answer = reply->readLine();
    Q_EMIT signalMessage(QString("[i]\tanswer size %1 bytes").arg(answer.size()));
    QString string = QString::fromStdString(answer.toStdString());

    Q_EMIT signalMessage(QString("[A]\t%1\n^^^^^^^^^^").arg(string));
    Q_EMIT signalStatus("Interpretation...");

    if(string.isEmpty())
    {
        Q_EMIT signalStatus("ERROR, no answer");
        Q_EMIT signalError();
    }
    else
        interpretateReply(string);
}

void MojangApi::readData(QNetworkReply *data)
{
    if(data->error())
    {
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalMessage(QString("[!]\tERROR. %1").arg(data->errorString()));
        Q_EMIT signalError();
        return;
    }

    QByteArray barray = data->readAll();
    Q_EMIT signalMessage(QString("[i]\tdata size %1 bytes\n^^^^^^^^^^").arg(barray.size()));
    Q_EMIT signalStatus("Interpretation...");

    interpretateData(barray);
}

QJsonDocument MojangApi::getJsonDocument(const QString &string)
{
    QJsonParseError jerror;
    QJsonDocument jdoc;

    auto barray = string.toUtf8();
    jdoc = QJsonDocument::fromJson(barray, &jerror);
    if(jerror.error != QJsonParseError::NoError)
    {
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalMessage(QString("[!]\tERROR. Json parsing: %1").arg(jerror.errorString()));
        Q_EMIT signalError();
        return QJsonDocument();
    }
    return jdoc;
}

void MojangApi::slotProgress(qint64 received, qint64 total)
{
    Q_EMIT signalMessage(QString("[<]\treceived: %1/%2 bytes").arg(QString::number(received), QString::number(total)));
}

// ServerStatusReader

ServerStatusReader::ServerStatusReader(QObject *parent):
    MojangApi(parent)
{
    setObjectName("ServerStatusReader");
}

void ServerStatusReader::interpretateReply(const QString& reply)
{
    QJsonDocument jdoc = getJsonDocument(reply);
    if(jdoc.isEmpty()) return;

    auto vl = jdoc.toVariant().toList();
    for(auto v: vl)
    {
        if(!v.isValid())
        {
            Q_EMIT signalMessage("[!]\tWARNING. Json parsing: nonvalid key");
            continue;
        }
        auto map = v.toMap();
        if(map.isEmpty())
        {
            Q_EMIT signalMessage("[!]\tWARNING. Json parsing: empty content");
            continue;
        }
        for(auto key: map.keys())
        {
            m_Statuses.insert(key, map.value(key).toString());
        }
    }
    Q_EMIT signalStatus("Done");
    Q_EMIT signalSuccess();
    Q_EMIT signalServers(m_Statuses);
}

PlayerProfileReader::PlayerProfileReader(QObject *parent):
    MojangApi(parent),
    m_Stage(0)
{
    setObjectName("PlayerProfileReader");
    m_Profile.DateTime = QDateTime::currentMSecsSinceEpoch();
}

void PlayerProfileReader::setProfileId(const QString &id)
{
    m_Stage = 1;
    m_Profile.Id = id;
}

void PlayerProfileReader::interpretateReply(const QString &reply)
{
    if(m_Stage == 0) //id
    {
        if(!interpretate_Id(getJsonDocument(reply))) return;

        m_Stage++;
        sendQuery(config->QueryPersonUuid().arg(m_Profile.Id));
    }
    else if(m_Stage == 1) // name history
    {
        if(!interpretate_History(getJsonDocument(reply))) return;

        m_Stage++;
        sendQuery(config->QueryProfileUuid().arg(m_Profile.Id));
    }
    else if(m_Stage == 2) // profile
    {
        if(!interpretate_Profile(getJsonDocument(reply))) return;

        if(!interpretate_SkinCape(getJsonDocument(m_SkinCapeValue))) return;

        m_Stage++;

        downloadData(m_Profile.SkinUrl); // m_SkinUrl всегда есть
    }
    else // сюда попадать не должны
    {
        Q_EMIT signalStatus("FATAL ERROR");
        Q_EMIT signalError();
        qCritical() << __func__ << ": wrong stage";
        abort();
    }
}

void PlayerProfileReader::interpretateData(const QByteArray &data)
{
    if(m_Stage == 3) // skin
    {
        if(data.isNull() || data.isEmpty())
            Q_EMIT signalMessage("[!]\tWARNING. Skin image data: empty content");
        else
            m_Profile.Skin = data.toBase64();

        QImage image;
        if(image.loadFromData(QByteArray::fromBase64(m_Profile.Skin.toLatin1()),
                               config->MojangImageFormat().toLocal8Bit().data()))
            Q_EMIT signalMessage(QString("[i]\tSkin image size: %1 X %2").
                                 arg(QString::number(image.width()), QString::number(image.height())));
        else
        {
            Q_EMIT signalMessage(QString("[!]\tWARNING. Skin image data: non interpreted as '%1' format").
                                 arg(config->MojangImageFormat()));
            m_Profile.Skin = "";
        }

        m_Stage++;

        if(m_Profile.CapeUrl.isEmpty()) interpretation_Final();
        else downloadData(m_Profile.CapeUrl);
    }
    else if(m_Stage == 4) // cape
    {
        if(data.isNull() || data.isEmpty())
            Q_EMIT signalMessage("[!]\tWARNING. Cape image data: empty content");
        else
            m_Profile.Cape = data.toBase64();

        QImage image;
        if(image.loadFromData(QByteArray::fromBase64(m_Profile.Cape.toLatin1()),
                               config->MojangImageFormat().toLocal8Bit().data()))
            Q_EMIT signalMessage(QString("[i]\tCape image size: %1 X %2").
                                 arg(QString::number(image.width()), QString::number(image.height())));
        else
        {
            Q_EMIT signalMessage(QString("[!]\tWARNING. Cape image data: non interpreted as %1 format").
                                 arg(config->MojangImageFormat()));
            m_Profile.Cape = "";
        }

        interpretation_Final();
    }
    else // сюда попадать не должны
    {
        Q_EMIT signalStatus("FATAL ERROR");
        Q_EMIT signalError();
        qCritical() << __func__ << ": wrong stage";
        abort();
    }
}

bool PlayerProfileReader::interpretate_Id(const QJsonDocument &document)
{
    if(document.isEmpty()) return false;
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto map = document.toVariant().toMap();
    if(map.isEmpty())
    {
        signalMessage("[!]\tWARNING. Json parsing: empty content");
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalError();
        return false;
    }

    if(!map.contains("id")) //NOTE: 'id' key
    {
        Q_EMIT signalMessage("[!]\tERROR. Json parsing: 'id' key is absent");
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalError();
        return false;
    }

    m_Profile.Id = map.value("id").toString();

    Q_EMIT signalMessage(QString("[i]\tPlayer id: %1").arg(m_Profile.Id));

    qDebug() << __func__ << "completed in" << QDateTime::currentMSecsSinceEpoch() - time << "ms";

    return true;
}

bool PlayerProfileReader::interpretate_History(const QJsonDocument &document)
{
    if(document.isEmpty()) return false;
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto vl = document.toVariant().toList();
    for(auto v: vl)
    {
        if(!v.isValid())
        {
            Q_EMIT signalMessage("[!]\tERROR. Json parsing: nonvalid key");
            Q_EMIT signalStatus("ERROR");
            Q_EMIT signalError();
            return false;
        }
        auto map = v.toMap();
        if(map.isEmpty())
        {
            Q_EMIT signalMessage("[!]\tERROR. Json parsing: empty content");
            Q_EMIT signalStatus("ERROR");
            Q_EMIT signalError();
            return false;
        }
        if(!map.contains("changedToAt") && map.contains("name")) // NOTE: keys 'name', 'changedToAt'
        {
            m_Profile.FirstName = map.value("name").toString();
            Q_EMIT signalMessage(QString("[i]\tPlayer first name: %1").arg(m_Profile.FirstName));
        }
        else if(map.contains("changedToAt") && map.contains("name"))
        {
            m_Profile.NameHistory.insert(map.value("changedToAt").toLongLong(), map.value("name").toString());
        }
        else
        {
            Q_EMIT signalMessage(QString("[!]\tWARNING: Player name history non interpretable"));
        }
    }
    Q_EMIT signalMessage(QString("[i]\tPlayer name history: %1 items").
                         arg(QString::number(m_Profile.NameHistory.count())));

    qDebug() << __func__ << "completed in" << QDateTime::currentMSecsSinceEpoch() - time << "ms";

    return true;
}

bool PlayerProfileReader::interpretate_Profile(const QJsonDocument &document)
{
    if(document.isEmpty()) return false;
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto map = document.toVariant().toMap();

    m_Profile.Legacy = map.contains("legacy") ? map.value("legacy").toBool() : false;
    m_Profile.Demo = map.contains("demo") ? map.value("demo").toBool() : false;

    // CurrentName читаем здесь, хотя мобли бы в interpretate_Id. На случай изначального поиска не по нику, а по id
    if(!map.contains("name")) //NOTE: 'name' key
    {
        Q_EMIT signalMessage("[!]\tERROR. Json parsing: 'name' key is absent");
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalError();
        return false;
    }
    m_Profile.CurrentName = map.value("name").toString();
    Q_EMIT signalMessage(QString("[i]\tPlayer current name: %1").arg(m_Profile.CurrentName));

    if(!map.contains("properties")) //NOTE: 'properties' key
    {
        Q_EMIT signalMessage("[!]\tERROR. Json parsing: 'properties' key is absent");
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalError();
        return false;
    }

    auto list = map.value("properties").toList();
    QString textures;
    for(int i = 0; i < list.count(); i++)
    {
        auto submap = list.at(i).toMap();
        if(submap.contains("name") && submap.value("name") == "textures") //NOTE: key 'name' = 'textures'
        {
            if(!submap.contains("value")) //NOTE: key 'textures.value'
            {
                Q_EMIT signalMessage("[!]\tERROR. Json parsing: 'properties.textures.value' key is absent");
                Q_EMIT signalStatus("ERROR");
                Q_EMIT signalError();
                return false;
            }
            textures = submap.value("value").toString();
            break;
        }

        if(i == list.count() - 1)
        {
            Q_EMIT signalMessage("[!]\tERROR. Json parsing: 'properties.textures' map is absent");
            Q_EMIT signalStatus("ERROR");
            Q_EMIT signalError();
            return false;
        }
    }
    m_SkinCapeValue = QString(QByteArray::fromBase64(textures.toLatin1()).data());
    Q_EMIT signalMessage(QString("[i]\tProperties.Textures value: %1").arg(m_SkinCapeValue));

    qDebug() << __func__ << "completed in" << QDateTime::currentMSecsSinceEpoch() - time << "ms";

    return true;
}

bool PlayerProfileReader::interpretate_SkinCape(const QJsonDocument& document)
{
    if(document.isEmpty()) return false;
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto map = document.toVariant().toMap();

    if(!map.contains("textures")) //NOTE: 'textures' key
    {
        Q_EMIT signalMessage("[!]\tERROR. Json parsing: Properties.Textures: 'textures' key is absent");
        Q_EMIT signalStatus("ERROR");
        Q_EMIT signalError();
        return false;
    }
    map = map.value("textures").toMap();

    if(map.isEmpty()) // может быть пустым
    {
        Q_EMIT signalMessage("[!]\tWARNING. Json parsing: Properties.Textures: 'textures' value not present");
        m_Profile.SkinModel = config->MojangModel2();
    }
    else
    {
        if(map.contains("SKIN")) //NOTE: 'SKIN' key
        {
            auto submap = map.value("SKIN").toMap();
            m_Profile.SkinUrl = submap.value("url").toString(); //NOTE: 'SKIN.url' key

            if(submap.contains("metadata")) //NOTE: 'SKIN.metadata' key
            {
                submap = submap.value("metadata").toMap();
                if(submap.value("model").toString() == "slim") m_Profile.SkinModel = config->MojangModel1(); //NOTE: 'SKIN.metadata.model' key
                else m_Profile.SkinModel = submap.value("model").toString(); // unknown model
            }
            else m_Profile.SkinModel = config->MojangModel2();
        }
        else
            Q_EMIT signalMessage("[i]\tJson parsing: Properties.Textures.Textures 'SKIN' value not present");

        if(map.contains("CAPE")) //NOTE: 'CAPE' key
        {
            auto submap = map.value("CAPE").toMap();
            m_Profile.CapeUrl = submap.value("url").toString(); //NOTE: 'CAPE.url' key
        }
    }

    if(m_Profile.SkinUrl.isEmpty())
        m_Profile.SkinUrl = config->MojangDefaultSkin().arg(m_Profile.SkinModel.toLower());

    Q_EMIT signalMessage(QString("[i]\tSKIN url: %1").arg(m_Profile.SkinUrl));
    Q_EMIT signalMessage(QString("[i]\tCAPE url: %1").arg(m_Profile.CapeUrl));

    qDebug() << __func__ << "completed in" << QDateTime::currentMSecsSinceEpoch() - time << "ms";

    return true;
}

void PlayerProfileReader::interpretation_Final()
{
    Q_EMIT signalProfile(m_Profile);
    Q_EMIT signalStatus("Done");
    Q_EMIT signalSuccess();
}


