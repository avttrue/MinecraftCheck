#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QString>
#include <QSettings>

/*!
 * \brief Config - настройки программы с загрузкой из файла конфига и сохранением в файл.
 * Интерфейс над QSettings.
 */
class Config : public QObject
{
    Q_OBJECT

public:
    explicit Config(const QString& in_AppDirectory);

    int SplashTime() const;
    int SplashSize() const;
    QString PathApp() const;
    QString PathAppConfig() const;
    QString PathLocalDB() const;
    int ButtonSize() const;
    void setButtonSize(int value);
    QString QueryServers() const;
    void setQueryServers(const QString &value);
    QString FontNameEvents() const;
    void setFontNameEvents(const QString &value);
    QString QueryPersonUuid() const;
    void setQueryPersonUuid(const QString &value);
    QString QueryPersonName() const;
    void setQueryPersonName(const QString &value);
    QString QueryProfileUuid() const;
    void setQueryProfileUuid(const QString &value);
    QString DateTimeFormat() const;
    void setDateTimeFormat(const QString &value);
    QString MojangDefaultSkin() const;
    void setMojangDefaultSkin(const QString &value);
    QString MojangModel1() const;
    void setMojangModel1(const QString &value);
    QString MojangModel2() const;
    void setMojangModel2(const QString &value);
    QString MojangImageFormat() const;
    void setMojangImageFormat(const QString &value);
    int ReportLedSize() const;
    void setReportLedSize(int value);
    bool ReportAutoOpen() const;
    void setReportAutoOpen(bool value);
    int ReportImgScale() const;
    void setReportImgScale(int value);
    bool AutoVacuum() const;
    void setAutoVacuum(bool value);
    bool AutoCollectProfiles() const;
    void setAutoCollectProfiles(bool value);
    bool AdvancedDBMode() const;
    void setAdvancedDBMode(bool value);
    QString LastDir() const;
    void setLastDir(const QString &value);
    bool KeepCommentsAtUpd() const;
    void setKeepCommentsAtUpd(bool value);
    int LogSize() const;
    void setLogSize(int value);
    bool SIMetric() const;
    void setSIMetric(bool value);
    bool OpenUrls() const;
    void setOpenUrls(bool value);
    int ReportMargins() const;
    void setReportMargins(int value);
    bool UseQtHtmlContent() const;
    void setUseQtHtmlContent(bool value);
    QString TableSkinMode() const;
    void setTableSkinMode(const QString &value);
    int TablePortraitSize() const;
    void setTablePortraitSize(int value);
    int TableSkinSize() const;
    void setTableSkinSize(int value);
    int ReportPortraitSize() const;
    void setReportPortraitSize(int value);
    bool ReportAddPortrait() const;
    void setReportAddPortrait(bool value);
    int TableCapeSize() const;
    void setTableCapeSize(int value);
    bool ShowCapeImage() const;
    void setShowCapeImage(bool value);

protected:
    void load();

private:
    QSettings* m_Settings;
    QString m_DateTimeFormat;           // формат отображения даты и времени
    QString m_PathAppConfig;            // путь до конфига приложения
    QString m_PathAppDir;               // путь до приложения
    QString m_LastDir;                  // путь до последнего каталога
    QString m_PathLocalDB;              // путь до локальной БД
    QString m_QueryServers;             // запрос статусов серверов
    QString m_QueryPersonUuid;          // запрос о персонаже по uuid
    QString m_QueryPersonName;          // запрос о персонаже по нику
    QString m_QueryProfileUuid;         // запрос о профиле по uuid
    QString m_MojangDefaultSkin;        // скин по-умолчанию
    QString m_MojangModel1;             // модель Steve
    QString m_MojangModel2;             // модель Alex
    QString m_MojangImageFormat;        // формат картинок в Mojang API
    QString m_FontNameEvents;           // семейство шрифта лога событий
    QString m_TableSkinMode;            // отображение скина в таблице: 'portrait', 'skin', 'none' (любое)
    int m_TableSkinSize;                // размер скина в таблице
    int m_TableCapeSize;                // размер плаща в таблице
    int m_TablePortraitSize;            // размер портрета в таблице
    int m_ReportPortraitSize;           // размер портрета в отчёте
    int m_SplashTime;                   // время отображения сплеш-заставки
    int m_SplashSize;                   // размер сплеш-заставки
    int m_ButtonSize;                   // размеры кнопок интерфейса
    int m_ReportLedSize;                // размер картинки индикатора серверов
    int m_ReportImgScale;               // масштаб картинок в отображении профиля игрока
    int m_LogSize;                      // количество строк лога событий
    int m_ReportMargins;                // отступы в % от краёв окна при выводе отчёта
    bool m_ReportAddPortrait;           // добавлять портреты в отчёт
    bool m_ReportAutoOpen;              // открывать отчёт после сохранения
    bool m_AutoVacuum;                  // выполнять vacuum при выходе
    bool m_AutoCollectProfiles;         // сохранять все профили игроков в БД
    bool m_AdvancedDBMode;              // возможность редактировать БД
    bool m_KeepCommentsAtUpd;           // сохранять комментарии при обновлении профиля
    bool m_SI_metric;                   // использовать систему СИ в отображении размеров файлов
    bool m_OpenUrls;                    // открывать URLы системным браузером
    bool m_UseQtHtmlContent;            // разрешать Qt фрматировать содержание html документов
    bool m_ShowCapeImage;               // отображать плащ в таблице как изображение
};

#endif // CONFIG_H
