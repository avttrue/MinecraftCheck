/*
 * параметры приложения
 */
#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "config.h"

#include <QString>

extern Config* config; // интерфейс над QSettings

// дефолтные параметры
const QString DT_FORMAT = "yyyy.MM.dd hh:mm:ss";
const QString TEXT_CODEC = "UTF-8";
const QString APP_NAME = "Minecraft Checker";
const QString APP_CFG = "config.cfg";
const QString LOCAL_DB = "profiles.db";
const QString FONT_NAME_EVENTS = "monospace";
const QString QUERY_SERVERS = "https://status.mojang.com/check";
const QString QUERY_PERSON_UUID = "https://api.mojang.com/user/profiles/%1/names";
const QString QUERY_PERSON_NAME = "https://api.mojang.com/users/profiles/minecraft/%1";
const QString QUERY_PROFILE_UUID = "https://sessionserver.mojang.com/session/minecraft/profile/%1";
const QString MOJANG_DEFAULT_SKIN = "http://assets.mojang.com/SkinTemplates/%1.png";
const QString MOJANG_IMAGE_FORMAT = "PNG";
const QString MOJANG_MODEL1 = "Alex";
const QString MOJANG_MODEL2 = "Steve";
const QString DATABASE_TYPE = "QSQLITE";
const auto REPORT_LED_SIZE = 32;
const auto REPORT_IMG_SCALE = 3;
const auto WINDOW_HEIGHT = 700;
const auto WINDOW_WIDTH = 900;
const auto SPLASH_TIME = 1000;
const auto SPLASH_SIZE = 512;
const auto SPLASH_FIN_TIME = 50;
const auto BUTTON_SIZE = 32;
const auto SEARCH_LINE_WIDTH = 400;
const auto LOG_SIZE = 0;
const auto REPORT_AUTOOPEN = false;
const auto AUTOVACUUM = true;
const auto AUTOCOLLECT_PROFILES = true;
const auto ADVANCED_DB_MODE = false;
const auto KEEP_COMMENTS_AT_UPD = true;

#endif // PROPERTIES_H
