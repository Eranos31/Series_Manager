#include "basededonnees.h"

BaseDeDonnees::BaseDeDonnees(QString chemin) {
    QString nomBDD = chemin + "/bdd.db";
    QFile *file = new QFile(nomBDD);
    if(!file->exists()) {
        // La BDD n'existe pas
        QString messageLog = "BaseDeDonnees::BaseDeDonnees() : Le fichier de base de données n'existe pas.\nLe programme va s'arreter.";
        QMessageBox::critical(NULL, "BaseDeDonnees", messageLog);
        MethodeDiverses::ecrireLog(messageLog);
        throw new QException();
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(nomBDD);
        if(!db.open()) {
            QString messageLog = "BaseDeDonnees::BaseDeDonnees() : La base de données n'a pu être ouverte.\nLe programme va s'arreter.";
            QMessageBox::critical(NULL, "", messageLog);
            MethodeDiverses::ecrireLog(messageLog + " " + db.lastError().text());
            throw new QException();
        }
        db.close();
    }
}

BaseDeDonnees::Retour BaseDeDonnees::requeteSelect(QStringList champs, QString table, QStringList jointures, QStringList conditions, QStringList ordres) {
    return execRequete(getRequeteSelect(champs, table, jointures, conditions, ordres));
}


BaseDeDonnees::Retour BaseDeDonnees::requeteInsert(QStringList champs, QStringList valeurs, QString table) {
    return execRequete(getRequeteInsert(champs, valeurs, table));
}

BaseDeDonnees::Retour BaseDeDonnees::requeteUpdate(QStringList champs, QString table, QStringList conditions) {
    return execRequete(getRequeteUpdate(champs, table, conditions));
}

BaseDeDonnees::Retour BaseDeDonnees::requeteDelete(QString table, QStringList conditions) {
    return execRequete(getRequeteDelete(table, conditions));
}

QString BaseDeDonnees::entreQuotes(QString champs) {
    return "'" + champs + "'";
}

QString BaseDeDonnees::entreParentheses(QString champs) {
    return "(" + champs + ")";
}


QString BaseDeDonnees::getRequeteSelect(QStringList champs, QString table, QStringList jointures, QStringList conditions, QStringList ordres) {
    QString requete = SELECT;
    requete.append(champs.join(VIRGULE));
    requete.append(FROM + table);
    if(!jointures.isEmpty())
        requete.append(jointures.join(" "));
    if(!conditions.isEmpty())
        requete.append(WHERE + conditions.join(AND));
    if(!ordres.isEmpty())
        requete.append(ORDER_BY + ordres.join(VIRGULE));
    return requete;
}

QString BaseDeDonnees::getRequeteUpdate(QStringList champs, QString table, QStringList conditions) {
    QString requete = UPDATE + table;
    requete.append(SET + champs.join(VIRGULE));
    if(!conditions.isEmpty())
        requete.append(WHERE + conditions.join(AND));
    return requete;
}

QString BaseDeDonnees::getRequeteDelete(QString table, QStringList conditions) {
    QString requete = DELETE + table;
    if(!conditions.isEmpty())
        requete.append(WHERE + conditions.join(AND));
    return requete;
}

QString BaseDeDonnees::getRequeteInsert(QStringList champs, QStringList valeurs, QString table) {
    QString requete = INSERT + table;
    requete.append(entreParentheses(champs.join(VIRGULE)));
    requete.append(VALUES + entreParentheses(valeurs.join(VIRGULE)));
    return requete;
}

BaseDeDonnees::Retour BaseDeDonnees::execRequete(QString requete) {
    BaseDeDonnees::Retour retour;
    retour.requete = requete;
    if(db.open()) {
        QSqlQuery query(db);
        if(query.exec(requete)) {
            if(query.isSelect()) {
                while(query.next()) {
                    QMap<QString, QString> map;
                    for(int i = 0; i < query.record().count(); i++) {
                        if(query.record().fieldName(i).startsWith(MAX) || query.record().fieldName(i).startsWith(MIN) || query.record().fieldName(i).startsWith(SUM) || query.record().fieldName(i).startsWith(AVG)) {
                            map[query.record().fieldName(i)] = query.value(i).toString();
                        } else {
                            map["`" + query.record().fieldName(i) + "`"] = query.value(i).toString();
                        }
                    }
                    retour.liste.append(map);
                }
            } else {
                if(retour.requete.startsWith(UPDATE)) {
                    retour.info = QString::number(query.numRowsAffected()) + " ligne(s) modifiée(s)";
                } else if (retour.requete.startsWith(DELETE)) {
                    retour.info = QString::number(query.numRowsAffected()) + " ligne(s) supprimée(s)";
                } else if (retour.requete.startsWith(INSERT)) {
                    retour.info = QString::number(query.numRowsAffected()) + " ligne(s) ajoutée(s)";
                }
            }
            retour.reussi = true;
        } else {
            retour.reussi = false;
            retour.erreur = query.lastError().text();
        }
    } else {
        retour.reussi = false;
        retour.erreur = db.lastError().text();
    }

    return retour;
}

