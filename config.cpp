#include "config.h"
#include "properties.h"

#include <QDebug>
#include <QDir>
#include <QTextCodec>

Config::Config(const QString& in_AppDirectory):
    m_Settings(nullptr)
{
    m_PathAppDir = in_AppDirectory;
    m_PathAppConfig = m_PathAppDir + QDir::separator() + APP_CFG;
    m_PathLocalDB = m_PathAppDir + QDir::separator() + LOCAL_DB;

    qInfo() << "AppConfig:" << m_PathAppConfig;
    qInfo() << "LocalDB:" << m_PathLocalDB;

    m_Settings = new QSettings(m_PathAppConfig, QSettings::IniFormat);
    m_Settings->setIniCodec(QTextCodec::codecForName(TEXT_CODEC.toLatin1()));

    load();
}

void Config::load()
{
    if(!m_Settings->contains("MainWindow/Height"))
        m_Settings->setValue("MainWindow/Height", WINDOW_HEIGHT);

    if(!m_Settings->contains("MainWindow/Width"))
        m_Settings->setValue("MainWindow/Width", WINDOW_WIDTH);

    if(!m_Settings->contains("MainWindow/SplashTime"))
        m_Settings->setValue("MainWindow/SplashTime", SPLASH_TIME);
    m_SplashTime = m_Settings->value("MainWindow/SplashTime").toInt();

    if(!m_Settings->contains("MainWindow/SplashSize"))
        m_Settings->setValue("MainWindow/SplashSize", SPLASH_SIZE);
    m_SplashSize = m_Settings->value("MainWindow/SplashSize").toInt();

    if(!m_Settings->contains("MainWindow/ButtonSize"))
        m_Settings->setValue("MainWindow/ButtonSize", BUTTON_SIZE);
    m_ButtonSize = m_Settings->value("MainWindow/ButtonSize").toInt();

    if(!m_Settings->contains("MainWindow/FontNameEvents"))
        m_Settings->setValue("MainWindow/FontNameEvents", FONT_NAME_EVENTS);
    m_FontNameEvents = m_Settings->value("MainWindow/FontNameEvents").toString();

    if(!m_Settings->contains("MainWindow/DateTimeFormat"))
        m_Settings->setValue("MainWindow/DateTimeFormat", DT_FORMAT);
    m_DateTimeFormat = m_Settings->value("MainWindow/DateTimeFormat").toString();

    if(!m_Settings->contains("ApiQueries/Servers"))
        m_Settings->setValue("ApiQueries/Servers", QUERY_SERVERS);
    m_QueryServers = m_Settings->value("ApiQueries/Servers").toString();

    if(!m_Settings->contains("ApiQueries/ProfileUuid"))
        m_Settings->setValue("ApiQueries/ProfileUuid", QUERY_PROFILE_UUID);
    m_QueryProfileUuid = m_Settings->value("ApiQueries/ProfileUuid").toString();

    if(!m_Settings->contains("ApiQueries/PersonName"))
        m_Settings->setValue("ApiQueries/PersonName", QUERY_PERSON_NAME);
    m_QueryPersonName = m_Settings->value("ApiQueries/PersonName").toString();

    if(!m_Settings->contains("ApiQueries/PersonUuid"))
        m_Settings->setValue("ApiQueries/PersonUuid", QUERY_PERSON_UUID);
    m_QueryPersonUuid = m_Settings->value("ApiQueries/PersonUuid").toString();

    if(!m_Settings->contains("Api/DefaultSkin"))
        m_Settings->setValue("Api/DefaultSkin", MOJANG_DEFAULT_SKIN);
    m_MojangDefaultSkin = m_Settings->value("Api/DefaultSkin").toString();

    if(!m_Settings->contains("Api/Model1"))
        m_Settings->setValue("Api/Model1", MOJANG_MODEL1);
    m_MojangModel1 = m_Settings->value("Api/Model1").toString();

    if(!m_Settings->contains("Api/Model2"))
        m_Settings->setValue("Api/Model2", MOJANG_MODEL2);
    m_MojangModel2 = m_Settings->value("Api/Model2").toString();

    if(!m_Settings->contains("Api/ImageFormat"))
        m_Settings->setValue("Api/ImageFormat", MOJANG_IMAGE_FORMAT);
    m_MojangImageFormat = m_Settings->value("Api/ImageFormat").toString();

    if(!m_Settings->contains("Report/LedSize"))
        m_Settings->setValue("Report/LedSize", REPORT_LED_SIZE);
    m_ReportLedSize = m_Settings->value("Report/LedSize").toInt();

    if(!m_Settings->contains("Report/ImgScale"))
        m_Settings->setValue("Report/ImgScale", REPORT_IMG_SCALE);
    m_ReportImgScale = m_Settings->value("Report/ImgScale").toInt();

    if(!m_Settings->contains("Report/AutoOpen"))
        m_Settings->setValue("Report/AutoOpen", REPORT_AUTOOPEN);
    m_ReportAutoOpen = m_Settings->value("Report/AutoOpen").toBool();

    if(!m_Settings->contains("Database/AutoVacuum"))
        m_Settings->setValue("Database/AutoVacuum", AUTOVACUUM);
    m_AutoVacuum = m_Settings->value("Database/AutoVacuum").toBool();

    if(!m_Settings->contains("Database/AutoCollectProfiles"))
        m_Settings->setValue("Database/AutoCollectProfiles", AUTOCOLLECT_PROFILES);
    m_AutoCollectProfiles = m_Settings->value("Database/AutoCollectProfiles").toBool();

    if(!m_Settings->contains("Database/AdvancedMode"))
        m_Settings->setValue("Database/AdvancedMode", ADVANCED_DB_MODE);
    m_AdvancedDBMode = m_Settings->value("Database/AdvancedMode").toBool();
}

