#include "basededonnees.h"

BaseDeDonnees::BaseDeDonnees() {
    if(!QDir("./data/images").exists()) {
        QDir dir;
        dir.mkpath("./data/images");
    }
    QFileInfo info("./bdd.db");
    if(info.exists()) {
        QFile(info.absoluteFilePath()).rename(info.absolutePath() + "/data/bdd.db");
    }

    log = new Log("BaseDeDonnees");
    QFile *file = new QFile("./data/bdd.db");
    if(!file->exists()) {
        log->ecrire("BaseDeDonnees::BaseDeDonnees() : Creation de la base de donnée");
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("./data/bdd.db");

        if(db.open()) {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : Ouverture de la base de donnée");
            QSqlQuery query(db);

            query.prepare("CREATE TABLE DATEOUVERTURE"
                          "("
                          "DERNIEREOUVERTURE DATE PRIMARY KEY NOT NULL"
                          ")");

            if(query.exec()) {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : Création de la table DATEOUVERTURE réussie");
            } else {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur la création de la table DATEOUVERTURE : " + query.lastError().text());
                QMessageBox::critical(NULL, "ERREUR", "Erreur sur la création de la table DATEOUVERTURE : " + query.lastError().text());
            }

            query.prepare("INSERT INTO DATEOUVERTURE (DERNIEREOUVERTURE) VALUES ('" + QDate::currentDate().toString("yyyy-MM-dd") + "')");

            if(query.exec()) {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : Ajout de la date du jour dans la table DATEOUVERTURE réussie");
            } else {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur l'ajout de la date du jour dans la table DATEOUVERTURE : " + query.lastError().text());
                QMessageBox::critical(NULL, "ERREUR", "Erreur sur l'ajout de la date du jour dans la table DATEOUVERTURE : " + query.lastError().text());
            }

            query.prepare("CREATE TABLE HISTORIQUE"
                          "("
                          "NOM          TEXT NOT NULL,"
                          "SAISON       TEXT NOT NULL,"
                          "EPISODE      TEXT NOT NULL,"
                          "DATEAJOUT    DATE NOT NULL"
                          ")");

            if(query.exec()) {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : Création de la table HISTORIQUE réussie");
            } else {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur la création de la table HISTORIQUE : " + query.lastError().text());
                QMessageBox::critical(NULL, "ERREUR", "Erreur sur la création de la table HISTORIQUE : " + query.lastError().text());
            }

            query.prepare("CREATE TABLE `FS#FICHE_SERIE` ("
                          "`FS#ID`      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                          "`FSNOM`      TEXT NOT NULL UNIQUE,"
                          "`FSIMAGE`	TEXT,"
                          "`FSWIKI`     TEXT,"
                          "`FSADDICTED` TEXT"
                          ");");

            if(query.exec()) {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : Création de la table FS#FICHE_SERIE réussie");
            } else {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur la création de la table FS#FICHE_SERIE : " + query.lastError().text());
                QMessageBox::critical(NULL, "ERREUR", "Erreur sur la création de la table FS#FICHE_SERIE : " + query.lastError().text());
            }

            query.prepare("CREATE TABLE `SA#SAISON` ("
                          "`SAFS#ID`            INTEGER NOT NULL UNIQUE,"
                          "`SASAISON`           TEXT NOT NULL,"
                          "`SANBEPISODE`        TEXT NOT NULL,"
                          "`SAEPISODECOURANT`	TEXT NOT NULL,"
                          "`SADATESORTIE`       TEXT NOT NULL,"
                          "`SAJOURSORTIE`       TEXT NOT NULL,"
                          "`SADATEMODIF`        INTEGER NOT NULL,"
                          "`SAWIKI`             INTEGER,"
                          "FOREIGN KEY(`SAFS#ID`) REFERENCES `FS#FICHE_SERIE.FS#ID`"
                          ");");

            if(query.exec()) {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : Création de la table SA#SAISON réussie");
            } else {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur la création de la table SA#SAISON : " + query.lastError().text());
                QMessageBox::critical(NULL, "ERREUR", "Erreur sur la création de la table SA#SAISON : " + query.lastError().text());
            }
        } else {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR BDD non ouverte : " + db.lastError().text());
            QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
        }
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("./data/bdd.db");

        if(!db.open()) {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : Erreur BDD non ouverte : " + db.lastError().text());
            QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
        } else {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : Ouverture de la base de donnee");
            QSqlQuery query(db);

            if(!query.exec("SELECT * FROM " + FICHE_SERIE_TABLE)) {
                query.prepare("CREATE TABLE `FS#FICHE_SERIE` ("
                              "`FS#ID`      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                              "`FSNOM`      TEXT NOT NULL UNIQUE,"
                              "`FSIMAGE`	TEXT,"
                              "`FSWIKI`     TEXT,"
                              "`FSADDICTED` TEXT"
                              ")");

                if(query.exec()) {
                    log->ecrire("BaseDeDonnees::BaseDeDonnees() : Création de la table FS#FICHE_SERIE réussie");
                } else {
                    log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur la création de la table FS#FICHE_SERIE : " + query.lastError().text());
                    QMessageBox::critical(NULL, "ERREUR", "Erreur sur la création de la table FS#FICHE_SERIE : " + query.lastError().text());
                }
            }

            if(!query.exec("SELECT * FROM " + SAISON_TABLE)) {
                query.prepare("CREATE TABLE `SA#SAISON` ("
                              "`SAFS#ID`            INTEGER NOT NULL UNIQUE,"
                              "`SASAISON`           TEXT NOT NULL,"
                              "`SANBEPISODE`        TEXT NOT NULL,"
                              "`SAEPISODECOURANT`	TEXT NOT NULL,"
                              "`SADATESORTIE`       TEXT NOT NULL,"
                              "`SAJOURSORTIE`       TEXT NOT NULL,"
                              "`SADATEMODIF`        INTEGER NOT NULL,"
                              "`SAWIKI`             TEXT,"
                              "`SADOUBLEEPISODE     TEXT`,"
                              "FOREIGN KEY(`SAFS#ID`) REFERENCES `FS#FICHE_SERIE.FS#ID`"
                              ")");

                if(query.exec()) {
                    log->ecrire("BaseDeDonnees::BaseDeDonnees() : Création de la table SA#SAISON réussie");
                } else {
                    log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur la création de la table SA#SAISON : " + query.lastError().text());
                    QMessageBox::critical(NULL, "ERREUR", "Erreur sur la création de la table SA#SAISON : " + query.lastError().text());
                }
            }
#ifndef QT_DEBUG
            if(query.exec("SELECT * FROM SERIE")) {
                QList<QString> champs;
                QList<QString> conditions;
                QList<QString> ordres;
                champs.append("NOM");
                champs.append("SAISON");
                champs.append("NBEPISODE");
                champs.append("EPISODECOURANT");
                champs.append("DATESORTIE");
                champs.append("JOURSORTIE");
                champs.append("DATEMODIF");
                champs.append("WIKI");
                QList<QMap<QString,QString> > listeSerie = requeteSelect(champs, "SERIE", conditions, ordres);
                for(int i = 0; i < listeSerie.count(); i++) {
                    QMap<QString,QString> map = listeSerie.at(i);
                    champs.clear();
                    QList<QString> valeurs;
                    QList<QString> ordres;
                    // Création de la fiche Série
                    champs.append(FICHE_SERIE_NOM);
                    valeurs.append(map.value("NOM"));
                    requeteInsert(champs, valeurs, FICHE_SERIE_TABLE);
                    // Récupération de l'ID
                    champs.clear();
                    conditions.clear();
                    champs.append(FICHE_SERIE_ID);
                    conditions.append(FICHE_SERIE_NOM + " = '" + map.value("NOM") + "'");
                    QString id = requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres).at(0).value(FICHE_SERIE_ID);
                    // Création de la saison
                    champs.clear();
                    valeurs.clear();

                    champs.append(SAISON_ID);
                    valeurs.append(id);
                    champs.append(SAISON_SAISON);
                    valeurs.append(map.value("SAISON"));
                    champs.append(SAISON_NB_EPISODE);
                    valeurs.append(map.value("NBEPISODE"));
                    champs.append(SAISON_EPISODE_COURANT);
                    valeurs.append(map.value("EPISODECOURANT"));
                    champs.append(SAISON_DATE_SORTIE);
                    valeurs.append(map.value("DATESORTIE"));
                    champs.append(SAISON_JOUR_SORTIE);
                    valeurs.append(map.value("JOURSORTIE"));
                    champs.append(SAISON_DATE_MODIF);
                    valeurs.append(map.value("DATEMODIF"));
                    champs.append(SAISON_WIKI);
                    valeurs.append(map.value("WIKI"));

                    requeteInsert(champs, valeurs, SAISON_TABLE);
                }
                db.open();
                query.exec("DROP TABLE SERIE");
            }
#endif
        }
    }
    db.close();
    log->ecrire("BaseDeDonnees::BaseDeDonnees() : Fermeture de la base de donnee");
}

