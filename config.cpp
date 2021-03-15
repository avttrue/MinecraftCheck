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

    QObject::connect(this, &QObject::destroyed, [=]() { qInfo() << "Config destroyed"; });
}

void Config::load()
{
    if(!m_Settings->contains("LastCatalog"))
        m_Settings->setValue("LastCatalog", m_PathAppDir);
    m_LastDir = m_Settings->value("LastCatalog").toString();

    if(!m_Settings->contains("SIMetric"))
        m_Settings->setValue("SIMetric", SI_METRIC);
    m_SI_metric = m_Settings->value("SIMetric").toBool();

    if(!m_Settings->contains("MainWindow/Height"))
        m_Settings->setValue("MainWindow/Height", WINDOW_HEIGHT);

    if(!m_Settings->contains("MainWindow/Width"))
        m_Settings->setValue("MainWindow/Width", WINDOW_WIDTH);

    if(!m_Settings->contains("MainWindow/ConfigWindowWidth"))
        m_Settings->setValue("MainWindow/ConfigWindowWidth", CONFIG_WINDOW_WIDTH);
    m_ConfigWindowWidth = m_Settings->value("MainWindow/ConfigWindowWidth").toInt();

    if(!m_Settings->contains("MainWindow/ConfigWindowHeight"))
        m_Settings->setValue("MainWindow/ConfigWindowHeight", CONFIG_WINDOW_HEIGHT);
    m_ConfigWindowHeight = m_Settings->value("MainWindow/ConfigWindowHeight").toInt();

    if(!m_Settings->contains("MainWindow/SearchWindowWidth"))
        m_Settings->setValue("MainWindow/SearchWindowWidth", SEARCH_WINDOW_WIDTH);
    m_SearchWindowWidth = m_Settings->value("MainWindow/SearchWindowWidth").toInt();

    if(!m_Settings->contains("MainWindow/SearchWindowHeight"))
        m_Settings->setValue("MainWindow/SearchWindowHeight", SEARCH_WINDOW_HEIGHT);
    m_SearchWindowHeight = m_Settings->value("MainWindow/SearchWindowHeight").toInt();

    if(!m_Settings->contains("MainWindow/CommentWindowWidth"))
        m_Settings->setValue("MainWindow/CommentWindowWidth", COMMENT_WINDOW_WIDTH);
    m_CommentWindowWidth = m_Settings->value("MainWindow/CommentWindowWidth").toInt();

    if(!m_Settings->contains("MainWindow/CommentWindowHeight"))
        m_Settings->setValue("MainWindow/CommentWindowHeight", COMMENT_WINDOW_HEIGHT);
    m_CommentWindowHeight = m_Settings->value("MainWindow/CommentWindowHeight").toInt();

    if(!m_Settings->contains("MainWindow/ProfViewWindowWidth"))
        m_Settings->setValue("MainWindow/ProfViewWindowWidth", PROFVIEW_WINDOW_WIDTH);
    m_ProfViewWindowWidth = m_Settings->value("MainWindow/ProfViewWindowWidth").toInt();

    if(!m_Settings->contains("MainWindow/ProfViewWindowHeight"))
        m_Settings->setValue("MainWindow/ProfViewWindowHeight", PROFVIEW_WINDOW_HEIGHT);
    m_ProfViewWindowHeight = m_Settings->value("MainWindow/ProfViewWindowHeight").toInt();

    if(!m_Settings->contains("MainWindow/OpenURLs"))
        m_Settings->setValue("MainWindow/OpenURLs", OPEN_URLS);
    m_OpenUrls = m_Settings->value("MainWindow/OpenURLs").toBool();

    if(!m_Settings->contains("MainWindow/SplashTime"))
        m_Settings->setValue("MainWindow/SplashTime", SPLASH_TIME);
    m_SplashTime = m_Settings->value("MainWindow/SplashTime").toInt();

    if(!m_Settings->contains("MainWindow/SplashServiceTime"))
        m_Settings->setValue("MainWindow/SplashServiceTime", SPLASH_SERVICE_TIME);
    m_SplashServiceTime = m_Settings->value("MainWindow/SplashServiceTime").toInt();

    if(!m_Settings->contains("MainWindow/CapeWidthHeightAspect"))
        m_Settings->setValue("MainWindow/CapeWidthHeightAspect", CAPE_WIDTH_HEIGHT_ASPECT);
    m_CapeWHAspect = m_Settings->value("MainWindow/CapeWidthHeightAspect").toInt();

    if(!m_Settings->contains("MainWindow/SplashSize"))
        m_Settings->setValue("MainWindow/SplashSize", SPLASH_SIZE);
    m_SplashSize = m_Settings->value("MainWindow/SplashSize").toInt();

    if(!m_Settings->contains("MainWindow/LogSize"))
        m_Settings->setValue("MainWindow/LogSize", LOG_SIZE);
    m_LogSize = m_Settings->value("MainWindow/LogSize").toInt();

    if(!m_Settings->contains("MainWindow/ButtonSize"))
        m_Settings->setValue("MainWindow/ButtonSize", BUTTON_SIZE);
    m_ButtonSize = m_Settings->value("MainWindow/ButtonSize").toInt();

    if(!m_Settings->contains("MainWindow/TablePortraitSize"))
        m_Settings->setValue("MainWindow/TablePortraitSize", TABLE_PORTRAIT_SIZE);
    m_TablePortraitSize = m_Settings->value("MainWindow/TablePortraitSize").toInt();

    if(!m_Settings->contains("MainWindow/TableSkinSize"))
        m_Settings->setValue("MainWindow/TableSkinSize", TABLE_SKIN_SIZE);
    m_TableSkinSize = m_Settings->value("MainWindow/TableSkinSize").toInt();

    if(!m_Settings->contains("MainWindow/TableCapeSize"))
        m_Settings->setValue("MainWindow/TableCapeSize", TABLE_CAPE_SIZE);
    m_TableCapeSize = m_Settings->value("MainWindow/TableCapeSize").toInt();

    if(!m_Settings->contains("MainWindow/ShowCapeImage"))
        m_Settings->setValue("MainWindow/ShowCapeImage", SHOW_CAPE_IMAGE);
    m_ShowCapeImage = m_Settings->value("MainWindow/ShowCapeImage").toBool();

    if(!m_Settings->contains("MainWindow/FontNameEvents"))
        m_Settings->setValue("MainWindow/FontNameEvents", FONT_NAME_EVENTS);
    m_FontNameEvents = m_Settings->value("MainWindow/FontNameEvents").toString();

    if(!m_Settings->contains("MainWindow/TableSkinMode"))
        m_Settings->setValue("MainWindow/TableSkinMode", TABLE_SKIN_MODE);
    m_TableSkinMode = m_Settings->value("MainWindow/TableSkinMode").toString();

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

    if(!m_Settings->contains("Report/CaptionColor"))
        m_Settings->setValue("Report/CaptionColor", REPORT_CAPTION_COLOR);
    m_ReportCaptionColor = m_Settings->value("Report/CaptionColor").toString();

    if(!m_Settings->contains("Report/LedSize"))
        m_Settings->setValue("Report/LedSize", REPORT_LED_SIZE);
    m_ReportLedSize = m_Settings->value("Report/LedSize").toInt();

    if(!m_Settings->contains("Report/ImgScale"))
        m_Settings->setValue("Report/ImgScale", REPORT_IMG_SCALE);
    m_ReportImgScale = m_Settings->value("Report/ImgScale").toInt();

    if(!m_Settings->contains("Report/PortraitSize"))
        m_Settings->setValue("Report/PortraitSize", REPORT_PORTRAIT_SIZE);
    m_ReportPortraitSize = m_Settings->value("Report/PortraitSize").toInt();

    if(!m_Settings->contains("Report/Margins"))
        m_Settings->setValue("Report/Margins", REPORT_MARGINS);
    m_ReportMargins = m_Settings->value("Report/Margins").toInt();

    if(!m_Settings->contains("Report/AutoOpen"))
        m_Settings->setValue("Report/AutoOpen", REPORT_AUTOOPEN);
    m_ReportAutoOpen = m_Settings->value("Report/AutoOpen").toBool();

    if(!m_Settings->contains("Report/AddPortrait"))
        m_Settings->setValue("Report/AddPortrait", REPORT_ADD_PORTRAIT);
    m_ReportAddPortrait = m_Settings->value("Report/AddPortrait").toBool();

    if(!m_Settings->contains("Report/UseQtHtmlContent"))
        m_Settings->setValue("Report/UseQtHtmlContent", USE_QT_HTML_CONTENT);
    m_UseQtHtmlContent = m_Settings->value("Report/UseQtHtmlContent").toBool();

    if(!m_Settings->contains("Database/AutoVacuum"))
        m_Settings->setValue("Database/AutoVacuum", AUTOVACUUM);
    m_AutoVacuum = m_Settings->value("Database/AutoVacuum").toBool();

    if(!m_Settings->contains("Database/AutoCollectProfiles"))
        m_Settings->setValue("Database/AutoCollectProfiles", AUTOCOLLECT_PROFILES);
    m_AutoCollectProfiles = m_Settings->value("Database/AutoCollectProfiles").toBool();

    if(!m_Settings->contains("Database/AdvancedMode"))
        m_Settings->setValue("Database/AdvancedMode", ADVANCED_DB_MODE);
    m_AdvancedDBMode = m_Settings->value("Database/AdvancedMode").toBool();

    if(!m_Settings->contains("Database/KeepCommentsAtUpd"))
        m_Settings->setValue("Database/KeepCommentsAtUpd", KEEP_COMMENTS_AT_UPD);
    m_KeepCommentsAtUpd = m_Settings->value("Database/KeepCommentsAtUpd").toBool();
}

