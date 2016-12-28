#include "basededonnees.h"

BaseDeDonnees::BaseDeDonnees() {
    log = new Log("BaseDeDonnees");
    QFile *file = new QFile("bdd.db");
    if(!file->exists()) {
        log->ecrire("BaseDeDonnees::BaseDeDonnees() : Creation de la base de donnée");
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("bdd.db");

        if(db.open()) {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : Ouverture de la base de donnée");
            QSqlQuery query(db);
            query.prepare("CREATE TABLE SERIE"
                          "("
                          "NOM TEXT PRIMARY KEY NOT NULL,"
                          "SAISON TEXT NOT NULL,"
                          "NBEPISODE TEXT NOT NULL,"
                          "EPISODECOURANT TEXT NOT NULL,"
                          "DATESORTIE DATE NOT NULL,"
                          "JOURSORTIE TEXT,"
                          "DATEMODIF DATE NOT NULL,"
                          "WIKI TEXT"
                          ")");
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : Requete SQL : CREATE TABLE SERIE (NOM TEXT PRIMARY KEY NOT NULL,SAISON TEXT NOT NULL,NBEPISODE TEXT NOT NULL,EPISODECOURANT TEXT NOT NULL,DATESORTIE DATE NOT NULL,JOURSORTIE TEXT NOT NULL,DATEMODIF DATE NOT NULL,WIKI TEXT);");

            if(query.exec()) {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : Création de la table SERIE réussie");
            } else {
                log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR sur la création de la table SERIE : " + query.lastError().text());
                QMessageBox::critical(NULL, "ERREUR", "Erreur sur la création de la table SERIE : " + query.lastError().text());
            }

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
        } else {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : ERREUR BDD non ouverte : " + db.lastError().text());
            QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
        }
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("bdd.db");

        if(!db.open()) {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : Erreur BDD non ouverte : " + db.lastError().text());
            QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
        } else {
            log->ecrire("BaseDeDonnees::BaseDeDonnees() : Ouverture de la base de donnee");
        }
    }
    db.close();
    log->ecrire("BaseDeDonnees::BaseDeDonnees() : Fermeture de la base de donnee");
}

BaseDeDonnees::~BaseDeDonnees(){
    QFile *file = new QFile("bdd.db");
    file->remove();
    log->ecrire("BaseDeDonnees::~BaseDeDonnees() : Suppression de la base de donnée");
}

