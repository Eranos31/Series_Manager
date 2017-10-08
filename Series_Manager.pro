#-------------------------------------------------
#
# Project created by QtCreator 2016-11-06T14:55:57
#
#-------------------------------------------------

QT       += core gui\
            sql\
            network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Series_Manager
TEMPLATE = app


SOURCES += main.cpp\
        fenetreprincipale.cpp \
    basededonnees.cpp \
    log.cpp \
    methodediverses.cpp \
    dialog.cpp \
    filedownloader.cpp

HEADERS  += fenetreprincipale.h \
    basededonnees.h \
    log.h \
    methodediverses.h \
    dialog.h \
    filedownloader.h

FORMS    += fenetreprincipale.ui \
    dialog.ui

OTHER_FILES += TODO.txt

RC_ICONS = SeriesManager.ico

RESOURCES += \
    ressource.qrc
