#include "dialog.h"
#include "ui_dialog.h"


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog) {
    ui->setupUi(this);
    bdd = ((FenetrePrincipale *)parent)->getBdd();
    this->setWindowTitle(((FenetrePrincipale *)parent)->windowTitle());
    this->setFixedHeight(700);
    this->setFixedWidth(1200);
    ui->tableWidget->setRowCount(0);
    refresh(MethodeDiverses::getConfig("Configuration/Telechargement"));
    majTreeWidget();

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(verificationChangementDossierTelechargement()));
    timer->start(500);
}

Dialog::~Dialog() {
    delete ui;
}

QString Dialog::tailleFichier(double taille, QString unite) {
    QString uniteFin;
    if(unite == "") {
        uniteFin = "o";
    } else if(unite == "o") {
        uniteFin = "Ko";
    } else if(unite == "Ko") {
        uniteFin = "Mo";
    } else if(unite == "Mo") {
        uniteFin = "Go";
    } else if(unite == "Go") {
        uniteFin = "To";
    } else {
        uniteFin = unite;
    }

    if(taille < 1024) {
        if(uniteFin == "o") {
            return QString::number(taille) + uniteFin;
        } else {
            return QString::number(taille, 'f', 3) + uniteFin;
        }
    } else {
        return tailleFichier(taille/1024, uniteFin);
    }
}

void Dialog::on_pushButtonDeplacerTout_clicked() {
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QCheckBox *checkBox;
        if(ui->tableWidget->cellWidget(i, 0) != NULL) {
            checkBox = (QCheckBox *)ui->tableWidget->cellWidget(i, 0)->layout()->itemAt(0)->widget();
        } else {
            checkBox = (QCheckBox *)ui->tableWidget->cellWidget(i, 1)->layout()->itemAt(0)->widget();
        }
        checkBox->setChecked(true);
    }
    activationBoutonDeplacer();
}

void Dialog::on_pushButtonSupprimerTout_clicked() {
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QCheckBox *checkBox = (QCheckBox *)ui->tableWidget->cellWidget(i, 2)->layout()->itemAt(0)->widget();
        checkBox->setChecked(true);
    }
    activationBoutonDeplacer();
}

void Dialog::on_pushButtonNeRienFaire_clicked() {
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QCheckBox *checkBox = (QCheckBox *)ui->tableWidget->cellWidget(i, 1)->layout()->itemAt(0)->widget();
        checkBox->setChecked(true);
    }
    activationBoutonDeplacer();
}

void Dialog::on_pushButtonDeplacerFichier_clicked() {
    QList<QString> listeFichierADeplacer;
    QList<QString> listeFichierASupprimer;
    timer->stop();
    ui->textBrowser->clear();

    for(int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QString nomFichier = ui->tableWidget->item(i, 3)->text();
        if(ui->tableWidget->cellWidget(i, 0) != NULL) {
            QCheckBox *checkBoxDeplacer = (QCheckBox *)ui->tableWidget->cellWidget(i, 0)->layout()->itemAt(0)->widget();
            if(checkBoxDeplacer->isChecked()) {
                listeFichierADeplacer.append(MethodeDiverses::getConfig("Configuration/Telechargement") + ui->tableWidget->item(i,8)->text() + "/" + nomFichier);
            }
        }
        if(ui->tableWidget->cellWidget(i, 2) != NULL) {
            QCheckBox *checkBoxSupprimer = (QCheckBox *)ui->tableWidget->cellWidget(i, 2)->layout()->itemAt(0)->widget();
            if(checkBoxSupprimer->isChecked()) {
                listeFichierASupprimer.append(MethodeDiverses::getConfig("Configuration/Telechargement") + ui->tableWidget->item(i,8)->text() + "/" + nomFichier);
            }
        }
    }

    deplacerFichier(listeFichierADeplacer);
    supprimerFichier(listeFichierASupprimer);

    fichierCharge.clear();
    ui->tableWidget->setRowCount(0);
    refresh(MethodeDiverses::getConfig("Configuration/Telechargement"));
    timer->start(500);
}

void Dialog::on_pushButtonRetour_clicked() {
    this->close();
}