void Config::setProfViewWindowHeight(int value)
{
    if(m_ProfViewWindowHeight == value) return;

    m_ProfViewWindowHeight = value;
    m_Settings->setValue("MainWindow/ProfViewWindowHeight", m_ProfViewWindowHeight);
}

void Config::setProfViewWindowWidth(int value)
{
    if(m_ProfViewWindowWidth == value) return;

    m_ProfViewWindowWidth = value;
    m_Settings->setValue("MainWindow/ProfViewWindowWidth", m_ProfViewWindowWidth);
}

void Config::setCommentWindowHeight(int value)
{
    if(m_CommentWindowHeight == value) return;

    m_CommentWindowHeight = value;
    m_Settings->setValue("MainWindow/CommentWindowHeight", m_CommentWindowHeight);
}

void Config::setCommentWindowWidth(int value)
{
    if(m_CommentWindowWidth == value) return;

    m_CommentWindowWidth = value;
    m_Settings->setValue("MainWindow/CommentWindowWidth", m_CommentWindowWidth);
}

void Config::setSearchWindowHeight(int value)
{
    if(m_SearchWindowHeight == value) return;

    m_SearchWindowHeight = value;
    m_Settings->setValue("MainWindow/SearchWindowHeight", m_SearchWindowHeight);
}

