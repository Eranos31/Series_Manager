#ifndef BASEDEDONNEES_H
#define BASEDEDONNEES_H

#include <QtCore>
#include <QSql>
#include <QSqlDatabase>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "log.h"
#include "methodediverses.h"


class BaseDeDonnees
{
public:
    BaseDeDonnees();
    ~BaseDeDonnees();
    void ajouter(QString nom, int saison, int nbEpisode, int jourSortie, QDate dateSortie, QString wiki, QString lien);
    void reporter(QString nom, QDate dateModif);
    void modifier(QString nom, int saison, int nbEpisode, int episodeCourant, int jourSortie, QDate date, QString wiki, QDate dateModif, QString lien, bool message);
    void supprimer(QString nom, bool msgBox);
    QList<QMap<QString, QString> > requeteListe(QString requete);
    QList<QMap<QString, QString> > requeteListeHier(QString requete);
    QMap<QString, QString> requete(QString requete);
    bool requeteInsertUpdate(QString requete);
    QList<QMap<QString, QString> > requeteHistorique();
    QDate derniereOuvertureBDD();
    void majDerniereOuvertureBDD();
    QMap<QString, QString> requeteVersion();
    QString requeteUneColonne(QString requete);
private:
    QSqlDatabase db;
    Log *log;
    MethodeDiverses methodeDiverses;
};

#endif // BASEDEDONNEES_H
