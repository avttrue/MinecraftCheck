QT       += core gui network sql svg
#https://stackoverflow.com/a/53234995/12177714
QMAKE_CXXFLAGS += "-fno-sized-deallocation"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET      = mcchk
TEMPLATE    = app
VERSION     = 1.0

DEFINES     += APP_VERS=\\\"$${VERSION}\\\"
DEFINES     += GIT_VERS=\\\"$$system(git describe --always)\\\"
#DEFINES     += GIT_VERS=\\\"-------\\\"
DEFINES     += BUILD_DATE='"\\\"$(shell date)\\\""'
DEFINES     += QT_DEPRECATED_WARNINGS

SOURCES += \
    config.cpp \
    controls.cpp \
    dbbrowser.cpp \
    dialogs/dialogvalueslist.cpp \
    helper.cpp \
    helpergraphics.cpp \
    main.cpp \
    mainwindow.cpp \
    mojangapi.cpp \
    properties.cpp \
    splashscreen.cpp \
    textlog.cpp

HEADERS += \
    config.h \
    controls.h \
    dbbrowser.h \
    dialogs/dialogvalueslist.h \
    helper.h \
    helpergraphics.h \
    mainwindow.h \
    mojangapi.h \
    properties.h \
    splashscreen.h \
    textlog.h

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
