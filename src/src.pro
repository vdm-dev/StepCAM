PROJECT_ROOT = $${PWD}/..

QT += core gui widgets

TARGET = StepCAM
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

INCLUDEPATH += $${PROJECT_ROOT}/dep

SOURCES += \
    aboutdialog.cpp \
    excellonparser.cpp \
    hpglparser.cpp \
    logfiltermodel.cpp \
    logtablemodel.cpp \
    main.cpp \
    mainwindow.cpp \
    mousewheeleventfilter.cpp \
    progressstatuswidget.cpp \
    utilities.cpp

HEADERS += \
    aboutdialog.h \
    abstractparser.h \
    excellonparser.h \
    hpglparser.h \
    logfiltermodel.h \
    logitem.h \
    logtablemodel.h \
    mainwindow.h \
    mousewheeleventfilter.h \
    progressstatuswidget.h \
    utilities.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    progressstatuswidget.ui

RESOURCES += \
    src.qrc

win32:RC_FILE = src.rc
