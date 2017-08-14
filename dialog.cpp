#include "dialog.h"
#include "ui_dialog.h"


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog) {
    ui->setupUi(this);
    pere = (FenetrePrincipale *)parent;
    this->setWindowTitle(pere->windowTitle());
    log = new Log();
    this->setFixedHeight(700);
    this->setFixedWidth(1200);
    ui->tableWidget->setRowCount(0);
    refresh(pere->getConfig("Configuration/Telechargement"));


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
                listeFichierADeplacer.append(pere->getConfig("Configuration/Telechargement") + ui->tableWidget->item(i,8)->text() + "/" + nomFichier);
            }
        }
        if(ui->tableWidget->cellWidget(i, 2) != NULL) {
            QCheckBox *checkBoxSupprimer = (QCheckBox *)ui->tableWidget->cellWidget(i, 2)->layout()->itemAt(0)->widget();
            if(checkBoxSupprimer->isChecked()) {
                listeFichierASupprimer.append(pere->getConfig("Configuration/Telechargement") + ui->tableWidget->item(i,8)->text() + "/" + nomFichier);
            }
        }
    }

    deplacerFichier(listeFichierADeplacer);
    supprimerFichier(listeFichierASupprimer);

    fichierCharge.clear();
    ui->tableWidget->setRowCount(0);
    refresh(pere->getConfig("Configuration/Telechargement"));
    timer->start(500);
}

void Dialog::on_pushButtonRetour_clicked() {
    closeEvent(new QCloseEvent());
    this->~Dialog();
}


void Dialog::deplacerFichier(QList<QString> liste) {
    QString dossierSerie = pere->getDossierSerie();
    QList<QString> listeSerie = pere->getListeSerie();

    foreach (QString nomFichier, liste) {
        bool estPasse = false;
        for(int i = 0; i < listeSerie.count(); i++) {
            if(nomFichier.contains(listeSerie.at(i), Qt::CaseInsensitive) || nomFichier.contains(QString(listeSerie.at(i)).replace(" ","."), Qt::CaseInsensitive)) {
                estPasse = true;

                QFile fichier(nomFichier);
                QFileInfo infoFichier(nomFichier);
                QString saison = infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("[Ss][0-9]{2}[Ee][0-9]{2}")) +1, 2).toUpper();
                QFile *newFichier = new QFile(dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison + "/" + listeSerie.at(i) + " " + infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("S[0-9]{2}E[0-9]{2}"), 0), 6).toUpper() + "." + infoFichier.suffix());

                if(newFichier->exists()) {
                    if(!methodeDiverses.msgBoxQuestion("Le fichier " + newFichier->fileName() + " existe déjà dans le dossier de la série.\nVoulez vous le remplacer ?")) {
                        newFichier->remove();
                    } else {
                        log->ecrire("Le fichier " + infoFichier.fileName() + " existe déjà et n'a pas été déplacé");
                        ui->textBrowser->append("Le fichier " + infoFichier.fileName() + " existe déjà et n'a pas été déplacé");
                    }
                }

                if(fichier.rename(dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison + "/" + listeSerie.at(i) + " " + infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("[Ss][0-9]{2}[Ee][0-9]{2}"), 0), 6).toUpper() + "." + infoFichier.suffix())) {
                    log->ecrire("\tEmplacement du nouveau fichier : " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison + "/" + listeSerie.at(i) + " " + infoFichier.fileName().mid(infoFichier.fileName().indexOf(QRegularExpression("[Ss][0-9]{2}[Ee][0-9]{2}"), 0), 6).toUpper() + "." + infoFichier.suffix());
                    ui->textBrowser->append("Le fichier " + infoFichier.fileName() + " a été déplacé dans le dossier " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison);
                } else {
                    log->ecrire("\tLe fichier " + infoFichier.fileName() + " n'a pas été déplacé dans le dossier " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison);
                    ui->textBrowser->append("Le fichier " + infoFichier.fileName() + " n'a été déplacé dans le dossier " + dossierSerie + "/" + listeSerie.at(i) + "/Saison " + saison);
                }
            }
        }
        if(!estPasse) {
            QString chemin = QFileDialog::getSaveFileName(this, "Déplacer " + QFileInfo(nomFichier).fileName(), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +  "/" + QFileInfo(nomFichier).fileName());
            if(!chemin.isNull()) {
                if(QFile(nomFichier).rename(chemin)) {
                    ui->textBrowser->append("Le fichier " + QFileInfo(nomFichier).fileName() + " a été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                    log->ecrire("Le fichier " + QFileInfo(nomFichier).fileName() + " a été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                } else {
                    ui->textBrowser->append("Le fichier " + QFileInfo(nomFichier).fileName() + " n'a pas été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                    log->ecrire("Le fichier " + QFileInfo(nomFichier).fileName() + " n'a pas été déplacé dans le dossier " + QFileInfo(chemin).absolutePath());
                }
            } else {
                ui->textBrowser->append("Aucun chemin n'a été choisi pour le fichier " + QFileInfo(nomFichier).fileName());
                log->ecrire("Aucun chemin n'a été choisi pour le fichier " + QFileInfo(nomFichier).fileName());
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
                    log->ecrire("WARNING : le fichier " + infoFile.fileName() + " n'a pas été supprimé.");
                    ui->textBrowser->append("WARNING : le fichier " + infoFile.fileName() + " n'a pas été supprimé.");
                } else {
                    log->ecrire("Le fichier " + infoFile.fileName() + " a été supprimé.");
                    ui->textBrowser->append("Le fichier " + infoFile.fileName() + " a été supprimé.");
                }
            } else {
                log->ecrire("Le fichier " + infoFile.fileName() + " a déjà été supprimé");
            }
        }
    }
}