BaseDeDonnees::~BaseDeDonnees(){
    //DO NOTHING
}

QList<QMap<QString, QString> > BaseDeDonnees::requeteSelect(QList<QString> champs, QString table, QList<QString> jointures, QList<QString> conditions, QList<QString> ordres) {
    QString requete = "SELECT ";
    QList<QMap<QString, QString> > liste;
    foreach (QString champ, champs) {
        requete.append(champ + ", ");
    }
    requete.replace(requete.length()-2, 2, " ");
    requete.append("FROM " + table + " ");

    for(int i = 0; i < jointures.count(); i++) {
        requete.append(jointures.at(i) + " ");
    }

    for (int i = 0; i < conditions.count(); i++) {
        if(i == 0) {
            requete.append("WHERE " + conditions.at(i) + " ");
        } else {
            requete.append("AND " + conditions.at(i) + " ");
        }
    }

    if(!ordres.isEmpty()) {
        requete.append("ORDER BY ");
        for(int i = 0; i < ordres.count(); i++) {
            requete.append(ordres.at(i) + ", ");
        }
        requete.replace(requete.length()-2, 2, " ");
    }

    if(db.open()) {
        log->ecrire("BaseDeDonnees::requeteSelect() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        log->ecrire("BaseDeDonnees::requeteSelect() : Requete SQL : " + requete);
        if(query.exec(requete)) {
            while(query.next()) {
                QMap<QString, QString> map;
                for(int i = 0; i < champs.count(); i++) {
                    map[champs.at(i)] = query.value(i).toString();
                }
                liste.append(map);
            }
        } else {
            log->ecrire("BaseDeDonnees::requeteSelect() : Erreur d'éxecution de la requête : " + query.lastError().text());
            QMessageBox::critical(NULL, "Erreur", "Erreur d'éxecution de la requête : " + query.lastError().text());
        }

    } else {
        log->ecrire("BaseDeDonnees::requeteSelect() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }
    db.close();
    log->ecrire("BaseDeDonnees::requeteSelect() : Fermeture de la base de donnée");

    return liste;
}

QList<QMap<QString, QString> > BaseDeDonnees::requeteSelect(QList<QString> champs, QString table, QList<QString> conditions, QList<QString> ordres) {
    QString requete = "SELECT ";
    QList<QMap<QString, QString> > liste;
    foreach (QString champ, champs) {
        requete.append(champ + ", ");
    }
    requete.replace(requete.length()-2, 2, " ");
    requete.append("FROM " + table + " ");
    for (int i = 0; i < conditions.count(); i++) {
        if(i == 0) {
            requete.append("WHERE " + conditions.at(i) + " ");
        } else {
            requete.append("AND " + conditions.at(i) + " ");
        }
    }

    if(!ordres.isEmpty()) {
        requete.append("ORDER BY ");
        for(int i = 0; i < ordres.count(); i++) {
            requete.append(ordres.at(i) + ", ");
        }
        requete.replace(requete.length()-2, 2, " ");
    }

    if(db.open()) {
        log->ecrire("BaseDeDonnees::requeteSelect() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        log->ecrire("BaseDeDonnees::requeteSelect() : Requete SQL : " + requete);
        if(query.exec(requete)) {
            while(query.next()) {
                QMap<QString, QString> map;
                for(int i = 0; i < champs.count(); i++) {
                    map[champs.at(i)] = query.value(i).toString();
                }
                liste.append(map);
            }
        } else {
            log->ecrire("BaseDeDonnees::requeteSelect() : Erreur d'éxecution de la requête : " + query.lastError().text());
            QMessageBox::critical(NULL, "Erreur", "Erreur d'éxecution de la requête : " + query.lastError().text());
        }

    } else {
        log->ecrire("BaseDeDonnees::requeteSelect() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }
    db.close();
    log->ecrire("BaseDeDonnees::requeteSelect() : Fermeture de la base de donnée");

    return liste;
}

bool BaseDeDonnees::requeteInsert(QList<QString> champs, QList<QString> valeurs, QString table) {
    QString requete = "INSERT INTO " + table + " (";
    bool res = false;
    foreach (QString champ, champs) {
        requete.append(champ + ", ");
    }
    requete.replace(requete.length()-2, 2, " ");
    requete.append(") VALUES (");
    foreach (QString valeur, valeurs) {
        requete.append("'" + valeur + "', ");
    }
    requete.replace(requete.length()-2, 2, ")");

    //QMessageBox::information(NULL, "", requete);

    if(db.open()) {
        log->ecrire("BaseDeDonnees::requeteInsert() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        log->ecrire("BaseDeDonnees::requeteInsert() : Requete SQL : " + requete);
        res = query.exec(requete);
        if(!res) {
            log->ecrire("BaseDeDonnees::requeteUpdate() : ERREUR - " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::requeteInsert() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }
    db.close();
    log->ecrire("BaseDeDonnees::requeteInsert() : Fermeture de la base de donnée");
    return res;
}

bool BaseDeDonnees::requeteUpdate(QList<QString> champs, QString table, QList<QString> conditions) {
    QString requete = "UPDATE " + table + " SET ";
    bool res = false;
    foreach (QString champ, champs) {
        requete.append(champ + ", ");
    }
    requete.replace(requete.length()-2, 2, " ");
    for (int i = 0; i < conditions.count(); i++) {
        if(i == 0) {
            requete.append("WHERE " + conditions.at(i) + " ");
        } else {
            requete.append("AND " + conditions.at(i) + " ");
        }
    }

    //QMessageBox::information(NULL, "", requete);

    if(db.open()) {
        log->ecrire("BaseDeDonnees::requeteUpdate() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        log->ecrire("BaseDeDonnees::requeteUpdate() : Requete SQL : " + requete);
        //return query.exec(requete);
        res = query.exec(requete);
        if(!res) {
            log->ecrire("BaseDeDonnees::requeteUpdate() : ERREUR - " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::requeteUpdate() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }
    db.close();
    log->ecrire("BaseDeDonnees::requeteUpdate() : Fermeture de la base de donnée");
    return res;
}

bool BaseDeDonnees::requeteDelete(QString table, QList<QString> conditions) {
    QString requete = "DELETE FROM " + table + " ";
    bool res = false;
    for (int i = 0; i < conditions.count(); i++) {
        if(i == 0) {
            requete.append("WHERE " + conditions.at(i) + " ");
        } else {
            requete.append("AND " + conditions.at(i) + " ");
        }
    }

    //QMessageBox::information(NULL, "", requete);

    if(db.open()) {
        log->ecrire("BaseDeDonnees::requeteDelete() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        log->ecrire("BaseDeDonnees::requeteDelete() : Requete SQL : " + requete);
        res = query.exec(requete);
        if(!res) {
            log->ecrire("BaseDeDonnees::requeteDelete() : ERREUR - " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::requeteDelete() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }
    db.close();
    log->ecrire("BaseDeDonnees::requeteDelete() : Fermeture de la base de donnée");
    return res;
}

QDate BaseDeDonnees::derniereOuvertureBDD() {
    if(db.open()) {
        log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Ouverture de la base de donnée");

        QSqlQuery query(db);
        QString requete = "SELECT * FROM DATEOUVERTURE";
        log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Requete SQL : " + requete);
        if(query.exec(requete)) {
            query.next();
            if(query.value(0).toString().contains(QRegExp("^[0-9]{4}-[0-9]{2}-[0-9]{2}$"))) {
                return methodeDiverses.stringToDate(query.value(0).toString());
            } else {
                log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Il y a eu un problème sur la date de dernière ouverture de la base de donnée. Ajout de la date de la veille.");
                QMessageBox::warning(NULL, "Atttention", "Il y a eu un problème sur la date de dernière ouverture de la base de donnée.\n"
                                                             "Ajout de la date de la veille.");
                query.prepare("DELETE FROM DATEOUVERTURE");

                if(query.exec()) {
                    query.prepare("INSERT INTO DATEOUVERTURE (DERNIEREOUVERTURE) VALUES ('" + QDate::currentDate().addDays(-1).toString("yyyy-MM-dd") + "')");

                    if(query.exec()) {
                        log->ecrire("BaseDeDonnees::BaseDeDonnees() : La date de la veille a été ajouté dans la base de donnée");
                        QMessageBox::information(NULL, "Information", "La date de la veille a été ajouté dans la base de donnée");
                        return QDate::currentDate().addDays(-1);
                    } else {
                        log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur l'ajout de la date du jour dans la table DATEOUVERTURE : " + query.lastError().text());
                        QMessageBox::critical(NULL, "ERREUR", "Erreur sur l'ajout de la date du jour dans la table DATEOUVERTURE : " + query.lastError().text());
                    }
                } else {
                    log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : L'entrée saisie n'a pas pu être supprimé");
                    QMessageBox::critical(NULL, "ERREUR", "Erreur l'entrée saisie n'a pas pu être supprimé");
                }
            }
        } else {
            log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Erreur d'éxecution de la requête : " + query.lastError().text());
            QMessageBox::critical(NULL, "Erreur", "Erreur d'éxecution de la requête : " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }

    db.close();
    log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Fermeture de la base de donnée");
}

void BaseDeDonnees::majDerniereOuvertureBDD() {
    if(db.open()) {
        log->ecrire("BaseDeDonnees::majDerniereOuvertureBDD() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        QString requete = "UPDATE DATEOUVERTURE SET DERNIEREOUVERTURE = '" + QDate::currentDate().toString("yyyy-MM-dd") + "'";
        log->ecrire("BaseDeDonnees::majDerniereOuvertureBDD() : Requete SQL : " + requete);
        if(query.exec(requete)) {
            log->ecrire("BaseDeDonnees::majDerniereOuvertureBDD() : La date d'ouverture de la BDD a été mise à jour");
        } else {
            log->ecrire("BaseDeDonnees::majDerniereOuvertureBDD() : ERREUR la date d'ouverture de la BDD n'a pas été mise à jour");
            QMessageBox::critical(NULL, "Erreur", "La date d'ouverture de la BDD n'a pas été mise à jour");
        }
    } else {
        log->ecrire("BaseDeDonnees::majDerniereOuvertureBDD() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }
    db.close();
    log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Fermeture de la base de donnée");
}
