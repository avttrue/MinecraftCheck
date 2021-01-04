cmake_minimum_required(VERSION 3.5)

set(APP_NAME
    "mcchk"
    )

set(CXX_FLAGS
    "-fno-sized-deallocation"
    )

set(RES_FILES
    resources.qrc
    )

set(PROJECT_SOURCES
    config.cpp
    config.h
    controls.cpp
    controls.h
    dbbrowser.cpp
    dbbrowser.h
    dialogs/dialogvalueslist.cpp
    dialogs/dialogvalueslist.h
    helper.cpp
    helper.h
    helpergraphics.cpp
    helpergraphics.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mojangapi.cpp
    mojangapi.h
    properties.cpp
    properties.h
    splashscreen.cpp
    splashscreen.h
    textlog.cpp
    textlog.h
    ${RES_FILES}
    )

project(${APP_NAME} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS ${CXX_FLAGS})

find_package(QT NAMES Qt6 Qt5
    COMPONENTS Core Gui Network Sql Svg
    REQUIRED)

find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS Core Gui Network Sql Svg
    REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${APP_NAME} ${PROJECT_SOURCES})

else()
    add_executable(${APP_NAME} ${PROJECT_SOURCES})

endif()

target_link_libraries(${APP_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Svg
    )