void Dialog::deplacerFichier(QList<QString> liste) {
    QString dossierSerie = MethodeDiverses::getConfig("Configuration/Chemin");
    QList<QString> listeSerie = getListeSerie();


    foreach (QString nomFichier, liste) {
        bool estPasse = false;
        for(int i = 0; i < listeSerie.count(); i++) {
            if(nomFichier.contains(listeSerie.at(i), Qt::CaseInsensitive) || nomFichier.contains(QString(listeSerie.at(i)).replace(" ","."), Qt::CaseInsensitive)) {
                estPasse = true;

                QFile fichier(nomFichier);
                QFileInfo infoFichier(nomFichier);
                QString saison = infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("[Ss][0-9]{2}[Ee][0-9]{2}")) + 1, 2).toUpper();
                QString episode = infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("[Ss][0-9]{2}[Ee][0-9]{2}")) + 4, 2).toUpper();
                QFile *newFichier = new QFile(dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison + "/" + listeSerie.at(i) + " " + infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("S[0-9]{2}E[0-9]{2}"), 0), 6).toUpper() + "." + infoFichier.suffix());

                if(newFichier->exists()) {
                    if(!methodeDiverses.msgBoxQuestion("Le fichier " + newFichier->fileName() + " existe déjà dans le dossier de la série.\nVoulez vous le remplacer ?")) {
                        newFichier->remove();
                    } else {
                        MethodeDiverses::ecrireLog("Le fichier " + infoFichier.fileName() + " existe déjà et n'a pas été déplacé");
                        ui->textBrowser->append("Le fichier " + infoFichier.fileName() + " existe déjà et n'a pas été déplacé");
                    }
                }

                if(fichier.rename(dossierSerie + listeSerie.at(i) + "/Saison " + saison + "/" + listeSerie.at(i) + " " + infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("[Ss][0-9]{2}[Ee][0-9]{2}"), 0), 6).toUpper() + "." + infoFichier.suffix())) {
                    MethodeDiverses::ecrireLog("\tEmplacement du nouveau fichier : " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison + "/" + listeSerie.at(i) + " " + infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("[Ss][0-9]{2}[Ee][0-9]{2}"), 0), 6).toUpper() + "." + infoFichier.suffix());
                    ui->textBrowser->append("Le fichier " + infoFichier.fileName() + " a été déplacé dans le dossier " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison);

                    // Vérifier s'il existe une ligne dans la table historique
                    QStringList champs, jointures, conditions, ordres;

                    champs.append(bdd->HISTORIQUE_NOM);
                    champs.append(bdd->HISTORIQUE_SAISON);
                    champs.append(bdd->HISTORIQUE_EPISODE);
                    champs.append(bdd->HISTORIQUE_ETAT);
                    conditions.append(bdd->HISTORIQUE_NOM + " = '" + listeSerie.at(i) + "'");
                    conditions.append(bdd->HISTORIQUE_SAISON + " = '" + saison + "'");
                    conditions.append(bdd->HISTORIQUE_EPISODE + " = '" + episode + "'");

                    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
                    QList<QMap<QString,QString> > liste = retour.liste;
                    if(liste.count()) {
                        // Si oui on l'a met a jour si l'état n'est pas Vu
                        champs.clear();
                        conditions.clear();

                        champs.append(bdd->HISTORIQUE_ETAT + " = CASE WHEN " + bdd->HISTORIQUE_ETAT + " = 'NV' THEN 'T' ELSE " + bdd->HISTORIQUE_ETAT + " END");
                        conditions.append(bdd->HISTORIQUE_NOM + " = '" + listeSerie.at(i) + "'");
                        conditions.append(bdd->HISTORIQUE_SAISON + " = '" + saison + "'");
                        conditions.append(bdd->HISTORIQUE_EPISODE + " = '" + episode + "'");

                        bdd->requeteUpdate(champs, bdd->TABLE_HISTORIQUE, conditions);
                    } else {
                        // Sinon on met à jour dans la table saison a la condition que l'épisode soit celui courant
                        champs.clear();
                        conditions.clear();

                        champs.append(bdd->SAISON_ETAT + " = CASE WHEN " + bdd->SAISON_ETAT + " = 'NV' THEN 'T' ELSE " + bdd->SAISON_ETAT + " END");
                        conditions.append(bdd->SAISON_ID + " = (SELECT " + bdd->FICHE_SERIE_ID + " FROM " + bdd->TABLE_FICHE_SERIE + " WHERE " + bdd->FICHE_SERIE_NOM + " = '" + listeSerie.at(i) + "')");
                        conditions.append(bdd->SAISON_EPISODE_COURANT + " = '" + episode + "'");

                        bdd->requeteUpdate(champs, bdd->TABLE_SAISON, conditions);
                    }
                } else {
                    MethodeDiverses::ecrireLog("\tLe fichier " + infoFichier.fileName() + " n'a pas été déplacé dans le dossier " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison);
                    ui->textBrowser->append("Le fichier " + infoFichier.fileName() + " n'a été déplacé dans le dossier " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison);
                }
            }
        }
        if(!estPasse) {
            QString chemin = QFileDialog::getSaveFileName(this, "Déplacer " + QFileInfo(nomFichier).fileName(), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +  "/" + QFileInfo(nomFichier).fileName());
            if(!chemin.isNull()) {
                if(QFile(nomFichier).rename(chemin)) {
                    ui->textBrowser->append("Le fichier " + QFileInfo(nomFichier).fileName() + " a été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                    MethodeDiverses::ecrireLog("Le fichier " + QFileInfo(nomFichier).fileName() + " a été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                } else {
                    ui->textBrowser->append("Le fichier " + QFileInfo(nomFichier).fileName() + " n'a pas été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                    MethodeDiverses::ecrireLog("Le fichier " + QFileInfo(nomFichier).fileName() + " n'a pas été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                }
            } else {
                ui->textBrowser->append("Aucun chemin n'a été choisi pour le fichier " + QFileInfo(nomFichier).fileName());
                MethodeDiverses::ecrireLog("Aucun chemin n'a été choisi pour le fichier " + QFileInfo(nomFichier).fileName());
            }
        }
    }
}

void Dialog::supprimerFichier(QList<QString> liste) {
    foreach (QString fichier, liste) {
        QFileInfo infoFile(fichier);
        if(infoFile.isDir()) {
            supprimerDossierEtContenu(fichier);
        } else if(infoFile.isFile()) {
            if(infoFile.exists()) {
                if(!QFile(infoFile.absoluteFilePath()).remove()) {
                    MethodeDiverses::ecrireLog("WARNING : le fichier " + infoFile.fileName() + " n'a pas été supprimé.");
                    ui->textBrowser->append("WARNING : le fichier " + infoFile.fileName() + " n'a pas été supprimé.");
                } else {
                    MethodeDiverses::ecrireLog("Le fichier " + infoFile.fileName() + " a été supprimé.");
                    ui->textBrowser->append("Le fichier " + infoFile.fileName() + " a été supprimé.");
                }
            } else {
                MethodeDiverses::ecrireLog("Le fichier " + infoFile.fileName() + " a déjà été supprimé");
            }
        }
    }
}

void Dialog::refresh(QString dossier) {
    QList<QString> listeSerie = getListeSerie();
    foreach (QFileInfo info, QDir(dossier).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries)) {
        if(info.isDir()) {
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
            fichierCharge.append(info.fileName());
            QCheckBox *checkNeRienFaire = new QCheckBox();
            QCheckBox *checkSupprimer = new QCheckBox();
            QWidget *widgetNeRienFaire = new QWidget();
            QWidget *widgetSupprimer = new QWidget();
            QHBoxLayout *layoutNeRienFaire = new QHBoxLayout(widgetNeRienFaire);
            QHBoxLayout *layoutSupprimer = new QHBoxLayout(widgetSupprimer);
            QButtonGroup *group = new QButtonGroup();

            layoutNeRienFaire->addWidget(checkNeRienFaire);
            layoutSupprimer->addWidget(checkSupprimer);
            layoutNeRienFaire->setAlignment(Qt::AlignCenter);
            layoutSupprimer->setAlignment(Qt::AlignCenter);
            layoutNeRienFaire->setContentsMargins(0,0,0,0);
            layoutSupprimer->setContentsMargins(0,0,0,0);
            widgetNeRienFaire->setLayout(layoutNeRienFaire);
            widgetSupprimer->setLayout(layoutSupprimer);
            group->addButton(checkNeRienFaire);
            group->addButton(checkSupprimer);
            group->setExclusive(true);
            checkNeRienFaire->setChecked(true);

            QTableWidgetItem *itemTailleFichier = new QTableWidgetItem(tailleFichier(info.size(), ""));
            itemTailleFichier->setTextAlignment(Qt::AlignRight|Qt::AlignCenter);

            ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1, 1, widgetNeRienFaire);
            ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1, 2, widgetSupprimer);
            QFileIconProvider iconProvider;
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, new QTableWidgetItem(QIcon(iconProvider.icon(info)), info.fileName()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 4, new QTableWidgetItem(info.created().toString("dd/MM/yyyy hh:mm:ss")));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 5, new QTableWidgetItem(info.lastModified().toString("dd/MM/yyyy hh:mm:ss")));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 6, itemTailleFichier);
            QTableWidgetItem *itemSeriePresente = new QTableWidgetItem();
            itemSeriePresente->setBackgroundColor(QColor("red"));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 7, itemSeriePresente);
            if(dossier == MethodeDiverses::getConfig("Configuration/Telechargement")) {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(MethodeDiverses::getConfig("Configuration/Telechargement"),"")));
            } else {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(MethodeDiverses::getConfig("Configuration/Telechargement"),".")));
            }
            refresh(info.absoluteFilePath());
        } else if (info.isFile()) {
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
            fichierCharge.append(info.fileName());
            QCheckBox *checkDeplacer = new QCheckBox();
            QCheckBox *checkNeRienFaire = new QCheckBox();
            QCheckBox *checkSupprimer = new QCheckBox();
            QWidget *widgetDeplacer = new QWidget();
            QWidget *widgetNeRienFaire = new QWidget();
            QWidget *widgetSupprimer = new QWidget();
            QHBoxLayout *layoutDeplacer = new QHBoxLayout(widgetDeplacer);
            QHBoxLayout *layoutNeRienFaire = new QHBoxLayout(widgetNeRienFaire);
            QHBoxLayout *layoutSupprimer = new QHBoxLayout(widgetSupprimer);
            QButtonGroup *group = new QButtonGroup();

            layoutDeplacer->addWidget(checkDeplacer);
            layoutNeRienFaire->addWidget(checkNeRienFaire);
            layoutSupprimer->addWidget(checkSupprimer);
            layoutDeplacer->setAlignment(Qt::AlignCenter);
            layoutNeRienFaire->setAlignment(Qt::AlignCenter);
            layoutSupprimer->setAlignment(Qt::AlignCenter);
            layoutDeplacer->setContentsMargins(0,0,0,0);
            layoutNeRienFaire->setContentsMargins(0,0,0,0);
            layoutSupprimer->setContentsMargins(0,0,0,0);
            widgetDeplacer->setLayout(layoutDeplacer);
            widgetNeRienFaire->setLayout(layoutNeRienFaire);
            widgetSupprimer->setLayout(layoutSupprimer);
            group->addButton(checkDeplacer);
            group->addButton(checkNeRienFaire);
            group->addButton(checkSupprimer);
            group->setExclusive(true);
            checkNeRienFaire->setChecked(true);

            QTableWidgetItem *itemTailleFichier = new QTableWidgetItem(tailleFichier(info.size(), ""));
            itemTailleFichier->setTextAlignment(Qt::AlignRight|Qt::AlignCenter);

            ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1, 0, widgetDeplacer);
            ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1, 1, widgetNeRienFaire);
            ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1, 2, widgetSupprimer);
            QFileIconProvider iconProvider;
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, new QTableWidgetItem(QIcon(iconProvider.icon(info)), info.fileName()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 4, new QTableWidgetItem(info.created().toString("dd/MM/yyyy hh:mm:ss")));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 5, new QTableWidgetItem(info.lastModified().toString("dd/MM/yyyy hh:mm:ss")));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 6, itemTailleFichier);
            QTableWidgetItem *itemSeriePresente = new QTableWidgetItem();
            itemSeriePresente->setBackgroundColor(QColor("red"));
            foreach (QString serie, listeSerie) {
                 if(info.fileName().contains(serie, Qt::CaseInsensitive) || info.fileName().contains(QString(serie).replace(" ","."), Qt::CaseInsensitive)) {
                    itemSeriePresente->setBackgroundColor(QColor("green"));
                }
            }
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 7, itemSeriePresente);
            if(dossier == MethodeDiverses::getConfig("Configuration/Telechargement")) {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(MethodeDiverses::getConfig("Configuration/Telechargement"),"")));
            } else {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(MethodeDiverses::getConfig("Configuration/Telechargement"),".")));
            }
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    activationBoutonDeplacer();
}

