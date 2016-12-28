#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDate>

class Log
{
public:
    Log();
    Log(QString nom);
    void ecrire(QString text);
private:
    QString nomFichier;
};

#endif // LOG_H
