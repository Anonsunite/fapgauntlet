#-------------------------------------------------
#
# Project created by QtCreator 2017-04-09T16:12:58
#
#-------------------------------------------------

QT       += core gui

lessThan(QT_MAJOR_VERSION, 5): QT += webkit
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webengine webenginewidgets

TARGET = FapGauntlet
TEMPLATE = app
LIBS += -lvlc -lcurl

SOURCES += main.cpp\
        mainwindow.cpp \
    image.cpp \
    videoplayer.cpp \
    downloadmanager.cpp \
    qdownloader.cpp \
    jsoncpp.cpp \
    settingswindow.cpp \
    settings.cpp \
    runguard.cpp \
    qdownloaderthread.cpp

HEADERS  += mainwindow.h \
    image.h \
    videoplayer.h \
    downloadmanager.h \
    qdownloader.h \
    json/json-forwards.h \
    json/json.h \
    settingswindow.h \
    settings.h \
    runguard.h \
    qdownloaderthread.h

FORMS    += mainwindow.ui \
    downloadmanager.ui \
    settingswindow.ui
