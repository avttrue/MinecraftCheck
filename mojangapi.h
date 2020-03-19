#ifndef MOJANGAPI_H
#define MOJANGAPI_H

#include <QMap>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

struct MojangApiProfile
{
    long DateTime = 0;
    QString CurrentName;
    QString FirstName;
    QString Id;
    QMap<qlonglong, QString> NameHistory;
    QString SkinUrl;
    QString Skin;
    QString SkinModel;
    QString CapeUrl;
    QString Cape;
    int Legacy = 0;
    int Demo = 0;
    QString Comment;
};

class MojangApi : public QObject
{
    Q_OBJECT
public:
    explicit MojangApi(QObject *parent);
    void sendQuery(const QString& query);
    void downloadData(const QString& query);
    void abort(bool forced = false);

protected:
    void readQueryReply(QNetworkReply* reply);
    void readData(QNetworkReply* reply);
    QJsonDocument getJsonDocument(const QString& string);
    virtual void interpretateReply(const QString&){}
    virtual void interpretateData(const QByteArray&){}

private:
    long m_Time;
    QNetworkAccessManager* m_NetworkManager;
    QNetworkReply* m_NetworkReply;

private Q_SLOTS:
    void slotProgress(qint64 received, qint64 total);

Q_SIGNALS:
    void signalSuccess();
    void signalError();
    void signalStatus(const QString& string);
    void signalMessage(const QString& string);
};

class ServerStatusReader : public MojangApi
{
    Q_OBJECT
public:
    explicit ServerStatusReader(QObject *parent = nullptr);

protected:
    void interpretateReply(const QString& reply);

private:
    QMap<QString, QString> m_Statuses;

Q_SIGNALS:
    void signalServers(QMap<QString, QString> servers);
};

class PlayerProfileReader : public MojangApi
{
    Q_OBJECT
public:
    explicit PlayerProfileReader(QObject *parent);
    void setProfileId(const QString& id); // если получаем профиль не по нику, а по id

protected:
    void interpretateReply(const QString& reply);
    void interpretateData(const QByteArray& data);
    bool interpretate_Id(const QJsonDocument& document);      // получение id
    bool interpretate_History(const QJsonDocument& document); // получение истории ников
    bool interpretate_Profile(const QJsonDocument& document); // получение профиля
    bool interpretate_SkinCape(const QJsonDocument& document);// получение Skin, Cape
    void interpretation_Final();                              // завершение интерпретации

private:
    QString m_SkinCapeValue;
    MojangApiProfile m_Profile;
    int m_Stage;                            // стадии интерпретации ответов сервера

Q_SIGNALS:
    void signalProfile(const MojangApiProfile& profile);
};

#endif // MOJANGAPI_H
