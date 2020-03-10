#include "splashscreen.h"
#include "properties.h"

#include <QIcon>

SplashScreen::SplashScreen()
    : QSplashScreen()
{
    setPixmap(QPixmap(":/resources/img/mainicon512.png").
                     scaled(config->SplashSize(), config->SplashSize(),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    setWindowIcon(QIcon(":/resources/img/mainicon512.png"));
    setWindowModality(Qt::WindowModal);
    setWindowState(Qt::WindowActive);
    setWindowFlag(Qt::WindowStaysOnTopHint);
}