void Dialog::majTreeWidget() {
    ui->treeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(TREE_WIDGET_NOM, MethodeDiverses::getConfig("Configuration/Telechargement"));
    ui->treeWidget->addTopLevelItem(item);
    remplirTreeWidget(item, MethodeDiverses::getConfig("Configuration/Telechargement"));
    ui->treeWidget->resizeColumnToContents(TREE_WIDGET_NOM);
    ui->treeWidget->resizeColumnToContents(TREE_WIDGET_DATE_CREATION);
    ui->treeWidget->resizeColumnToContents(TREE_WIDGET_DATE_MODIFICATION);
    ui->treeWidget->resizeColumnToContents(TREE_WIDGET_TAILLE);
}

void Dialog::remplirTreeWidget(QTreeWidgetItem *itemPere, QString chemin) {
    foreach (QFileInfo fileInfo, QDir(chemin).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries)) {
        QFileIconProvider iconProvider;
        if(fileInfo.isDir()) {
            // Si c'est un dossier
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(TREE_WIDGET_NOM, fileInfo.fileName());
            item->setIcon(TREE_WIDGET_NOM, QIcon(iconProvider.icon(fileInfo)));
            item->setText(TREE_WIDGET_DATE_CREATION, fileInfo.created().toString("dd/MM/yyyy hh:mm:ss"));
            item->setText(TREE_WIDGET_DATE_MODIFICATION, fileInfo.lastModified().toString("dd/MM/yyyy hh:mm:ss"));
            item->setCheckState(TREE_WIDGET_NE_RIEN_FAIRE, Qt::Checked);
            item->setCheckState(TREE_WIDGET_SUPPRIMER, Qt::Unchecked);
            itemPere->addChild(item);
            remplirTreeWidget(item, chemin + "/" + fileInfo.fileName());
        } else {
            // Si c'est un fichier
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(TREE_WIDGET_NOM, fileInfo.fileName());
            item->setIcon(TREE_WIDGET_NOM, QIcon(iconProvider.icon(fileInfo)));
            item->setText(TREE_WIDGET_DATE_CREATION, fileInfo.created().toString("dd/MM/yyyy hh:mm:ss"));
            item->setText(TREE_WIDGET_DATE_MODIFICATION, fileInfo.lastModified().toString("dd/MM/yyyy hh:mm:ss"));
            item->setText(TREE_WIDGET_TAILLE, tailleFichier(fileInfo.size(), ""));
            item->setCheckState(TREE_WIDGET_DEPLACER, Qt::Unchecked);
            item->setCheckState(TREE_WIDGET_NE_RIEN_FAIRE, Qt::Checked);
            item->setCheckState(TREE_WIDGET_SUPPRIMER, Qt::Unchecked);
            itemPere->addChild(item);
            // TODO On ajoute les autres informations et les checkbox
            itemPere->addChild(item);
        }
    }
    itemPere->setExpanded(true);
}

