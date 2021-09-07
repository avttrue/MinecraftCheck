#include "mainwindow.h"
#include "properties.h"
#include "helper.h"
#include "splashscreen.h"

#include <QApplication>
#include <QTextCodec>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setStyleSheet(getTextFromRes(":/resources/main.css"));

    QTextCodec::setCodecForLocale(QTextCodec::codecForName(TEXT_CODEC.toLatin1()));
    config = new Config(application.applicationDirPath());

    SplashScreen splash;
    splash.show();

    MainWindow window;

    QEventLoop loop;
    QTimer::singleShot(config->SplashTime(), &application, [&application, &window, &splash, &loop]()
    {
        splash.finish(&window);
        window.show();
        application.setActiveWindow(&window);
        loop.quit();
    });
    loop.exec(QEventLoop::ExcludeUserInputEvents);


    return application.exec();
}
