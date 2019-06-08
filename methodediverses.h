#ifndef METHODEDIVERSES_H
#define METHODEDIVERSES_H
#include <QString>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>

class MethodeDiverses
{
public:
    MethodeDiverses();
    QString dayToString(const int date);
    int dayToInt(const QString date);
    int stringToInt(const QString date);
    bool msgBoxQuestion(const QString text);
    QTableWidgetItem* itemForTableWidget(const QString text, bool center);
    QString dateToString(QDate date);
    QDate stringToDate(QString text);
    QDate jmaToDate(QString text);
    QString formalismeEntier(int entier);
    QString formatDate(QDate date);
    QString formatDateJJMMAA(QDate date);
    QString monthToString(int mois);
    static void ecrireLog(QString message);
    static void supprimerAncienFichier();
    static QString getConfig(QString config);
    static int getConfig(QString config, int valeur);
    static void setConfig(QString config, QString valeur);
};

#endif // METHODEDIVERSES_H