void Config::setSearchWindowWidth(int value)
{
    if(m_SearchWindowWidth == value) return;

    m_SearchWindowWidth = value;
    m_Settings->setValue("MainWindow/SearchWindowWidth", m_SearchWindowWidth);
}

void Config::setConfigWindowHeight(int value)
{
    if(m_ConfigWindowHeight == value) return;

    m_ConfigWindowHeight = value;
    m_Settings->setValue("MainWindow/ConfigWindowHeight", m_ConfigWindowHeight);
}

void Config::setConfigWindowWidth(int value)
{
    if(m_ConfigWindowWidth == value) return;

    m_ConfigWindowWidth = value;
    m_Settings->setValue("MainWindow/ConfigWindowWidth", m_ConfigWindowWidth);
}

void Config::setReportCaptionColor(const QString &value)
{
    if(m_ReportCaptionColor == value) return;
    
    m_ReportCaptionColor = value;
    m_Settings->setValue("Report/CaptionColor", m_ReportCaptionColor);
}

void Config::setCapeWHAspect(int value)
{
    if(m_CapeWHAspect == value) return;

    m_CapeWHAspect = value;
    m_Settings->setValue("MainWindow/CapeWidthHeightAspect", m_CapeWHAspect);
}

void Config::setSplashServiceTime(int value)
{
    if(m_SplashServiceTime == value) return;

    m_SplashServiceTime = value;
    m_Settings->setValue("MainWindow/SplashServiceTime", m_SplashServiceTime);
}

void Config::setShowCapeImage(bool value)
{
    if(m_ShowCapeImage == value) return;

    m_ShowCapeImage = value;
    m_Settings->setValue("MainWindow/ShowCapeImage", m_ShowCapeImage);
}

void Config::setTableCapeSize(int value)
{
    if(m_TableCapeSize == value) return;

    m_TableCapeSize = value;
    m_Settings->setValue("MainWindow/TableCapeSize", m_TableCapeSize);
}

void Config::setReportAddPortrait(bool value)
{
    if(m_ReportAddPortrait == value) return;
    
    m_ReportAddPortrait = value;
    m_Settings->setValue("Report/AddPortrait", m_ReportAddPortrait);
}