void Dialog::on_tableWidget_clicked(const QModelIndex &index) {
    if(index.column() >= 0 && index.column() <= 2 && ui->tableWidget->cellWidget(index.row(), index.column()) != NULL) {
        QCheckBox *checkBox;
        if(index.column() == 2 && ui->tableWidget->cellWidget(index.row(), 0) == NULL && QDir(MethodeDiverses::getConfig("Configuration/Telechargement") + ui->tableWidget->item(index.row(), 8)->text().replace(".","") + "/" + ui->tableWidget->item(index.row(), 3)->text()).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() != 0) {
            QMessageBox::information(this, this->windowTitle(), "Le dossier contient des fichiers. Si vous le supprimer tous les fichiers et dossiers présent dedans seront supprimés");
        }
        checkBox = (QCheckBox *)ui->tableWidget->cellWidget(index.row(), index.column())->layout()->itemAt(0)->widget();
        checkBox->setChecked(true);
    }
    activationBoutonDeplacer();
}

void Dialog::verificationChangementDossierTelechargement() {
    fichierTrouve.clear();
    nombreFichier(MethodeDiverses::getConfig("Configuration/Telechargement"));
    bool res = contenuIdentique();
    if(fichierCharge.count() != fichierTrouve.count() || !res) {
        QMessageBox::information(this, this->windowTitle(), "Les fichiers ont été modifié. Le tableau va être rechargé.");
        fichierCharge.clear();
        ui->tableWidget->setRowCount(0);
        refresh(MethodeDiverses::getConfig("Configuration/Telechargement"));

        majTreeWidget();
    }
}