QDate BaseDeDonnees::derniereOuvertureBDD() {
    if(db.open()) {
        MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : Ouverture de la base de donnée");

        QSqlQuery query(db);
        QString requete = "SELECT * FROM DATEOUVERTURE";
        MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : Requete SQL : " + requete);
        if(query.exec(requete)) {
            query.next();
            if(query.value(0).toString().contains(QRegExp("^[0-9]{4}-[0-9]{2}-[0-9]{2}$"))) {
                return methodeDiverses.stringToDate(query.value(0).toString());
            } else {
                MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : Il y a eu un problème sur la date de dernière ouverture de la base de donnée. Ajout de la date de la veille.");
                QMessageBox::warning(NULL, "Atttention", "Il y a eu un problème sur la date de dernière ouverture de la base de donnée.\n"
                                                             "Ajout de la date de la veille.");
                query.prepare("DELETE FROM DATEOUVERTURE");

                if(query.exec()) {
                    query.prepare("INSERT INTO DATEOUVERTURE (DERNIEREOUVERTURE) VALUES ('" + QDate::currentDate().addDays(-1).toString("yyyy-MM-dd") + "')");

                    if(query.exec()) {
                        MethodeDiverses::ecrireLog("BaseDeDonnees::BaseDeDonnees() : La date de la veille a été ajouté dans la base de donnée");
                        QMessageBox::information(NULL, "Information", "La date de la veille a été ajouté dans la base de donnée");
                        return QDate::currentDate().addDays(-1);
                    } else {
                        MethodeDiverses::ecrireLog("BaseDeDonnees::BaseDeDonnees() : ERREUR sur l'ajout de la date du jour dans la table DATEOUVERTURE : " + query.lastError().text());
                        QMessageBox::critical(NULL, "ERREUR", "Erreur sur l'ajout de la date du jour dans la table DATEOUVERTURE : " + query.lastError().text());
                    }
                } else {
                    MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : L'entrée saisie n'a pas pu être supprimé");
                    QMessageBox::critical(NULL, "ERREUR", "Erreur l'entrée saisie n'a pas pu être supprimé");
                }
            }
        } else {
            MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : Erreur d'éxecution de la requête : " + query.lastError().text());
            QMessageBox::critical(NULL, "Erreur", "Erreur d'éxecution de la requête : " + query.lastError().text());
        }
    } else {
        MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }

    db.close();
    MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : Fermeture de la base de donnée");
}

void BaseDeDonnees::majDerniereOuvertureBDD() {
    if(db.open()) {
        MethodeDiverses::ecrireLog("BaseDeDonnees::majDerniereOuvertureBDD() : Ouverture de la base de donnée");
        QSqlQuery query(db);
        QString requete = "UPDATE DATEOUVERTURE SET DERNIEREOUVERTURE = '" + QDate::currentDate().toString("yyyy-MM-dd") + "'";
        MethodeDiverses::ecrireLog("BaseDeDonnees::majDerniereOuvertureBDD() : Requete SQL : " + requete);
        if(query.exec(requete)) {
            MethodeDiverses::ecrireLog("BaseDeDonnees::majDerniereOuvertureBDD() : La date d'ouverture de la BDD a été mise à jour");
        } else {
            MethodeDiverses::ecrireLog("BaseDeDonnees::majDerniereOuvertureBDD() : ERREUR la date d'ouverture de la BDD n'a pas été mise à jour");
            QMessageBox::critical(NULL, "Erreur", "La date d'ouverture de la BDD n'a pas été mise à jour");
        }
    } else {
        MethodeDiverses::ecrireLog("BaseDeDonnees::majDerniereOuvertureBDD() : Erreur BDD non ouverte : " + db.lastError().text());
        QMessageBox::critical(NULL, "Erreur", "Erreur BDD non ouverte : " + db.lastError().text());
    }
    db.close();
    MethodeDiverses::ecrireLog("BaseDeDonnees::derniereOuvertureBDD() : Fermeture de la base de donnée");
}