void Dialog::refresh(QString dossier) {
    QList<QString> listeSerie = pere->getListeSerie();
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
            if(dossier == pere->getConfig("Configuration/Telechargement")) {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(pere->getConfig("Configuration/Telechargement"),"")));
            } else {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(pere->getConfig("Configuration/Telechargement"),".")));
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
            if(dossier == pere->getConfig("Configuration/Telechargement")) {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(pere->getConfig("Configuration/Telechargement"),"")));
            } else {
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 8, new QTableWidgetItem(dossier.replace(pere->getConfig("Configuration/Telechargement"),".")));
            }
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    activationBoutonDeplacer();
}

void Dialog::on_tableWidget_clicked(const QModelIndex &index) {
    if(index.column() >= 0 && index.column() <= 2 && ui->tableWidget->cellWidget(index.row(), index.column()) != NULL) {
        QCheckBox *checkBox;
        if(index.column() == 2 && ui->tableWidget->cellWidget(index.row(), 0) == NULL && QDir(pere->getConfig("Configuration/Telechargement") + ui->tableWidget->item(index.row(), 8)->text().replace(".","") + "/" + ui->tableWidget->item(index.row(), 3)->text()).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() != 0) {
            QMessageBox::information(this, this->windowTitle(), "Le dossier contient des fichiers. Si vous le supprimer tous les fichiers et dossiers présent dedans seront supprimés");
        }
        checkBox = (QCheckBox *)ui->tableWidget->cellWidget(index.row(), index.column())->layout()->itemAt(0)->widget();
        checkBox->setChecked(true);
    }
    activationBoutonDeplacer();
}

void Dialog::verificationChangementDossierTelechargement() {
    fichierTrouve.clear();
    nombreFichier(pere->getConfig("Configuration/Telechargement"));
    bool res = contenuIdentique();
    if(fichierCharge.count() != fichierTrouve.count() || !res) {
        QMessageBox::information(this, this->windowTitle(), "Les fichiers ont été modifié. Le tableau va être rechargé.");
        fichierCharge.clear();
        ui->tableWidget->setRowCount(0);
        refresh(pere->getConfig("Configuration/Telechargement"));
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
                log->ecrire("Le dossier " + fileInfo.fileName() + " a été supprimé");
            } else {
                ui->textBrowser->append("Le dossier " + fileInfo.fileName() + " n'a pas été supprimé");
                log->ecrire("Le dossier " + fileInfo.fileName() + " n'a pas été supprimé");

            }
        } else if(fileInfo.isFile()) {
            if(QFile(fileInfo.absoluteFilePath()).remove()) {
                ui->textBrowser->append("Le fichier " + fileInfo.fileName() + " a été supprimé");
                log->ecrire("Le fichier " + fileInfo.fileName() + " a été supprimé");
            }
        }
    }
    dir.rmdir(dossier);
}