void Config::setReportPortraitSize(int value)
{
    if(m_ReportPortraitSize == value) return;

    m_ReportPortraitSize = value;
    m_Settings->setValue("Report/PortraitSize", m_ReportPortraitSize);
}

void Config::setTableSkinSize(int value)
{
    if(m_TableSkinSize == value) return;

    m_TableSkinSize = value;
    m_Settings->setValue("MainWindow/TableSkinSize", m_TableSkinSize);
}

void Config::setTablePortraitSize(int value)
{
    if(m_TablePortraitSize == value) return;

    m_TablePortraitSize = value;
    m_Settings->setValue("MainWindow/TablePortraitSize", m_TablePortraitSize);
}

void Config::setTableSkinMode(const QString &value)
{
    if(m_TableSkinMode == value) return;

    m_TableSkinMode = value;
    m_Settings->setValue("MainWindow/TableSkinMode", m_TableSkinMode);
}

void Config::setUseQtHtmlContent(bool value)
{
    if(m_UseQtHtmlContent == value) return;

    m_UseQtHtmlContent = value;
    m_Settings->setValue("Report/UseQtHtmlContent", m_UseQtHtmlContent);
}

void Config::setReportMargins(int value)
{
    if(m_ReportMargins == value) return;
    
    m_ReportMargins = value;
    m_Settings->setValue("Report/Margins", m_ReportMargins);
}

void Config::setOpenUrls(bool value)
{
    if(m_OpenUrls == value) return;

    m_OpenUrls = value;
    m_Settings->setValue("MainWindow/OpenURLs", m_OpenUrls);
}

void Config::setSIMetric(bool value)
{
    if(m_SI_metric == value) return;

    m_SI_metric = value;
    m_Settings->setValue("SIMetric", m_SI_metric);
}

void Config::setLogSize(int value)
{
    if(m_LogSize == value) return;
    
    m_LogSize = value >= 0 ? value : 0;
    m_Settings->setValue("MainWindow/LogSize", m_LogSize);
}

void Config::setKeepCommentsAtUpd(bool value)
{
    if(m_KeepCommentsAtUpd == value) return;

    m_KeepCommentsAtUpd = value;
    m_Settings->setValue("Database/KeepCommentsAtUpd", m_KeepCommentsAtUpd);
}

void Config::setLastDir(const QString &value)
{
    if(m_LastDir == value) return;

    m_LastDir = value;
    m_Settings->setValue("LastCatalog", m_LastDir);
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

int Config::CapeWHAspect() const { return m_CapeWHAspect; }
int Config::SplashServiceTime() const { return m_SplashServiceTime; }
bool Config::ShowCapeImage() const { return m_ShowCapeImage; }
int Config::TableCapeSize() const { return m_TableCapeSize; }
bool Config::ReportAddPortrait() const { return m_ReportAddPortrait; }
int Config::ReportPortraitSize() const { return m_ReportPortraitSize; }
int Config::TableSkinSize() const { return m_TableSkinSize; }
int Config::TablePortraitSize() const { return m_TablePortraitSize; }
QString Config::TableSkinMode() const { return m_TableSkinMode; }
bool Config::UseQtHtmlContent() const { return m_UseQtHtmlContent; }
int Config::ReportMargins() const { return m_ReportMargins; }
bool Config::OpenUrls() const { return m_OpenUrls; }
bool Config::SIMetric() const { return m_SI_metric; }
int Config::LogSize() const { return m_LogSize; }
bool Config::KeepCommentsAtUpd() const { return m_KeepCommentsAtUpd; }
QString Config::LastDir() const { return m_LastDir; }
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
QString Config::ReportCaptionColor() const { return m_ReportCaptionColor; }
int Config::ConfigWindowWidth() const { return m_ConfigWindowWidth; }
int Config::ConfigWindowHeight() const { return m_ConfigWindowHeight; }
int Config::SearchWindowHeight() const { return m_SearchWindowHeight; }
int Config::SearchWindowWidth() const { return m_SearchWindowWidth; }
int Config::CommentWindowHeight() const { return m_CommentWindowHeight; }
int Config::CommentWindowWidth() const { return m_CommentWindowWidth; }
int Config::ProfViewWindowHeight() const { return m_ProfViewWindowHeight; }
int Config::ProfViewWindowWidth() const { return m_ProfViewWindowWidth; }
