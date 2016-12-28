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
    methodediverses.cpp

HEADERS  += fenetreprincipale.h \
    basededonnees.h \
    log.h \
    methodediverses.h

FORMS    += fenetreprincipale.ui

OTHER_FILES += TODO.txt\
               myapp.rc

RC_FILE = myapp.rc
