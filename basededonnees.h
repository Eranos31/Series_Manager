#ifndef BASEDEDONNEES_H
#define BASEDEDONNEES_H

#include <QtCore>
#include <QSql>
#include <QSqlDatabase>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QStandardPaths>
#include "log.h"
#include "methodediverses.h"


class BaseDeDonnees
{
public:
    BaseDeDonnees();
    ~BaseDeDonnees();
    QDate derniereOuvertureBDD();
    void majDerniereOuvertureBDD();
    // Nouvelle procédure
    QList<QMap<QString, QString> > requeteSelect(QList<QString> champs, QString table, QList<QString> jointures, QList<QString> conditions, QList<QString> ordres);
    QList<QMap<QString, QString> > requeteSelect(QList<QString> champs, QString table, QList<QString> conditions, QList<QString> ordres);
    bool requeteInsert(QList<QString> champs, QList<QString> valeurs, QString table);
    bool requeteUpdate(QList<QString> champs, QString table, QList<QString> conditions);
    bool requeteDelete(QString table, QList<QString> conditions);

    // TABLE FS#FICHE_SERIE
    const QString FICHE_SERIE_TABLE = "`FS#FICHE_SERIE`";
    const QString FICHE_SERIE_ID = "`FS#ID`";
    const QString FICHE_SERIE_NOM = "FSNOM";
    const QString FICHE_SERIE_IMAGE = "FSIMAGE";
    const QString FICHE_SERIE_WIKI = "FSWIKI";
    const QString FICHE_SERIE_ADDICTED = "FSADDICTED";
    const QString FICHE_SERIE_TERMINE = "FSTERMINE";
    // TABLE SA#SAISON
    const QString SAISON_TABLE = "`SA#SAISON`";
    const QString SAISON_ID = "`SAFS#ID`";
    const QString SAISON_SAISON = "SASAISON";
    const QString SAISON_NB_EPISODE = "SANBEPISODE";
    const QString SAISON_EPISODE_COURANT = "SAEPISODECOURANT";
    const QString SAISON_DATE_SORTIE = "SADATESORTIE";
    const QString SAISON_DATE_MODIF = "SADATEMODIF";
    const QString SAISON_WIKI = "SAWIKI";
    const QString SAISON_EPISODE_EN_PLUS = "SAEPISODEENPLUS";
    const QString SAISON_VU = "SAVU";
    // TABLE HISTORIQUE
    const QString HISTORIQUE_TABLE = "HISTORIQUE";
    const QString HISTORIQUE_NOM = "NOM";
    const QString HISTORIQUE_SAISON = "SAISON";
    const QString HISTORIQUE_EPISODE = "EPISODE";
    const QString HISTORIQUE_DATE_AJOUT = "DATEAJOUT";
    const QString HISTORIQUE_VU = "VU";
    // JOINTURE
    const QString JOINTURE_TYPE = "TYPE";
    const QString JOINTURE_TABLE = "TABLE";
    const QString JOINTURE_CONDITIONS = "CONDITIONS";
private:
    QSqlDatabase db;
    Log *log;
    MethodeDiverses methodeDiverses;
};

#endif // BASEDEDONNEES_H