void BaseDeDonnees::ajouter(QString nom, int saison, int nbEpisode, int jourSortie, QDate dateSortie, QString wiki) {
    if(db.open()) {
        log->ecrire("BaseDeDonnees::ajouter() : Ouverture de la base de donnée");
        QString s_saison = "";
        QString s_nbEpisode = "";

        if(saison > 0 && saison < 10) {
            s_saison.append("0" + QString::number(saison));
        } else {
            s_saison.append(QString::number(saison));
        }

        if(nbEpisode > 0 && nbEpisode < 10) {
            s_nbEpisode.append("0" + QString::number(nbEpisode));
        } else {
            s_nbEpisode.append(QString::number(nbEpisode));
        }

        QSqlQuery query(db);
        query.prepare("INSERT INTO SERIE(NOM,SAISON,NBEPISODE,EPISODECOURANT,DATESORTIE,JOURSORTIE,DATEMODIF, WIKI) "
                      "VALUES (:nom, :saison, :nbepisode, '01', :datesortie, :joursortie, :datemodif, :wiki)");
        query.bindValue(":nom", nom);
        query.bindValue(":saison", s_saison);
        query.bindValue(":nbepisode", s_nbEpisode);
        query.bindValue(":datesortie", dateSortie.toString("yyyy-MM-dd"));
        query.bindValue(":joursortie", jourSortie);
        query.bindValue(":datemodif", dateSortie.addDays(-7).toString("yyyy-MM-dd"));
        query.bindValue(":wiki",wiki);

        log->ecrire("BaseDeDonnees::ajouter() : Requete SQL  : INSERT INTO SERIE(NOM,SAISON,NBEPISODE,EPISODECOURANT,DATESORTIE,JOURSORTIE,DATEMODIF,WIKI) VALUES ('" +  nom + "','" + s_saison + "','" + s_nbEpisode + "','01', " + dateSortie.toString("yyyy-MM-dd") + "','" + jourSortie + "','" + dateSortie.addDays(-7).toString("yyyy-MM-dd") + "', '" + wiki + "')");

        if(query.exec()) {
            log->ecrire("BaseDeDonnees::ajouter() : La série a été ajoutée");
            QMessageBox::information(NULL, "Ajout réussi", "La série a été ajoutée !");
        } else {
            log->ecrire("BaseDeDonnees::ajouter() : La série n'a pas été ajouté : " + query.lastError().text());
            QMessageBox::critical(NULL, "ERREUR", "La série n'a pas été ajouté : " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::ajouter() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
    }

    db.close();
    log->ecrire("BaseDeDonnees::ajouter() : Fermeture de la base de donnée");
}

void BaseDeDonnees::reporter(QString nom, QDate dateModif) {
    if(db.open()) {
        log->ecrire("BaseDeDonnees::reporter() : Ouverture de la base de donnée");

        QSqlQuery query(db);
        query.prepare("UPDATE SERIE "
                      "SET DATEMODIF = :datemodif "
                      "WHERE NOM =  :nom");
        query.bindValue(":datemodif", dateModif.toString("yyyy-MM-dd"));
        query.bindValue(":nom", nom);

        log->ecrire("BaseDeDonnees::reporter() : Requete SQL : UPDATE SERIE SET DATEMODIF = '" + dateModif.toString("yyyy-MM-dd") + "' WHERE NOM = '" + nom + "'");

        if(query.exec()) {
            log->ecrire("BaseDeDonnees::reporter() : La série a été reportée");
            QMessageBox::information(NULL, "Modification réussi", "La série a été reportée");
        } else {
            log->ecrire("BaseDeDonnees::reporter : La série n'a pas été reporté : " + query.lastError().text());
            QMessageBox::critical(NULL,"Erreur", "La série n'a pas été reporté : " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::reporter() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
    }

    db.close();
    log->ecrire("BaseDeDonnees::reporter() : Fermeture de la base de donnée");
}

void BaseDeDonnees::modifier(QString nom, int saison, int nbEpisode, int episodeCourant, int jourSortie, QDate date, QString wiki, QDate dateModif, bool message) {
    QString s_saison = "";
    QString s_nbEpisode = "";
    QString s_episodeCourant = "";

    if(db.open()) {
        log->ecrire("BaseDeDonnees::modifier() : Ouverture de la base de donnée");

        if(saison > 0 && saison < 10) {
            s_saison.append("0" + QString::number(saison));
        } else {
            s_saison.append(QString::number(saison));
        }

        if(episodeCourant > 0 && episodeCourant < 10) {
            s_episodeCourant.append("0" + QString::number(episodeCourant));
        } else {
            s_episodeCourant.append(QString::number(episodeCourant));
        }

        if(nbEpisode > 0 && nbEpisode < 10) {
            s_nbEpisode.append("0" + QString::number(nbEpisode));
        } else {
            s_nbEpisode.append(QString::number(nbEpisode));
        }

        QSqlQuery query(db);
        query.prepare("UPDATE SERIE "
                      "SET SAISON = :saison,"
                      "NBEPISODE = :nbepisode,"
                      "EPISODECOURANT = :episodecourant,"
                      "JOURSORTIE = :joursortie,"
                      "DATESORTIE = :date,"
                      "WIKI = :wiki,"
                      "DATEMODIF = :datemodif "
                      "WHERE NOM = :nom");

        query.bindValue(":datemodif", date);
        query.bindValue(":nom", nom);
        query.bindValue(":saison", s_saison);
        query.bindValue(":nbepisode", s_nbEpisode);
        query.bindValue(":episodecourant", s_episodeCourant);
        query.bindValue(":joursortie", QString::number(jourSortie));
        query.bindValue(":date", date);
        query.bindValue(":wiki",wiki);
        query.bindValue(":datemodif",dateModif);

        log->ecrire("BaseDeDonnees::modifier() : Requete SQL : UPDATE SERIE SET SAISON = '" + s_saison + "', NBEPISODE = '" + s_nbEpisode + "', EPISODECOURANT = '" + s_episodeCourant + "', JOURSORTIE = '" + QString::number(jourSortie) + "', DATEMODIF = '" + date.toString("yyyy-MM-dd") + "', WIKI = '" + wiki + "', DATEMODIF = '" + dateModif.toString("yyyy-MM-DD") + "' WHERE NOM = '" + nom + "'");

        if(query.exec()) {
            log->ecrire("BaseDeDonnees::modifier() : La serie a ete modifie");
            if(message) {
                QMessageBox::information(NULL,"Modification réussi","La série a été modifié !");
            }
        } else {
            log->ecrire("BaseDeDonnees::modifier() : La sérié n'a pas été modifié : " + query.lastError().text());
            if(message) {
                QMessageBox::critical(NULL,"Erreur Modification", "La sérié n'a pas été modifié : " + query.lastError().text());
            }
        }

    } else {
        log->ecrire("BaseDeDonnees::modifier() : Erreur BDD non ouverte : " + db.lastError().text());
        if(message) {
            QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
        }
    }

    db.close();
    log->ecrire("BaseDeDonnees::modifier() : Fermeture de la base de donnée");
}

void BaseDeDonnees::supprimer(QString nom, bool msgBox) {
    if(db.open()) {
        log->ecrire("BaseDeDonnees::supprimer() : Ouverture de la base de donnée");

        QSqlQuery query(db);
        query.prepare("DELETE FROM SERIE "
                      "WHERE NOM = :nom");
        query.bindValue(":nom", nom);

        log->ecrire("BaseDeDonnees::supprimer() : Requete SQL : DELETE FROM SERIE WHERE NOM = " + nom);
        if(query.exec()) {
            log->ecrire("BaseDeDonnees::supprimer() : La série a été supprimée");
            if(msgBox) {
                QMessageBox::information(NULL, "Suppression effectuée", "La série a été supprimée");
            }
        } else {
            log->ecrire("BaseDeDonnees::supprimer() : La série n'a pas été supprimé : " + query.lastError().text());
            if(msgBox) {
                QMessageBox::information(NULL, "Suppression effectuée", "La série n'a pas été supprimé : " + query.lastError().text());
            }
        }
    } else {
        log->ecrire("BaseDeDonnees::supprimer() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
    }

    db.close();
    log->ecrire("BaseDeDonnees::supprimer() : Fermeture de la base de donnée");
}

QList<QMap<QString, QString> > BaseDeDonnees::requeteListe(QString requete) {
    QList<QMap<QString, QString> > liste;
    if(db.open()) {
        log->ecrire("BaseDeDonnees::requeteListe() : Ouverture de la base de donnée");

        QSqlQuery query(db);
        log->ecrire("BaseDeDonnees::requeteListe() : Requete SQL : " + requete);
        if(query.exec(requete)) {
            while(query.next()) {
                QMap<QString, QString> sousListe;
                sousListe["NOM"] = query.value(0).toString();
                sousListe["SAISON"] = query.value(1).toString();
                sousListe["NBEPISODE"] = query.value(2).toString();
                sousListe["EPISODECOURANT"] = query.value(3).toString();
                sousListe["DATESORTIE"] = query.value(4).toString();
                sousListe["JOURSORTIE"] = query.value(5).toString();
                sousListe["DATEMODIF"] = query.value(6).toString();
                sousListe["WIKI"] = query.value(7).toString();
                liste.append(sousListe);
            }
        } else {
            log->ecrire("BaseDeDonnees::requeteListe() : Erreur d'éxecution de la requête : " + query.lastError().text());
            QMessageBox::critical(NULL, "ERREUR", "Erreur d'éxecution de la requête : " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::requeteListe() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "ERREUR", "Erreur BDD non ouverte : " + db.lastError().text());
    }

    db.close();
    log->ecrire("BaseDeDonnees::requeteListe() : Fermeture de la base de données");
    return liste;
}

QMap<QString, QString> BaseDeDonnees::requete(QString requete) {
    QMap<QString, QString> liste;
    if(db.open()) {
        log->ecrire("BaseDeDonnees::requete() : Ouverture de la base de donnée");

        QSqlQuery query(db);
        log->ecrire("BaseDeDonnees::requete() : Requete SQL : " + requete);

        if(query.exec(requete)) {
            query.next();
            liste["NOM"] = query.value(0).toString();
            liste["SAISON"] = query.value(1).toString();
            liste["NBEPISODE"] = query.value(2).toString();
            liste["JOURSORTIE"] = query.value(3).toString();
            liste["EPISODECOURANT"] = query.value(4).toString();
            liste["DATESORTIE"] = query.value(5).toString();
            liste["WIKI"] = query.value(6).toString();
            liste["DATEMODIF"] = query.value(7).toString();
        } else {
            log->ecrire("BaseDeDonnees::requete() : Erreur d'éxecution de la requête : " + query.lastError().text());
            QMessageBox::critical(NULL, "Erreur", "Erreur d'éxecution de la requête : " + query.lastError().text());
        }
    } else {
        log->ecrire("BaseDeDonnees::requete() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }

    db.close();
    log->ecrire("BaseDeDonnees::requete() : Fermeture de la base de donnée");
    return liste;
}

QDate BaseDeDonnees::derniereOuvertureBDD() {
    if(db.open()) {
        log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Ouverture de la base de donnée");

        QSqlQuery query(db);
        query.prepare("SELECT * FROM DATEOUVERTURE");
        log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Requete SQL :  : SELECT * FROM DATEOUVERTURE");
        if(query.exec()) {
            query.next();
            if(query.value(0).toString().contains(QRegExp("^[0-9]{4}-[0-9]{2}-[0-9]{2}$"))) {
                QString res = query.value(0).toString();
                QStringList list = res.split("-");
                QString a = list.at(0);
                QString m = list.at(1);
                QString j = list.at(2);
                return QDate(a.toInt(), m.toInt(), j.toInt());
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
        log->ecrire("BaseDeDonnees::derniereOuvertureBDD() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        query.prepare("UPDATE DATEOUVERTURE SET DERNIEREOUVERTURE = '" + QDate::currentDate().toString("yyyy-MM-dd") + "'");
        log->ecrire("BaseDeDonnees::majDerniereOuvertureBDD() : Requete SQL : UPDATE DATEOUVERTURE SET DERNIEREOUVERTURE = '" + QDate::currentDate().toString("yyyy-MM-dd") + "'");
        if(query.exec()) {
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
