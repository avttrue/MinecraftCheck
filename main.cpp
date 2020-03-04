#include "mainwindow.h"
#include "properties.h"
#include "helper.h"

#include <QApplication>
#include <QSplashScreen>
#include <QTextCodec>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setStyleSheet(getTextFromRes(":/resources/main.css"));

    QTextCodec::setCodecForLocale(QTextCodec::codecForName(TEXT_CODEC.toLatin1()));
    config = new Config(application.applicationDirPath());

    QSplashScreen splash;
    splash.setPixmap(QPixmap(":/resources/img/mainicon512.png").
                     scaled(config->SplashSize(), config->SplashSize(),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    splash.setWindowIcon(QIcon(":/resources/img/mainicon512.png"));
    splash.setWindowModality(Qt::WindowModal);
    splash.setWindowState(Qt::WindowActive);
    splash.show();
    QEventLoop loop;
    QTimer::singleShot(config->SplashTime(), &loop, &QEventLoop::quit);
    loop.exec();

    MainWindow window;
    window.show();
    splash.finish(&window);

    return application.exec();
}