void Config::setAdvancedDBMode(bool value)
{
    if(m_AdvancedDBMode == value) return;

    m_AdvancedDBMode = value;
    m_Settings->setValue("Database/AdvancedMode", m_AdvancedDBMode);
}

void Config::setAutoCollectProfiles(bool value)
{
    if(m_AutoCollectProfiles == value) return;
    
    m_AutoCollectProfiles = value;
    m_Settings->setValue("Database/AutoCollectProfiles", m_AutoCollectProfiles);
}

void Config::setAutoVacuum(bool value)
{
    if(m_AutoVacuum == value) return;
    
    m_AutoVacuum = value;
    m_Settings->setValue("Database/AutoVacuum", m_AutoVacuum);
}

void Config::setReportImgScale(int value)
{
    if(m_ReportImgScale == value) return;
    
    m_ReportImgScale = value;
    m_Settings->setValue("Report/ImgScale", m_ReportImgScale);
}

void Config::setReportAutoOpen(bool value)
{
    if(m_ReportAutoOpen == value) return;
    
    m_ReportAutoOpen = value;
    m_Settings->setValue("Report/AutoOpen", m_ReportAutoOpen);
}

void Config::setReportLedSize(int value)
{
    if(m_ReportLedSize == value) return;
    
    m_ReportLedSize = value;
    m_Settings->setValue("Report/LedSize", m_ReportLedSize);
}

void Config::setMojangImageFormat(const QString &value)
{
    if(m_MojangImageFormat == value) return;
    
    m_MojangImageFormat = value;
    m_Settings->setValue("Api/ImageFormat", m_MojangImageFormat);
}

void Config::setMojangModel2(const QString &value)
{
    if(m_MojangModel2 == value) return;

    m_MojangModel2 = value;
    m_Settings->setValue("Api/Model2", m_MojangModel2);
}

void Config::setMojangModel1(const QString &value)
{
    if(m_MojangModel1 == value) return;

    m_MojangModel1 = value;
    m_Settings->setValue("Api/Model1", m_MojangModel1);
}

void Config::setMojangDefaultSkin(const QString &value)
{
    if(m_MojangDefaultSkin == value) return;

    m_MojangDefaultSkin = value;
    m_Settings->setValue("Api/DefaultSkin", m_MojangDefaultSkin);
}

void Config::setDateTimeFormat(const QString &value)
{
    if(m_DateTimeFormat == value) return;
    
    m_DateTimeFormat = value;
    m_Settings->setValue("MainWindow/DateTimeFormat", m_DateTimeFormat);
}

void Config::setFontNameEvents(const QString &value)
{
    if(m_FontNameEvents == value) return;

    m_FontNameEvents = value;
    m_Settings->setValue("MainWindow/FontNameEvents", m_FontNameEvents);
}

void Config::setButtonSize(int value)
{
    if(m_ButtonSize == value) return;

    m_ButtonSize = value;
    m_Settings->setValue("MainWindow/ButtonSize", m_ButtonSize);
}

void Config::setQueryProfileUuid(const QString &value)
{
    if(m_QueryProfileUuid == value) return;

    m_QueryProfileUuid = value;
    m_Settings->setValue("ApiQueries/ProfileUuid", m_QueryProfileUuid);
}

void Config::setQueryPersonName(const QString &value)
{
   if(m_QueryPersonName == value) return;

    m_QueryPersonName = value;
    m_Settings->setValue("ApiQueries/PersonName", m_QueryPersonName);
}

void Config::setQueryPersonUuid(const QString &value)
{
    if(m_QueryPersonUuid == value) return;

    m_QueryPersonUuid = value;
    m_Settings->setValue("ApiQueries/PersonUuid", m_QueryPersonUuid);
}

void Config::setQueryServers(const QString &value)
{
    if(m_QueryServers == value) return;

    m_QueryServers = value;
    m_Settings->setValue("ApiQueries/Servers", m_QueryServers);
}

bool Config::AdvancedDBMode() const { return m_AdvancedDBMode; }
bool Config::AutoCollectProfiles() const { return m_AutoCollectProfiles; }
bool Config::AutoVacuum() const { return m_AutoVacuum; }
int Config::ReportImgScale() const { return m_ReportImgScale; }
bool Config::ReportAutoOpen() const { return m_ReportAutoOpen; }
int Config::ReportLedSize() const { return m_ReportLedSize; }
QString Config::MojangImageFormat() const { return m_MojangImageFormat; }
QString Config::MojangDefaultSkin() const { return m_MojangDefaultSkin; }
QString Config::MojangModel1() const { return m_MojangModel1; }
QString Config::MojangModel2() const { return m_MojangModel2; }
QString Config::DateTimeFormat() const { return m_DateTimeFormat; }
QString Config::QueryProfileUuid() const { return m_QueryProfileUuid; }
QString Config::QueryPersonName() const { return m_QueryPersonName; }
QString Config::QueryPersonUuid() const { return m_QueryPersonUuid; }
QString Config::FontNameEvents() const { return m_FontNameEvents; }
QString Config::QueryServers() const { return m_QueryServers; }
int Config::ButtonSize() const { return m_ButtonSize; }
int Config::SplashTime() const { return m_SplashTime; }
int Config::SplashSize() const { return m_SplashSize; }
QString Config::PathApp() const { return m_PathAppDir; }
QString Config::PathAppConfig() const { return m_PathAppConfig; }
QString Config::PathLocalDB() const { return m_PathLocalDB; }