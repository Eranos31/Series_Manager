#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QTextCodec>
#include <QDate>
#include <QSettings>
#include <QStandardPaths>

class Log
{
public:
    Log();
    Log(QString nom);
    void ecrire(QString text);
private slots:
    void supprimerAncienFichier();
private:
    QString nomFichier;
};

#endif // LOG_H