void Dialog::nombreFichier(QString chemin) {
    foreach (QFileInfo info, QDir(chemin).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries)) {
        if(info.isDir()) {
            fichierTrouve.append(info.fileName());
            nombreFichier(info.absoluteFilePath());
        } else if (info.isFile()) {
            fichierTrouve.append(info.fileName());
        }
    }
}

bool Dialog::contenuIdentique() {
    foreach (QString fichier, fichierTrouve) {
        if(!fichierCharge.contains(fichier)) {
            return false;
        }
    }
    return true;
}

void Dialog::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
    timer->stop();
}

void Dialog::activationBoutonDeplacer() {
    bool actif = false;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QCheckBox *checkBox = (QCheckBox *)ui->tableWidget->cellWidget(i,1)->layout()->itemAt(0)->widget();
        if(!checkBox->isChecked()) {
            actif = true;
        }
    }
    ui->pushButtonDeplacerFichier->setEnabled(actif);
}

void Dialog::supprimerDossierEtContenu(QString dossier) {
    QDir dir;
    foreach (QFileInfo fileInfo, QDir(dossier).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries)) {
        if(fileInfo.isDir()) {
            supprimerDossierEtContenu(fileInfo.absoluteFilePath());
            if(dir.rmdir(fileInfo.absoluteFilePath())) {
                ui->textBrowser->append("Le dossier " + fileInfo.fileName() + " a été supprimé");
                MethodeDiverses::ecrireLog("Le dossier " + fileInfo.fileName() + " a été supprimé");
            } else {
                ui->textBrowser->append("Le dossier " + fileInfo.fileName() + " n'a pas été supprimé");
                MethodeDiverses::ecrireLog("Le dossier " + fileInfo.fileName() + " n'a pas été supprimé");

            }
        } else if(fileInfo.isFile()) {
            if(QFile(fileInfo.absoluteFilePath()).remove()) {
                ui->textBrowser->append("Le fichier " + fileInfo.fileName() + " a été supprimé");
                MethodeDiverses::ecrireLog("Le fichier " + fileInfo.fileName() + " a été supprimé");
            }
        }
    }
    dir.rmdir(dossier);
}

QList<QString> Dialog::getListeSerie() {
    QList<QString> listeSerie;
    foreach (QFileInfo file, QDir(MethodeDiverses::getConfig("Configuration/Chemin")).entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs)) {
        if(file.isDir() && file.absoluteFilePath() != MethodeDiverses::getConfig("Configuration/Telechargement")) {
            QString nom;
            nom = file.absoluteFilePath();
            nom.replace(MethodeDiverses::getConfig("Configuration/Chemin"), "");
            listeSerie.append(nom);

        }
    }
    return listeSerie;
}
