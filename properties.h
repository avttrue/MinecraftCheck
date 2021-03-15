/*
 * параметры приложения
 */
#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "config.h"

#include <QString>

extern Config* config; // интерфейс над QSettings

// дефолтные параметры
const QString DT_FORMAT =               "yyyy.MM.dd hh:mm:ss";
const QString TEXT_CODEC =              "UTF-8";
const QString APP_NAME =                "Minecraft Checker";
const QString APP_VERSION =             "1.2";
const QString APP_CFG =                 "config.cfg";
const QString LOCAL_DB =                "profiles.db";
const QString TOOLBUTTON_NAME =         "MCCheckerToolButton"; // see main.css
const QString FONT_NAME_EVENTS =        "monospace";
const QString QUERY_SERVERS =           "https://status.mojang.com/check";
const QString QUERY_PERSON_UUID =       "https://api.mojang.com/user/profiles/%1/names";
const QString QUERY_PERSON_NAME =       "https://api.mojang.com/users/profiles/minecraft/%1";
const QString QUERY_PROFILE_UUID =      "https://sessionserver.mojang.com/session/minecraft/profile/%1";
const QString MOJANG_DEFAULT_SKIN =     "http://assets.mojang.com/SkinTemplates/%1.png";
const QString MOJANG_IMAGE_FORMAT =     "PNG";
const QString MOJANG_MODEL1 =           "Alex";
const QString MOJANG_MODEL2 =           "Steve";
const QString DATABASE_TYPE =           "QSQLITE";
const QString TABLE_SKIN_MODE =         "portrait"; // 'portrait', 'skin', 'none'
const QString REPORT_CAPTION_COLOR =    "#B0E0E6";
const auto TABLE_SKIN_SIZE =            64;
const auto TABLE_CAPE_SIZE =            64;
const auto CAPE_WIDTH_HEIGHT_ASPECT =   2;
const auto TABLE_PORTRAIT_SIZE =        32;
const auto REPORT_PORTRAIT_SIZE =       64;
const auto REPORT_LED_SIZE =            32;
const auto REPORT_IMG_SCALE =           3;
const auto REPORT_MARGINS =             25;
const auto WINDOW_HEIGHT =              700;
const auto WINDOW_WIDTH =               900;
const auto CONFIG_WINDOW_HEIGHT =       480;
const auto CONFIG_WINDOW_WIDTH =        480;
const auto SEARCH_WINDOW_HEIGHT =       250;
const auto SEARCH_WINDOW_WIDTH =        350;
const auto COMMENT_WINDOW_HEIGHT =      200;
const auto COMMENT_WINDOW_WIDTH =       350;
const auto PROFVIEW_WINDOW_HEIGHT =     480;
const auto PROFVIEW_WINDOW_WIDTH =      480;
const auto SPLASH_TIME =                1000;
const auto SPLASH_SIZE =                512;
const auto SPLASH_SERVICE_TIME =        50;
const auto BUTTON_SIZE =                26;
const auto SEARCH_LINE_WIDTH =          400;
const auto LOG_SIZE =                   0;
const auto TASK_SEPARATOR_LEN =         42;
const auto REPORT_AUTOOPEN =            false;
const auto REPORT_ADD_PORTRAIT =        true;
const auto USE_QT_HTML_CONTENT =        false;
const auto AUTOVACUUM =                 true;
const auto AUTOCOLLECT_PROFILES =       true;
const auto ADVANCED_DB_MODE =           false;
const auto KEEP_COMMENTS_AT_UPD =       true;
const auto SI_METRIC =                  false;
const auto OPEN_URLS =                  true;
const auto SHOW_CAPE_IMAGE =            true;

#endif // PROPERTIES_H
