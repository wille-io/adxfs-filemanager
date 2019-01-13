#-------------------------------------------------
#
# Project created by QtCreator 2015-03-02T16:12:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = adxexp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    VFI.cpp \
    RDBAFake.cpp \
    VFIEntryWidget.cpp

HEADERS  += mainwindow.h \
    VFI.h \
    List.h \
    header.h \
    RDBAFake.h \
    VFIEntryWidget.h

FORMS    += mainwindow.ui
