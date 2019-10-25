#ifndef BASEDEDONNEES_H
#define BASEDEDONNEES_H

#include <QtCore>
#include <QSql>
#include <QSqlDatabase>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QStandardPaths>
#include "methodediverses.h"


class BaseDeDonnees
{
public:
    struct Retour {
        bool reussi;
        QString  requete;
        QString erreur;
        QString info;
        QList<QMap<QString, QString> > liste;
    };

    // TABLE FS#FICHE_SERIE
    const QString TABLE_FICHE_SERIE = "`FS#FICHE_SERIE`";
    const QString FICHE_SERIE_ID = "`FS#ID`";
    const QString FICHE_SERIE_NOM = "`FSNOM`";
    const QString FICHE_SERIE_IMAGE = "`FSIMAGE`";
    const QString FICHE_SERIE_WIKI = "`FSWIKI`";
    const QString FICHE_SERIE_ADDICTED = "`FSADDICTED`";
    const QString FICHE_SERIE_TERMINE = "`FSTERMINE`";
    const QString FICHE_SERIE_TYPE_SERIE_ID = "`FSTS#ID`";
    // TABLE SA#SAISON
    const QString TABLE_SAISON = "`SA#SAISON`";
    const QString SAISON_ID = "`SAFS#ID`";
    const QString SAISON_SAISON = "`SASAISON`";
    const QString SAISON_NB_EPISODE = "`SANBEPISODE`";
    const QString SAISON_EPISODE_COURANT = "`SAEPISODECOURANT`";
    const QString SAISON_DATE_SORTIE = "`SADATESORTIE`";
    const QString SAISON_DATE_MODIF = "`SADATEMODIF`";
    const QString SAISON_WIKI = "`SAWIKI`";
    const QString SAISON_EPISODE_EN_PLUS = "`SAEPISODEENPLUS`";
    const QString SAISON_ETAT = "`SAETAT`";
    // TABLE HISTORIQUE
    const QString TABLE_HISTORIQUE = "`HISTORIQUE`";
    const QString HISTORIQUE_NOM = "`NOM`";
    const QString HISTORIQUE_SAISON = "`SAISON`";
    const QString HISTORIQUE_EPISODE = "`EPISODE`";
    const QString HISTORIQUE_DATE_AJOUT = "`DATEAJOUT`";
    const QString HISTORIQUE_ETAT = "`ETAT`";
    // TABLE TYPE
    const QString TABLE_TYPE_SERIE = "`TS#TYPE_SERIE`";
    const QString TYPE_SERIE_ID = "`TS#ID`";
    const QString TYPE_SERIE_NOM = "`TSNOM`";
    const QString TYPE_SERIE_RED = "`TSRED`";
    const QString TYPE_SERIE_GREEN = "`TSGREEN`";
    const QString TYPE_SERIE_BLUE = "`TSBLUE`";
    //const QString TYPE_SERIE_ = "``";

    const QString SELECT = "SELECT ";
    const QString UPDATE = "UPDATE ";
    const QString DELETE = "DELETE FROM ";
    const QString INSERT = "INSERT INTO ";
    const QString MAX = " MAX";
    const QString MIN = " MIN";
    const QString SUM = " SUM";
    const QString AVG = " AVG";
    const QString DISTINCT = " DISTINCT ";
    const QString FROM = " FROM ";
    const QString INNER_JOIN = " INNER JOIN ";
    const QString LEFT_JOIN = " LEFT JOIN ";
    const QString RIGHT_JOIN = " RIGHT JOIN ";
    const QString ON = " ON ";
    const QString WHERE = " WHERE ";
    const QString AND = " AND ";
    const QString EGALE = " = ";
    const QString VIRGULE = ", ";
    const QString ORDER_BY = " ORDER BY ";
    const QString DESC = "DESC";
    const QString SET = " SET ";
    const QString VALUES = " VALUES ";
    const QString CASE = " CASE ";
    const QString WHEN = " WHEN ";
    const QString IS_NULL = " IS NULL ";
    const QString THEN = " THEN ";
    const QString ELSE = " ELSE ";
    const QString END = " END ";
    const QString NOT = " NOT ";
    const QString IN = " IN ";
    const QString UPPER = " UPPER";
    const QString LOWER = " LOWER";

    BaseDeDonnees(QString chemin);
    Retour requeteSelect(QStringList champs, QString table, QStringList jointures, QStringList conditions, QStringList ordres);
    Retour requeteInsert(QStringList champs, QStringList valeurs, QString table);
    Retour requeteUpdate(QStringList champs, QString table, QStringList conditions);
    Retour requeteDelete(QString table, QStringList conditions);
    QString entreQuotes(QString champs);
    QString entreParentheses(QString champs);
    QString getRequeteSelect(QStringList champs, QString table, QStringList jointures, QStringList conditions, QStringList ordres);
    QDate derniereOuvertureBDD();
    void majDerniereOuvertureBDD();
private slots:
    QString getRequeteUpdate(QStringList champs, QString table, QStringList conditions);
    QString getRequeteDelete(QString table, QStringList conditions);
    QString getRequeteInsert(QStringList champs, QStringList valeurs, QString table);
    Retour execRequete(QString requete);

private:
    QSqlDatabase db;
    MethodeDiverses methodeDiverses;
};

#endif // BASEDEDONNEES_H
