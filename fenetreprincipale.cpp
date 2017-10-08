#include "fenetreprincipale.h"
#include "ui_fenetreprincipale.h"
#include <QScrollBar>

FenetrePrincipale::FenetrePrincipale(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FenetrePrincipale) {
    ui->setupUi(this);

    QDir dir;
#ifdef QT_DEBUG
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs").exists())
        dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs");
    QFile *file = new QFile(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini");
#else
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs").exists())
        dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs");
    QFile *file = new QFile(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini");
#endif

    log = new Log();

    // Refresh automatique
    QTimer *timer = new QTimer();
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
    timer->start(300000);

    QTimer* timerDossierSerie = new QTimer();
    QObject::connect(timerDossierSerie, SIGNAL(timeout()), this, SLOT(majIndicateur()));
    timerDossierSerie->start(500);

    if(file->exists()) {
        if(getConfig("Configuration/Chemin") == "" ||
           getConfig("Configuration/Telechargement") == "" ||
           getConfig("Configuration/Extension") == "" ||
           getConfig("Configuration/PurgeLog") == "") {
            premiereConnexion();
        } else {
            initialisation();
        }
    } else {
        premiereConnexion();
    }
}

FenetrePrincipale::~FenetrePrincipale() {
    delete ui;
}

void FenetrePrincipale::premiereConnexion() {
    log->ecrire("FenetrePrincipale::premiereConnexion() : Début du paramétrage de première connexion");
    ui->menuBar->setVisible(false);
    ui->mainToolBar->setVisible(false);
    ui->pagePrincipaleLabelDossierSerie->setVisible(false);
    ui->pagePrincipaleLabelDossierSerie_2->setVisible(false);
    ui->pagePrincipaleLabelInternet->setVisible(false);
    ui->pagePrincipaleLabelInternet_2->setVisible(false);
    ui->stackedWidget->setCurrentWidget(ui->pageConfig);
    ui->pageConfigurationSpinBoxLogEfface->setValue(getConfig("Configuration/PurgeLog").toInt());
    ui->pageConfigurationLineDossierTelechargement->setText(getConfig("Configuration/Telechargement"));
    ui->pageConfigurationLineDossierSerie->setText(getConfig("Configuration/Chemin"));
    ui->pageConfigurationLineEditExtension->setText(getConfig("Configuration/Extension"));
    switch (getConfig("Configuration/Qualite").toInt()) {
    case 0:
        ui->pageConfigurationRadioButtonQualiteToutes->setChecked(true);
        break;
    case 1:
        ui->pageConfigurationRadioButtonQualite720->setChecked(true);
        break;
    case 2:
        ui->pageConfigurationRadioButtonQualite1080->setChecked(true);
        break;
    }
    switch (getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        ui->pageConfigurationRadioButtonSousTitresToutes->setChecked(true);
        break;
    case 1:
        ui->pageConfigurationRadioButtonSousTitresVOSTFR->setChecked(true);
        break;
    }
    log->ecrire("FenetrePrincipale::premiereConnexion() : Fin du paramétrage de première connexion");
}

void FenetrePrincipale::initialisation() {
    log->ecrire("FenetrePrincipale::initialisation() : Début de l'initialisation");
    // Mettre le menu en visible
    ui->menuBar->setVisible(true);
    ui->mainToolBar->setVisible(true);
    ui->pagePrincipaleLabelDossierSerie->setVisible(true);
    ui->pagePrincipaleLabelDossierSerie_2->setVisible(true);
    ui->pagePrincipaleLabelInternet->setVisible(true);
    ui->pagePrincipaleLabelInternet_2->setVisible(true);
    ui->pageReporterSpinBox->setMinimum(1);
    // Masquer les champs
    ui->pageReporterLabelNomSerie->setVisible(false);
    ui->pageAjoutModifLabelRetour->setVisible(false);
    ui->pageAjoutModifLabelCheminPhoto->setVisible(false);
    // Recuperation de la base de données ou création si elle n'existe pas
    this->bdd = new BaseDeDonnees();
    // Récupération de la version de l'appli
    this->version = "2.5";
#ifdef QT_DEBUG
    this->dateVersion = QFileInfo("./debug/Series_Manager.exe").lastModified().date();
    this->heureVersion = QFileInfo("./debug/Series_Manager.exe").lastModified().time();
    this->setWindowTitle("Series Manager " + this->version + " DEBUG");
#else
    this->dateVersion = QFileInfo("Series_Manager.exe").lastModified().date();
    this->heureVersion = QFileInfo("Series_Manager.exe").lastModified().time();
    this->setWindowTitle("Series Manager " + this->version);
#endif
    // Charge le fichier de configuration
    chargementConfiguration();
    // Place l'appli sur la page principale
    ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
    // Met l'onglet Aujourd'hui
    ui->pagePrincipaleTabWidget->setCurrentWidget(ui->pagePrincipaleTabWidgetTabAuj);
    // Rafraichis les listes
    refresh();
    log->ecrire("FenetrePrincipale::initialisation() : Fin de l'initialisation");
}

void FenetrePrincipale::refresh() {
    log->ecrire("FenetrePrincipale::refresh() : Début de l'actualisation");
    // Met a jour les episodes de la veille
    majEpisode();

    QList<QString> champs;
    QList<QString> jointures;
    QList<QString> conditions;
    QList<QString> ordres;
    champs.append(HISTORIQUE_NOM);
    champs.append(HISTORIQUE_SAISON);
    champs.append(HISTORIQUE_EPISODE);
    conditions.append(HISTORIQUE_DATE_AJOUT + " = '" + QDate::currentDate().addDays(-1).toString("yyyy-MM-dd") + "' ");
    //ordres.append(HISTORIQUE_DATE_AJOUT + " DESC");
    ordres.append(HISTORIQUE_NOM);
    ordres.append(HISTORIQUE_EPISODE);
    QList<QMap<QString,QString>> listeHier = bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);

    champs.clear();
    conditions.clear();
    ordres.clear();
    champs.append(FICHE_SERIE_NOM);
    champs.append(FICHE_SERIE_WIKI);
    champs.append(FICHE_SERIE_ADDICTED);
    champs.append(SAISON_SAISON);
    champs.append(SAISON_NB_EPISODE);
    champs.append(SAISON_EPISODE_COURANT);
    champs.append(SAISON_DATE_SORTIE);
    champs.append(SAISON_JOUR_SORTIE);
    champs.append(SAISON_DATE_MODIF);
    champs.append(SAISON_WIKI);
    conditions.append(FICHE_SERIE_ID + " = " + SAISON_ID);
    ordres.append(SAISON_DATE_MODIF);
    ordres.append(SAISON_JOUR_SORTIE);

    listeGlobal = bdd->requeteSelect(champs, FICHE_SERIE_TABLE + ", " + SAISON_TABLE, jointures, conditions, ordres);

    ordres.clear();
    ordres.append(FICHE_SERIE_NOM);
    conditions.append(SAISON_DATE_MODIF + " = '" + QDate::currentDate().addDays(-7).toString("yyyy-MM-dd") + "' ");

    listeQuoti = bdd->requeteSelect(champs, FICHE_SERIE_TABLE + ", " + SAISON_TABLE, conditions, ordres);

    int indice = 0;
    bool activationBoutonWikiGlobal = false;
    bool activationBoutonAddictedGlobal = false;
    QString qualite;
    QString sousTitres;
    QString fin;

    switch (getConfig("Configuration/Qualite").toInt()) {
    case 0:
        qualite = "";
        break;
    case 1:
        qualite = "+720p";
        break;
    case 2:
        qualite = "+1080p";
        break;
    }

    switch (getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        sousTitres = "";
        break;
    case 1:
        sousTitres = "+VOSTFR";
        break;
    }

    //Effacement des tableau
    log->ecrire("\tEffacement des liste effectué");

    on_pageVosSeriesComboBox_currentIndexChanged(getConfig("Configuration/ListeSerie"));
    log->ecrire("\tAjout des données dans le tableau vos séries");

    // refresh de l'affichage
    ui->pagePrincipaleTableWidgetDisplay->setRowCount(0);
    // Refresh de l'onglet aujourd'hui
    for (int i = 0; i < listeQuoti.count(); i++) {
        QMap<QString, QString> map = listeQuoti.value(i);

        // Page Principale
        QSignalMapper* mapper1 = new QSignalMapper();
        QSignalMapper* mapper2 = new QSignalMapper();
        QSignalMapper* mapper3 = new QSignalMapper();
        QSignalMapper* mapper4 = new QSignalMapper();

        QPushButton* url = new QPushButton(i_logo, "");
        QPushButton* wiki = new QPushButton(i_wiki, "");
        QPushButton* reporter = new QPushButton(i_reporter, "");
        QPushButton* addicted = new QPushButton(i_addicted, "");

        url->setToolTip("Ouvre le lien URL de " + map.value(FICHE_SERIE_NOM));
        wiki->setToolTip("Ouvre le lien Wikipédia de " + map.value(FICHE_SERIE_NOM));
        reporter->setToolTip("Ouvre la fênetre de report de " + map.value(FICHE_SERIE_NOM));
        addicted->setToolTip("Ouvre le lien Addic7ed de " + map.value(FICHE_SERIE_NOM));

        mapper1->setMapping(url, lien + QString(map.value(FICHE_SERIE_NOM)).replace(" ", "+") + "+S" + map.value(SAISON_SAISON) + "E" + map.value(SAISON_EPISODE_COURANT) + sousTitres + qualite + fin);
        if(map.value(SAISON_WIKI) != "") {
            mapper2->setMapping(wiki, map.value(SAISON_WIKI));
        } else if (map.value(FICHE_SERIE_WIKI) != "") {
            mapper2->setMapping(wiki, map.value(FICHE_SERIE_WIKI));
        } else {
            mapper2->setMapping(wiki, "");
            wiki->setEnabled(false);
        }
        mapper3->setMapping(reporter, map.value(FICHE_SERIE_NOM));
        if(map.value(FICHE_SERIE_ADDICTED) != "") {
            mapper4->setMapping(addicted, map.value(FICHE_SERIE_ADDICTED) + "/" + QString::number(map.value(SAISON_SAISON).toInt()) + "/" + QString::number(map.value(SAISON_EPISODE_COURANT).toInt()) + "/8");
        } else {
            addicted->setEnabled(false);
        }
        QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(url, SIGNAL(clicked()), mapper1, SLOT(map()));

        QObject::connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(wiki, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

        QObject::connect(mapper3, SIGNAL(mapped(QString)), this, SLOT(bouton_reporter_clicked(QString)));
        QObject::connect(reporter, SIGNAL(clicked(bool)), mapper3, SLOT(map()));

        QObject::connect(mapper4, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(addicted, SIGNAL(clicked(bool)), mapper4, SLOT(map()));

        ui->pagePrincipaleTableWidgetDisplay->setRowCount(indice+1);
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(map.value(FICHE_SERIE_NOM), false));
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(map.value(SAISON_SAISON), true));
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(map.value(SAISON_EPISODE_COURANT), true));
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 3, url);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 4, addicted);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 5, wiki);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 6, reporter);

        indice++;

    }

    log->ecrire("\tActualisation de l'onglet Aujourd'hui effectué");

    if(listeQuoti.isEmpty()) {
        ui->pagePrincipaleBoutonUrl->setEnabled(false);
    } else {
        ui->pagePrincipaleBoutonUrl->setEnabled(true);
    }

    for (int i = 0; i < listeQuoti.count(); ++i) {
        if(!listeQuoti.at(i).value(FICHE_SERIE_WIKI).isEmpty() || !listeQuoti.at(i).value(SAISON_WIKI).isEmpty()) {
            activationBoutonWikiGlobal = true;
        }
    }
    ui->pagePrincipaleBoutonWiki->setEnabled(activationBoutonWikiGlobal);


    for (int i = 0; i < listeQuoti.count(); ++i) {
        if(!listeQuoti.at(i).value(FICHE_SERIE_ADDICTED).isEmpty()) {
            activationBoutonAddictedGlobal = true;
        }
    }
    ui->pagePrincipalePushButtonAddicted->setEnabled(activationBoutonAddictedGlobal);

    log->ecrire("\tParamètrage des boutons globaux");



    // Refresh de l'onglet hier
    ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(0);

    indice = 0;

    for(int i = 0; i < listeHier.count(); i++) {
        QMap<QString, QString> mapHier = listeHier.value(i);
        QList<QString> jointures;
        champs.clear();
        conditions.clear();
        ordres.clear();
        champs.append(FICHE_SERIE_WIKI);
        champs.append(SAISON_WIKI);
        //conditions.append(FICHE_SERIE_ID + " = " + SAISON_ID);
        jointures.append("LEFT JOIN " + SAISON_TABLE + " ON " + FICHE_SERIE_ID + " = " + SAISON_ID);
        conditions.append(FICHE_SERIE_NOM + " = '" + mapHier.value(HISTORIQUE_NOM) + "'");
        QList<QMap<QString, QString> >listeWiki = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE/* + ", " + SAISON_TABLE*/, jointures, conditions, ordres);

        QSignalMapper* mapper1 = new QSignalMapper();
        QSignalMapper* mapper2 = new QSignalMapper();
        QSignalMapper* mapper3 = new QSignalMapper();

        QPushButton* lienHier = new QPushButton(i_logo, "");
        QPushButton* wikiHier = new QPushButton(i_wiki, "");
        QPushButton* addicted = new QPushButton(i_addicted, "");

        lienHier->setToolTip("Ouvre le lien URL de " + mapHier.value(HISTORIQUE_NOM));
        wikiHier->setToolTip("Ouvre le lien Wikipédia de " + mapHier.value(HISTORIQUE_NOM));
        addicted->setToolTip("Ouvre le lien Addic7ed de " + mapHier.value(HISTORIQUE_NOM));

        QString lienWiki;
        if(!listeWiki.isEmpty()) {
            if(listeWiki.at(0).value(SAISON_WIKI) != "") {
                lienWiki = listeWiki.at(0).value(SAISON_WIKI);
            } else if(listeWiki.at(0).value(FICHE_SERIE_WIKI) != "") {
                lienWiki = listeWiki.at(0).value(FICHE_SERIE_WIKI);
            }
        } else {
            lienWiki = "";
            wikiHier->setEnabled(false);
        }

        champs.clear();
        conditions.clear();
        ordres.clear();
        champs.append(FICHE_SERIE_ADDICTED);
        conditions.append(FICHE_SERIE_NOM + " = '" + mapHier.value(HISTORIQUE_NOM) + "'");
        QString lienAddicted = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres).at(0).value(FICHE_SERIE_ADDICTED);

        mapper1->setMapping(lienHier, lien + QString(mapHier.value(HISTORIQUE_NOM)).replace(" ", "+") + "+S" + mapHier.value(HISTORIQUE_SAISON) + "E" + mapHier.value(HISTORIQUE_EPISODE) + sousTitres + qualite + fin);
        mapper2->setMapping(wikiHier, lienWiki);
        if(lienAddicted != "") {
            mapper3->setMapping(addicted, lienAddicted + "/" + QString::number(mapHier.value(HISTORIQUE_SAISON).toInt()) + "/" + QString::number(mapHier.value(HISTORIQUE_EPISODE).toInt()) + "/8");
        } else {
            addicted->setEnabled(false);
        }

        QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(lienHier, SIGNAL(clicked()), mapper1, SLOT(map()));

        QObject::connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(wikiHier, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

        QObject::connect(mapper3, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(addicted, SIGNAL(clicked(bool)), mapper3, SLOT(map()));

        ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(indice+1);
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 0, methodeDiverses.itemForTableWidget(mapHier.value(HISTORIQUE_NOM), false));
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 1, methodeDiverses.itemForTableWidget(mapHier.value(HISTORIQUE_SAISON), true));
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 2, methodeDiverses.itemForTableWidget(mapHier.value(HISTORIQUE_EPISODE), true));
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 3, lienHier);
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 4, addicted);
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 5, wikiHier);

        indice++;
    }

    log->ecrire("\tActualisation de l'onglet Hier effectué");

    // Refresh de l'historique
    ui->pageHistoriqueTableWidget->setRowCount(0);

    indice = 0;

    champs.clear();
    conditions.clear();
    ordres.clear();
    champs.append(HISTORIQUE_NOM);
    champs.append(HISTORIQUE_SAISON);
    champs.append(HISTORIQUE_EPISODE);
    champs.append(HISTORIQUE_DATE_AJOUT);
    ordres.append(HISTORIQUE_DATE_AJOUT + " DESC");
    QList<QMap<QString, QString> > listeHistorique = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    for(int i = 0; i < listeHistorique.count(); i++) {
        QMap<QString, QString> map = listeHistorique.value(i);

        QSignalMapper* mapper = new QSignalMapper();
        QPushButton* url = new QPushButton(i_logo, "");
        url->setToolTip("Ouvre le lien URL de " + map.value(HISTORIQUE_NOM) + "pour l'épisode donné");
        mapper->setMapping(url, lien + QString(map.value(HISTORIQUE_NOM)).replace(" ","+") + "+S" + map.value(HISTORIQUE_SAISON) + "E" + map.value(HISTORIQUE_EPISODE) + sousTitres + qualite + fin);
        QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(url, SIGNAL(clicked()), mapper, SLOT(map()));

        ui->pageHistoriqueTableWidget->setRowCount(indice+1);
        ui->pageHistoriqueTableWidget->setItem(indice, 0, methodeDiverses.itemForTableWidget(map.value(HISTORIQUE_NOM), false));
        ui->pageHistoriqueTableWidget->setItem(indice, 1, methodeDiverses.itemForTableWidget(map.value(HISTORIQUE_SAISON), true));
        ui->pageHistoriqueTableWidget->setItem(indice, 2, methodeDiverses.itemForTableWidget(map.value(HISTORIQUE_EPISODE), true));
        ui->pageHistoriqueTableWidget->setItem(indice, 3, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(map.value(HISTORIQUE_DATE_AJOUT)).toString("dd/MM/yy"), true));
        ui->pageHistoriqueTableWidget->setCellWidget(indice, 4, url);

        indice++;
    }

    log->ecrire("\tActualisation du tableau de l'historique effectué");

    // Dimensionner les colonnes
    ui->pagePrincipaleTableWidgetDisplay->resizeColumnsToContents();
    ui->pagePrincipaleTableWidgetDisplay->setColumnWidth(3, 60);
    ui->pagePrincipaleTableWidgetDisplay->setColumnWidth(4, 60);
    ui->pagePrincipaleTableWidgetDisplay->setColumnWidth(5, 60);
    ui->pagePrincipaleTableWidgetDisplay->setColumnWidth(6, 60);

    ui->pagePrincipaleTableWidgetDisplay_2->resizeColumnsToContents();
    ui->pagePrincipaleTableWidgetDisplay_2->setColumnWidth(3, 60);
    ui->pagePrincipaleTableWidgetDisplay_2->setColumnWidth(4, 60);
    ui->pagePrincipaleTableWidgetDisplay_2->setColumnWidth(5, 60);

    ui->pageHistoriqueTableWidget->resizeColumnsToContents();
    ui->pageHistoriqueTableWidget->setColumnWidth(4, 60);

    log->ecrire("\tDimensionnement des colonnes des tableaux effectué");

    if(listeGlobal.isEmpty()) {
        ui->pagePrincipaleBoutonVosSeries->setEnabled(false);
        ui->pagePrincipaleBoutonModifier->setEnabled(false);
        ui->toolBarModifier->setEnabled(false);
    } else {
        ui->pagePrincipaleBoutonVosSeries->setEnabled(true);
        ui->pagePrincipaleBoutonModifier->setEnabled(true);
        ui->toolBarModifier->setEnabled(true);
    }

    log->ecrire("\tAjout des noms de série pour les pages de sélection de modification et de suppression");

    log->ecrire("\tTri des liste de modification et de suppression");

    ficheSerie.clear();
    champs.clear();
    conditions.clear();
    ordres.clear();
    champs.append(FICHE_SERIE_NOM);
    QList<QMap<QString,QString> > listeFicheSerie = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres);
    for(int i = 0; i < listeFicheSerie.count(); i++) {
        ficheSerie.append(listeFicheSerie.at(i).value(FICHE_SERIE_NOM));
    }

    log->ecrire("\tAjout des noms des séries du dossier des série dans la liste déroulante de l'ajout de série");

    ui->statusBar->showMessage("Actualisé à " + QTime::currentTime().toString("hh:mm:ss"));
    log->ecrire("FenetrePrincipale::refresh() : Fin de l'actualisation");
}

void FenetrePrincipale::bouton_lien_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::bouton_lien_clicked() : Début de la génération du lien");
    if(QDesktopServices::openUrl(QUrl(nom))) {
        log->ecrire("\tOuverture du lien " + nom);
    } else {
        QMessageBox::warning(this, this->windowTitle(), "Le lien " + nom + " n'a pu être ouvert");
        log->ecrire("\tLe lien " + nom + " n'a pu être ouvert");
    }

    log->ecrire("FenetrePrincipale::bouton_lien_clicked() : Fin de la génération du lien");
}

void FenetrePrincipale::bouton_dossier_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::bouton_dossier_clicked() : Début de l'ouverture du dossier");
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + nom))) {
        QMessageBox::warning(this,this->windowTitle(), "Le dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
        log->ecrire("\tLe dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
    } else {
        log->ecrire("\tLe dossier " + dossierSerie + nom + " a été ouvert");
    }
    log->ecrire("FenetrePrincipale::bouton_dossier_clicked() : Fin de l'ouverture du dossier");
}

void FenetrePrincipale::bouton_reporter_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::bouton_reporter_clicked() : Ouverture de la page de report de série");
    on_pageReporterSpinBox_valueChanged(ui->pageReporterSpinBox->value());
    ui->stackedWidget->setCurrentWidget(ui->pageReporter);
    ui->pageReporterLabel->setText("De combien de semaines voulez vous reporter " + nom + " ?");
    ui->pageReporterLabelNomSerie->setText(nom);
    ui->pageReporterSpinBox->setValue(1);
}

void FenetrePrincipale::majIndicateur() {
    if(QDir(dossierSerie).exists()) {
        ui->pagePrincipaleLabelDossierSerie_2->setPixmap(i_feuVert);
    } else {
        ui->pagePrincipaleLabelDossierSerie_2->setPixmap(i_feuRouge);
    }

    QNetworkConfigurationManager network;
    if(network.isOnline()) {
        ui->pagePrincipaleLabelInternet_2->setPixmap(i_feuVert);
    } else {
        ui->pagePrincipaleLabelInternet_2->setPixmap(i_feuRouge);
    }

    if(QDir(getConfig("Configuration/Telechargement")).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() > 0) {
        ui->pagePrincipaleBoutonDeplacerFichier->setEnabled(true);
        ui->toolBarDeplcerFichiers->setEnabled(true);
    } else {
        ui->pagePrincipaleBoutonDeplacerFichier->setEnabled(false);
        ui->toolBarDeplcerFichiers->setEnabled(false);
    }
}

void FenetrePrincipale::chargementConfiguration() {
    log->ecrire("FenetrePrincipale::chargementConfiguration() : Début du chargement du fichier de configuration");
    QString listeSerie = getConfig("Configuration/ListeSerie");

    if(listeSerie == "Toutes") {
        ui->pageVosSeriesComboBox->setCurrentIndex(0);
    } else if (listeSerie == "Série en cours") {
        ui->pageVosSeriesComboBox->setCurrentIndex(1);
    } else if (listeSerie == "Série à venir") {
        ui->pageVosSeriesComboBox->setCurrentIndex(2);
    }

    this->dossierSerie = getConfig("Configuration/Chemin");
    if(getConfig("Dimension/Fullscreen").toInt()) {
        this->setWindowState(this->windowState()|Qt::WindowMaximized);
    } else {
        this->setGeometry(getConfig("Dimension/X", 724), getConfig("Dimension/Y", 303), getConfig("Dimension/W", 472), getConfig("Dimension/H", 434));
    }

    lien = ui->pageConfigurationLineEditSite->text() + getConfig("Configuration/Extension") + "/engine/search?q=";

    log->ecrire("FenetrePrincipale::chargementConfiguration() : Fin du chargement du fichier de configuration");
}

void FenetrePrincipale::majEpisode() {
    log->ecrire("FenetrePrincipale::majEpisode() : Début de mise à jour des episodes de la veille");

    QDate dateDerniereOuverture = bdd->derniereOuvertureBDD();
    QDate dateDerniereOuverturePlus1 = dateDerniereOuverture.addDays(1);

    while(dateDerniereOuverture <= QDate::currentDate().addDays(-1)) {
        QList<QString> champs;
        QList<QString> conditions;
        QList<QString> ordres;
        champs.append(FICHE_SERIE_ID);
        champs.append(FICHE_SERIE_NOM);
        champs.append(FICHE_SERIE_WIKI);
        champs.append(SAISON_SAISON);
        champs.append(SAISON_NB_EPISODE);
        champs.append(SAISON_EPISODE_COURANT);
        champs.append(SAISON_DATE_SORTIE);
        champs.append(SAISON_JOUR_SORTIE);
        champs.append(SAISON_DATE_MODIF);
        champs.append(SAISON_WIKI);
        conditions.append(FICHE_SERIE_ID + " = " + SAISON_ID);
        conditions.append(SAISON_DATE_MODIF + " = '" + methodeDiverses.dateToString(dateDerniereOuverture.addDays(-7)) + "'");
        QList<QMap<QString,QString> > liste = bdd->requeteSelect(champs, FICHE_SERIE_TABLE + ", " + SAISON_TABLE, conditions, ordres);

        if(!liste.isEmpty()) {
            for (int i = 0; i < liste.count(); ++i) {
                QMap<QString, QString> list = liste.value(i);

                log->ecrire("\tMise à jour de " + list.value(FICHE_SERIE_NOM));
                champs.clear();
                conditions.clear();
                champs.append(SAISON_EPISODE_COURANT + " = '" + methodeDiverses.formalismeEntier(list.value(SAISON_EPISODE_COURANT).toInt() + 1) + "'");
                champs.append(SAISON_DATE_MODIF + " = '" + dateDerniereOuverture.toString("yyyy-MM-dd") + "'");
                conditions.append(SAISON_ID + " = '" + list.value(FICHE_SERIE_ID) + "'");
                bdd->requeteUpdate(champs, SAISON_TABLE, conditions);

                champs.clear();
                QList<QString> valeurs;
                champs.append(HISTORIQUE_NOM);
                champs.append(HISTORIQUE_SAISON);
                champs.append(HISTORIQUE_EPISODE);
                champs.append(HISTORIQUE_DATE_AJOUT);
                valeurs.append(list.value(FICHE_SERIE_NOM));
                valeurs.append(list.value(SAISON_SAISON));
                valeurs.append(list.value(SAISON_EPISODE_COURANT));
                valeurs.append(dateDerniereOuverture.toString("yyyy-MM-dd"));
                bdd->requeteInsert(champs, valeurs, HISTORIQUE_TABLE);
            }
        }
        // Verifie si une série est terminé
        verificationSerieTerminer();

        dateDerniereOuverture = dateDerniereOuverture.addDays(1);
        dateDerniereOuverturePlus1 = dateDerniereOuverturePlus1.addDays(1);
    }
    bdd->majDerniereOuvertureBDD();
    log->ecrire("FenetrePrincipale::majEpisode() : Fin de mise à jour des episodes de la veille");
}

void FenetrePrincipale::redimensionnerTableau() {
    // Paramètrage de la taille minimum du tableau
    // Tableau principale
    // Aujourd'hui
    int tailleMax = 2;
    for(int i = 0; i < ui->pagePrincipaleTableWidgetDisplay->columnCount(); i++) {
        tailleMax += ui->pagePrincipaleTableWidgetDisplay->columnWidth(i);
    }
    tailleMax += ui->pagePrincipaleTableWidgetDisplay->verticalHeader()->width();
    int hauteurLigne = 0;
    for(int i = 0; i < ui->pagePrincipaleTableWidgetDisplay->rowCount(); i++) {
        hauteurLigne += ui->pagePrincipaleTableWidgetDisplay->rowHeight(i);
    }
    int hauteurTotal = hauteurLigne + ui->pagePrincipaleTableWidgetDisplay->horizontalHeader()->height();
    if(hauteurTotal > ui->pagePrincipaleTableWidgetDisplay->height()) {
        tailleMax += ui->pagePrincipaleTableWidgetDisplay->verticalScrollBar()->width();
    }
    ui->pagePrincipaleTableWidgetDisplay->setMinimumWidth(tailleMax);

    // Hier
    tailleMax = 2;
    for(int i = 0; i < ui->pagePrincipaleTableWidgetDisplay_2->columnCount(); i++) {
        tailleMax += ui->pagePrincipaleTableWidgetDisplay_2->columnWidth(i);
    }
    tailleMax += ui->pagePrincipaleTableWidgetDisplay_2->verticalHeader()->width();
    hauteurLigne = 0;
    for(int i = 0; i < ui->pagePrincipaleTableWidgetDisplay_2->rowCount(); i++) {
        hauteurLigne += ui->pagePrincipaleTableWidgetDisplay_2->rowHeight(i);
    }
    hauteurTotal = hauteurLigne + ui->pagePrincipaleTableWidgetDisplay_2->horizontalHeader()->height();
    if(hauteurTotal > ui->pagePrincipaleTableWidgetDisplay_2->height()) {
        tailleMax += ui->pagePrincipaleTableWidgetDisplay_2->verticalScrollBar()->width();
    }
    ui->pagePrincipaleTableWidgetDisplay_2->setMinimumWidth(tailleMax);
    // Tableau vos series
    tailleMax = 2;
    for(int i = 0; i < ui->pageVosSeriesDisplay->columnCount(); i++) {
        tailleMax += ui->pageVosSeriesDisplay->columnWidth(i);
    }
    tailleMax += ui->pageVosSeriesDisplay->verticalHeader()->width();
    hauteurLigne = 0;
    for(int i = 0; i < ui->pageVosSeriesDisplay->rowCount(); i++) {
        hauteurLigne += ui->pageVosSeriesDisplay->rowHeight(i);
    }
    hauteurTotal = hauteurLigne + ui->pageVosSeriesDisplay->horizontalHeader()->height();
    if(hauteurTotal > ui->pageVosSeriesDisplay->height()) {
        tailleMax += ui->pageVosSeriesDisplay->verticalScrollBar()->width();
    }
    ui->pageVosSeriesDisplay->setMinimumWidth(tailleMax);
    // Tableau historique
    tailleMax = 2;
    for(int i = 0; i < ui->pageHistoriqueTableWidget->columnCount(); i++) {
        tailleMax += ui->pageHistoriqueTableWidget->columnWidth(i);
    }
    tailleMax += ui->pageHistoriqueTableWidget->verticalHeader()->width();
    hauteurLigne = 0;
    for(int i = 0; i < ui->pageHistoriqueTableWidget->rowCount(); i++) {
        hauteurLigne += ui->pageHistoriqueTableWidget->rowHeight(i);
    }
    hauteurTotal = hauteurLigne + ui->pageHistoriqueTableWidget->horizontalHeader()->height();
    if(hauteurTotal > ui->pageHistoriqueTableWidget->height()) {
        tailleMax += ui->pageHistoriqueTableWidget->verticalScrollBar()->width();
    }
    ui->pageHistoriqueTableWidget->setMinimumWidth(tailleMax);
    // Tableau Liste Saison
    tailleMax = 2;
    for(int i = 0; i < ui->pageAjoutModifTableWidgetListeSaison->columnCount(); i++) {
        tailleMax += ui->pageAjoutModifTableWidgetListeSaison->columnWidth(i);
    }
    tailleMax += ui->pageAjoutModifTableWidgetListeSaison->verticalHeader()->width();
    hauteurLigne = 0;
    for(int i = 0; i < ui->pageAjoutModifTableWidgetListeSaison->rowCount(); i++) {
        hauteurLigne += ui->pageAjoutModifTableWidgetListeSaison->rowHeight(i);
    }
    hauteurTotal = hauteurLigne + ui->pageAjoutModifTableWidgetListeSaison->horizontalHeader()->height();
    if(hauteurTotal > ui->pageAjoutModifTableWidgetListeSaison->height()) {
        tailleMax += ui->pageAjoutModifTableWidgetListeSaison->verticalScrollBar()->width();
    }
    ui->pageAjoutModifTableWidgetListeSaison->setMinimumWidth(tailleMax);
}

void FenetrePrincipale::verificationSerieTerminer() {
    log->ecrire("FenetrePrincipale::verificationSerieTerminer() : Début de vérification des séries terminées");
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;
    champs.append(FICHE_SERIE_ID);
    champs.append(FICHE_SERIE_NOM);
    champs.append(SAISON_SAISON);
    champs.append(SAISON_NB_EPISODE);
    champs.append(SAISON_EPISODE_COURANT);
    conditions.append(FICHE_SERIE_ID + " = " + SAISON_ID);
    QList<QMap<QString,QString> > liste = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE + ", " + SAISON_TABLE, conditions, ordres);
    for (int i = 0; i < liste.count(); ++i) {
        QMap<QString,QString> list = liste.value(i);

        if(list.value(SAISON_NB_EPISODE).toInt() < list.value(SAISON_EPISODE_COURANT).toInt()){
            log->ecrire("\tSuppression de " + list.value(FICHE_SERIE_NOM));
            conditions.clear();
            conditions.append(SAISON_ID + " = '" + list.value(FICHE_SERIE_ID) + "'");
            bdd->requeteDelete(SAISON_TABLE, conditions);
        }
    }
    log->ecrire("FenetrePrincipale::verificationSerieTerminer() : Fin de vérification des séries terminées");
}

QString FenetrePrincipale::getConfig(QString config) {
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    return settings.value(config).toString();
}

int FenetrePrincipale::getConfig(QString config, int valeur) {
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    return settings.value(config, valeur).toInt();
}

void FenetrePrincipale::setConfig(QString config, QString valeur) {
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    settings.setValue(config, valeur);
}

QString FenetrePrincipale::getDossierSerie() {
    return this->dossierSerie;
}

void FenetrePrincipale::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
    if(getConfig("Dimension/W", 472) != this->geometry().width())
        setConfig("Dimension/W", QString::number(this->geometry().width()));
    if(getConfig("Dimension/H", 434) != this->geometry().height())
        setConfig("Dimension/H", QString::number(this->geometry().height()));
    if(getConfig("Dimension/X",724) != this->geometry().x())
        setConfig("Dimension/X", QString::number(this->geometry().x()));
    if(getConfig("Dimension/Y", 303) != this->geometry().y())
        setConfig("Dimension/Y",QString::number(this->geometry().y()));
    if(getConfig("Dimension/Fullscreen", 0) != this->isMaximized())
        setConfig("Dimension/Fullscreen", QString::number(this->isMaximized()));
}

void FenetrePrincipale::resizeEvent(QResizeEvent *event) {
    redimensionnerTableau();
}

void FenetrePrincipale::loadImage() {
    QPixmap image;
    image.loadFromData(m_pImgCtrl->downloadedData());
    QDir dir;
    if(image.save(dir.currentPath() + "/data/images/" + ui->pageAjoutModifComboFicheSerieNom->currentText() + ".jpg")) {
        ui->pageAjoutModifLabelPhoto->setPixmap(image.scaled(ui->pageAjoutModifLabelPhoto->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->pageAjoutModifLabelCheminPhoto->setText(QFileInfo("/data/images/" + ui->pageAjoutModifComboFicheSerieNom->currentText() + ".jpg").absoluteFilePath());
    } else {
        QMessageBox::information(this, this->windowTitle(), "Impossible de télécharger l'image !\nChoississez une autre image ou enregistrer l'image sur votre ordinateur.");
    }

}

/*******************************************************\
*                                                       *
*                          MENU                         *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_menuFichierAccueil_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pagePrincipale)
        ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
}

void FenetrePrincipale::on_menuFichierHistoriqueRechercher_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageHistoriqueRechercher) {
        ui->stackedWidget->setCurrentWidget(ui->pageHistoriqueRechercher);
        ui->pageHistoriqueRechercherComboBoxNomSerie->setCurrentIndex(0);
        ui->pageHistoriqueRechercherLabelAffichage->setText("");
        ui->pageHistoriqueRechercherTableWidget->setRowCount(0);
        QList<QString> champs;
        QList<QString> conditions;
        QList<QString> ordres;
        champs.append("DISTINCT " + HISTORIQUE_NOM);
        QList<QMap<QString, QString> > liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
        for(int i = 0; i < liste.count(); i++) {
            ui->pageHistoriqueRechercherComboBoxNomSerie->addItem(liste.at(i).value("DISTINCT " + HISTORIQUE_NOM));
        }
        ui->pageHistoriqueRechercherTableWidget->resizeColumnsToContents();
    }
}

void FenetrePrincipale::on_menuFichierHistoriqueAjouter_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageHistoriqueAjouter) {
        ui->stackedWidget->setCurrentWidget(ui->pageHistoriqueAjouter);
        ui->pageHistoriqueAjouterPushButtonAjouter->setEnabled(false);
        ui->pageHistoriqueAjouterComboBoxSerie->setCurrentIndex(0);
        ui->pageHistoriqueAjouterTableWidget->setRowCount(0);
        QList<QString> champs;
        QList<QString> conditions;
        QList<QString> ordres;
        champs.append(FICHE_SERIE_NOM);
        QList<QMap<QString,QString> > liste = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres);
        for(int i = 0; i < liste.count(); i++) {
            ui->pageHistoriqueAjouterComboBoxSerie->addItem(liste.at(i).value(FICHE_SERIE_NOM));
        }
    }
}

void FenetrePrincipale::on_menuFichierHistoriqueModifier_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageHistoriqueModifier) {
        ui->stackedWidget->setCurrentWidget(ui->pageHistoriqueModifier);
        ui->pageHistoriqueModifierComboBoxRechercheNom->setCurrentIndex(0);
        QList<QString> champs;
        QList<QString> conditions;
        QList<QString> ordres;
        champs.append("DISTINCT " + HISTORIQUE_NOM);
        QList<QMap<QString, QString> > liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
        ui->pageHistoriqueModifierComboBoxRechercheNom->clear();
        for(int i = 0; i < liste.count(); i++) {
            ui->pageHistoriqueModifierComboBoxRechercheNom->addItem(liste.at(i).value("DISTINCT " + HISTORIQUE_NOM));
        }
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setDate(QDate());
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setEnabled(false);

        on_pageHistoriqueModifierComboBoxRechercheNom_currentTextChanged(ui->pageHistoriqueModifierComboBoxRechercheNom->currentText());
        on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged(ui->pageHistoriqueModifierSpinBoxRechercheSaison->value());
    }
}

void FenetrePrincipale::on_menuFichierHistoriqueConsulter_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageHistorique)
        ui->stackedWidget->setCurrentWidget(ui->pageHistorique);
}

void FenetrePrincipale::on_menuFichierHistoriqueImporter_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageHistoriqueImporter) {
        ui->stackedWidget->setCurrentWidget(ui->pageHistoriqueImporter);
        ui->pageHistoriqueImporterLineEditCheminFichier->setText("");
        ui->pageHistoriqueImporterPushButtonImporter->setEnabled(false);
        ui->pageHistoriqueImporterPushButtonChargerFichier->setEnabled(false);
        ui->pageHistoriqueImporterTableWidget->setRowCount(0);
    }
}

void FenetrePrincipale::on_menuFichierToutes_les_s_ries_triggered() {
    if(listeGlobal.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageVosSeries);
        on_pageVosSeriesComboBox_currentIndexChanged(ui->pageVosSeriesComboBox->currentText());
    } else {
        QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de série !");
    }
}

void FenetrePrincipale::on_menuFichierAjouter_triggered() {
    ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
    ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
    ui->pageAjoutModifBoutonValider->setText("Ajouter");
    ui->pageAjoutModifComboFicheSerieNom->clear();
    ui->pageAjoutModifComboFicheSerieNom->setEditable(true);
    ui->pageAjoutModifCheckBoxCreerSaison->setChecked(true);
    // Ajout des séries dans le comboBox
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;
    champs.append(FICHE_SERIE_NOM);
    conditions.append(FICHE_SERIE_NOM + " NOT IN (SELECT " + FICHE_SERIE_NOM + " FROM " + FICHE_SERIE_TABLE + " INNER JOIN " + SAISON_TABLE + " ON " + FICHE_SERIE_ID + " = " + SAISON_ID + ")");
    QList<QMap<QString, QString> > serieSansSaison = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres);
    if(!serieSansSaison.isEmpty())
        ui->pageAjoutModifComboFicheSerieNom->addItem("");
    for(int i = 0; i < serieSansSaison.count(); i++) {
        ui->pageAjoutModifComboFicheSerieNom->addItem(serieSansSaison.at(i).value(FICHE_SERIE_NOM));
    }
    // Mise a zéro des champs
    ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
    ui->pageAjoutModifLabelCheminPhoto->setText("");
    ui->pageAjoutModifComboFicheSerieNom->setCurrentIndex(0);
    ui->pageAjoutModifLineEditFicheSerieWiki->setText("");
    ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
    ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(ui->pageAjoutModifSpinBoxSaisonNbEpisode->minimum());
    ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
    ui->pageAjoutModifLineEditSaisonWiki->setText("");
    ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
    ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());

    //Gestion des visibilité
    ui->pageAjoutModifGroupBoxSaison->setVisible(true);
    ui->pageAjoutModifPushButtonSupprimerSaison->setEnabled(false);
    ui->pageAjoutModifPushButtonSupprimerSaison->setVisible(false);
    ui->pageAjoutModifBoutonValider->setEnabled(false);
    ui->pageAjoutModifBoutonDossier->setVisible(false);
    ui->pageAjoutModifBoutonWiki->setVisible(false);
    ui->pageAjoutModifComboBoxListeSaison->setVisible(false);
    ui->pageAjoutModifTableWidgetListeSaison->setVisible(false);
    ui->pageAjoutModifLabelEpisodeCourant->setVisible(false);
    ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setVisible(false);
    ui->pageAjoutModifLabelDateProchain->setVisible(false);
    ui->pageAjoutModifDateEditSaisonDateProchain->setVisible(false);
    ui->pageAjoutModifLabelDateProchain_2->setVisible(false);
    ui->pageAjoutModifLabelCreationDossierAuto->setVisible(true);
    ui->pageAjoutModifCheckBoxCreationDossierAuto->setVisible(true);
    ui->pageAjoutModifCheckBoxCreerSaison->setVisible(true);
    ui->pageAjoutModifLabelCreerSaison->setVisible(true);
    ui->pageAjoutModifPushButtonParcourirPhoto->setEnabled(false);
    ui->pageAjoutModifLineEditDepuisURL->setEnabled(false);
}

void FenetrePrincipale::on_menuFichierModifier_triggered() {
    ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
    ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
    ui->pageAjoutModifBoutonValider->setText("Modifier");
    ui->pageAjoutModifComboFicheSerieNom->clear();
    ui->pageAjoutModifComboFicheSerieNom->setEditable(false);
    QList<QString> champs;
    QList<QString> conditions;
    champs.append(FICHE_SERIE_NOM);
    // Ajout des  Serie dans le comboBox
    if(!ficheSerie.isEmpty())
        ui->pageAjoutModifComboFicheSerieNom->addItem("");
    for(int i = 0; i < ficheSerie.count(); i++) {
        ui->pageAjoutModifComboFicheSerieNom->addItem(ficheSerie.at(i));
    }

    if(ui->pageAjoutModifBoutonValider->text() == "Modifier" && ui->pageAjoutModifComboFicheSerieNom->currentText() == "") {
        ui->pageAjoutModifGroupBoxSaison->setVisible(false);
        ui->pageAjoutModifPushButtonSupprimerSaison->setEnabled(false);
        ui->pageAjoutModifPushButtonSupprimerSaison->setVisible(true);
        ui->pageAjoutModifBoutonValider->setEnabled(false);
        ui->pageAjoutModifBoutonWiki->setEnabled(false);
    }

    // Mise a zéro des champs
    ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
    ui->pageAjoutModifLabelCheminPhoto->setText("");
    ui->pageAjoutModifComboFicheSerieNom->setCurrentIndex(0);
    ui->pageAjoutModifLineEditFicheSerieWiki->setText("");
    ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
    ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(ui->pageAjoutModifSpinBoxSaisonNbEpisode->minimum());
    ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
    ui->pageAjoutModifLineEditSaisonWiki->setText("");
    ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
    ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());

    //Gestion des visibilité
    ui->pageAjoutModifBoutonDossier->setVisible(true);
    ui->pageAjoutModifBoutonDossier->setEnabled(false);
    ui->pageAjoutModifBoutonWiki->setVisible(true);
    ui->pageAjoutModifComboBoxListeSaison->setVisible(true);
    ui->pageAjoutModifTableWidgetListeSaison->setVisible(true);
    ui->pageAjoutModifLabelEpisodeCourant->setVisible(true);
    ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setVisible(true);
    ui->pageAjoutModifLabelDateProchain->setVisible(true);
    ui->pageAjoutModifDateEditSaisonDateProchain->setVisible(true);
    ui->pageAjoutModifLabelDateProchain_2->setVisible(true);
    ui->pageAjoutModifLabelCreationDossierAuto->setVisible(false);
    ui->pageAjoutModifCheckBoxCreationDossierAuto->setVisible(false);
    ui->pageAjoutModifCheckBoxCreerSaison->setVisible(false);
    ui->pageAjoutModifLabelCreerSaison->setVisible(false);
    ui->pageAjoutModifPushButtonParcourirPhoto->setEnabled(false);
    ui->pageAjoutModifLineEditDepuisURL->setEnabled(false);
}

void FenetrePrincipale::on_menuFichierQuitter_triggered() {
    log->ecrire("Fermeture de l'application");
    closeEvent(new QCloseEvent());
    QCoreApplication::quit();
}

void FenetrePrincipale::on_menuOptionsActualiser_triggered() {
    refresh();
}

void FenetrePrincipale::on_menuOptionsParam_tres_triggered() {
    ui->stackedWidget->setCurrentWidget(ui->pageConfig);
    ui->pageConfigurationLabelConsigne->setVisible(false);
    ui->pageConfigurationSpinBoxLogEfface->setValue(getConfig("Configuration/PurgeLog").toInt());
    ui->pageConfigurationLineDossierTelechargement->setText(getConfig("Configuration/Telechargement"));
    ui->pageConfigurationLineDossierTelechargement->setCursorPosition(0);
    ui->pageConfigurationLineDossierSerie->setText(getConfig("Configuration/Chemin"));
    switch (getConfig("Configuration/Qualite").toInt()) {
    case 0:
        ui->pageConfigurationRadioButtonQualiteToutes->setChecked(true);
        break;
    case 1:
        ui->pageConfigurationRadioButtonQualite720->setChecked(true);
        break;
    case 2:
        ui->pageConfigurationRadioButtonQualite1080->setChecked(true);
        break;
    }
    switch (getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        ui->pageConfigurationRadioButtonSousTitresToutes->setChecked(true);
        break;
    case 1:
        ui->pageConfigurationRadioButtonSousTitresVOSTFR->setChecked(true);
        break;
    }
    ui->pageConfigurationLineEditExtension->setText(getConfig("Configuration/Extension"));
}

void FenetrePrincipale::on_menuOptionsDossier_de_log_triggered() {
#ifdef QT_DEBUG
    QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs"));
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs"));
#endif
}

void FenetrePrincipale::on_menuAideA_Propos_triggered() {
    QMessageBox msgBox;
    msgBox.setText("Version " + this->version + "\nBuild du " + this->dateVersion.toString("dd/MM/yyyy") + " - " + this->heureVersion.toString("hh:mm:ss")+ "<br/><br/>"
                   "Le projet Series Manager est venu le jour où le nombre de séries "
                   "que je regardais commençait à atteindre un nombre impressionnant.<br/><br/>"
                   "J'ai donc eu l'idée de créé un programme permettant de lister les "
                   "séries de la saison et de créé automatiquement les liens de recherches "
                   "sur le site de téléchargement Zone de téléchargement.<br/><br/>"
                   "&copy;2014-2017 Eranos Corporation. Tout droits réservés.<br/><br/>"
                   "Ce projet a été développé sous Qt Creator 4.2.1<br/>"
                   "Plus d'info sur Qt : <a href='https://www.qt.io/'>https://www.qt.io/</a><br/>");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setIconPixmap(i_seriesManager);
    msgBox.exec();
}

/*******************************************************\
*                                                       *
*                       TOOLBAR                         *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_toolBarAccueil_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pagePrincipale) {
        on_menuFichierAccueil_triggered();
    }
}

void FenetrePrincipale::on_toolBarParam_tres_triggered() {
    on_menuOptionsParam_tres_triggered();
}

void FenetrePrincipale::on_toolBarActualiser_triggered() {
    on_menuOptionsActualiser_triggered();
}

void FenetrePrincipale::on_toolBarQuitter_triggered() {
    on_menuFichierQuitter_triggered();
}

void FenetrePrincipale::on_toolBarAjouter_triggered() {
    if((ui->stackedWidget->currentWidget() == ui->pageAjoutModification && ui->pageAjoutModifBoutonValider->text() != "Ajouter") || (ui->stackedWidget->currentWidget() != ui->pageAjoutModification)) {
        on_menuFichierAjouter_triggered();
    } else {
        ui->statusBar->showMessage("Vous etes déjà sur cette page", 5000);
    }
}

void FenetrePrincipale::on_toolBarModifier_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageAjoutModification || ui->pageAjoutModifBoutonValider->text() != "Modifier") {
        on_menuFichierModifier_triggered();
    } else {
        ui->statusBar->showMessage("Vous etes déjà sur cette page", 5000);
    }
}

void FenetrePrincipale::on_toolBarDeplcerFichiers_triggered() {
    on_pagePrincipaleBoutonDeplacerFichier_clicked();
}

/*******************************************************\
*                                                       *
*                  PAGE CONFIGURATION                   *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageConfigurationBoutonParcourir_clicked() {
    ui->pageConfigurationLineDossierSerie->setText(QFileDialog::getExistingDirectory(this, tr("Choisir un dossier"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QFileDialog::ShowDirsOnly));
}

void FenetrePrincipale::on_pageConfigurationBoutonParcourir_2_clicked() {
    ui->pageConfigurationLineDossierTelechargement->setText(QFileDialog::getExistingDirectory(this, tr("Choisir un dossier"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QFileDialog::ShowDirsOnly));
}

void FenetrePrincipale::on_pageConfigurationBoutonTerminer_clicked() {
    if(ui->pageConfigurationLineDossierSerie->text() != "" && ui->pageConfigurationLineDossierTelechargement->text() != "" && ui->pageConfigurationLineEditExtension->text() != "") {
        setConfig("Configuration/Chemin", ui->pageConfigurationLineDossierSerie->text());
        setConfig("Configuration/Extension", ui->pageConfigurationLineEditExtension->text());
        lien = ui->pageConfigurationLineEditSite->text() + ui->pageConfigurationLineEditExtension->text() + "/engine/search?q=";
        setConfig("Configuration/ListeSerie", "Toutes");
        setConfig("Configuration/PurgeLog", QString::number(ui->pageConfigurationSpinBoxLogEfface->value()));
        if(ui->pageConfigurationRadioButtonQualiteToutes->isChecked()) {
            setConfig("Configuration/Qualite", "0");
        } else if(ui->pageConfigurationRadioButtonQualite720->isChecked()) {
            setConfig("Configuration/Qualite", "1");
        } else if(ui->pageConfigurationRadioButtonQualite1080->isChecked()) {
            setConfig("Configuration/Qualite", "2");
        }
        if(ui->pageConfigurationRadioButtonSousTitresToutes->isChecked()) {
            setConfig("Configuration/SousTitres", "0");
        } else {
            setConfig("Configuration/SousTitres", "1");
        }
        setConfig("Configuration/Telechargement", ui->pageConfigurationLineDossierTelechargement->text());
        setConfig("Dimension/Fullscreen", QString::number(this->isMaximized()));
        setConfig("Dimension/H", QString::number(this->geometry().height()));
        setConfig("Dimension/W", QString::number(this->geometry().width()));
        setConfig("Dimension/X", QString::number(this->geometry().x()));
        setConfig("Dimension/Y", QString::number(this->geometry().y()));
        if(!ui->mainToolBar->isVisible()) {
            initialisation();
        }
        dossierSerie =  ui->pageConfigurationLineDossierSerie->text();
        ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
    } else {
        QMessageBox::warning(this, this->windowTitle(), "Veuillez remplir toutes les informations demandées !");
    }
}

/*******************************************************\
*                                                       *
*                   PAGE PRINCIPALE                     *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pagePrincipaleTableWidgetDisplay_doubleClicked(const QModelIndex &index) {
    on_menuFichierModifier_triggered();
    ui->pageAjoutModifComboFicheSerieNom->setCurrentText(ui->pagePrincipaleTableWidgetDisplay->item(index.row(), 0)->text());
    ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
}

void FenetrePrincipale::on_pagePrincipaleTableWidgetDisplay_2_doubleClicked(const QModelIndex &index) {
    on_menuFichierModifier_triggered();
    ui->pageAjoutModifComboFicheSerieNom->setCurrentText(ui->pagePrincipaleTableWidgetDisplay_2->item(index.row(), 0)->text());
    ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
}

void FenetrePrincipale::on_pagePrincipaleTableWidgetDisplay_customContextMenuRequested(const QPoint &pos) {
    if(ui->pagePrincipaleTableWidgetDisplay->rowCount() > 0 && !ui->pagePrincipaleTableWidgetDisplay->selectedRanges().isEmpty()) {
        QMenu menuContextuel(this);
        QAction *copier = menuContextuel.addAction(QIcon(), "Copier le nom de la série");
        QAction *copierComplet = menuContextuel.addAction(QIcon(), "Copier le nom, la saison et l'épisode de la série");
        QAction *dossier;
        if(QDir(this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 1)->text()).exists()) {
            menuContextuel.addSeparator();
            dossier = menuContextuel.addAction(QIcon(), "Ouvrir le dossier de la série");
        }

        QAction *a = menuContextuel.exec(ui->pagePrincipaleTableWidgetDisplay->viewport()->mapToGlobal(pos));

        if(a == copier) {
            QApplication::clipboard()->setText(ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 0)->text());
        } else if(a == copierComplet) {
            QApplication::clipboard()->setText(ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 0)->text() + " S" + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 1)->text() + "E" + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 2)->text());
        } else if(a == dossier) {
            if(QDesktopServices::openUrl(QUrl::fromLocalFile(this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 1)->text()))) {
                log->ecrire("Ouverture du dossier de " + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 0)->text());
            } else {
                QMessageBox::warning(this, this->windowTitle(), "Le dossier n'a pas pu être ouvert");
                log->ecrire("Le dossier n'a pas pu être ouvert");
            }
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleTableWidgetDisplay_2_customContextMenuRequested(const QPoint &pos) {
    if(ui->pagePrincipaleTableWidgetDisplay_2->rowCount() > 0 && !ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().isEmpty()) {
        QMenu menuContextuel(this);
        QAction *copier = menuContextuel.addAction(QIcon(), "Copier le nom de la série");
        QAction *copierComplet = menuContextuel.addAction(QIcon(), "Copier le nom, la saison et l'épisode de la série");
        QAction *dossier;
        if(QDir(this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 1)->text()).exists()) {
            menuContextuel.addSeparator();
            dossier = menuContextuel.addAction(QIcon(), "Ouvrir le dossier de la série");
        }

        QAction *a = menuContextuel.exec(ui->pagePrincipaleTableWidgetDisplay_2->viewport()->mapToGlobal(pos));

        if(a == copier) {
            QApplication::clipboard()->setText(ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text());
        } else if(a == copierComplet) {
            QApplication::clipboard()->setText(ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text() + " S" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 1)->text() + "E" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 2)->text());
        } else if(a == dossier) {
            if(QDesktopServices::openUrl(QUrl::fromLocalFile(this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 1)->text()))) {
                log->ecrire("Ouverture du dossier de " + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text());
            } else {
                QMessageBox::warning(this, this->windowTitle(), "Le dossier n'a pas pu être ouvert");
                log->ecrire("Le dossier n'a pas pu être ouvert");
            }
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleBoutonVosSeries_clicked() {
    on_menuFichierToutes_les_s_ries_triggered();
}

void FenetrePrincipale::on_pagePrincipaleBoutonAjouter_clicked() {
    on_menuFichierAjouter_triggered();
}

void FenetrePrincipale::on_pagePrincipaleBoutonModifier_clicked() {
    on_menuFichierModifier_triggered();
}

void FenetrePrincipale::on_pagePrincipaleBoutonUrl_clicked() {
    if(this->listeQuoti.isEmpty()){
        QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de série à regarder !");
    } else {
        QString qualite;
        QString sousTitres;
        QString fin;

        switch (getConfig("Configuration/Qualite").toInt()) {
        case 0:
            qualite = "";
            break;
        case 1:
            qualite = "+720p";
            break;
        case 2:
            qualite = "+1080p";
            break;
        }

        switch (getConfig("Configuration/SousTitres").toInt()) {
        case 0:
            sousTitres = "";
            break;
        case 1:
            sousTitres = "+VOSTFR";
            break;
        }

        for (int i = 0; i < listeQuoti.count(); ++i) {
            QMap<QString,QString> list = listeQuoti.value(i);
            if(methodeDiverses.stringToDate(list.value(SAISON_DATE_SORTIE)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(SAISON_DATE_MODIF)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(SAISON_DATE_MODIF)) != QDate::currentDate()) {
                QDesktopServices::openUrl(lien + QString(list.value(FICHE_SERIE_NOM)).replace(" ","+") + "+S" + list.value(SAISON_SAISON) + "E" + list.value(SAISON_EPISODE_COURANT) + sousTitres + qualite + fin);
            }
        }
    }
}

void FenetrePrincipale::on_pagePrincipalePushButtonAddicted_clicked() {
    for(int i = 0; i < listeQuoti.count(); i++) {
        if(listeQuoti.at(i).value(FICHE_SERIE_ADDICTED) != "")
            bouton_lien_clicked(listeQuoti.at(i).value(FICHE_SERIE_ADDICTED) + "/" + QString::number(listeQuoti.at(i).value(SAISON_SAISON).toInt()) + "/" + QString::number(listeQuoti.at(i).value(SAISON_EPISODE_COURANT).toInt()) + "/8");
    }
}

void FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() {
    log->ecrire("FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() : Début de l'ouverture du dossier de téléchargement");
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(getConfig("Configuration/Telechargement")))) {
        log->ecrire("\tOuverture du dossier de téléchargement");
    } else {
        QMessageBox::warning(this, this->windowTitle(), "Le dossier de téléchargement n'a pas pu être ouvert");
        log->ecrire("\tLe dossier de téléchargement n'a pas pu être ouvert");
    }
    log->ecrire("FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() : Début de l'ouverture du dossier de téléchargement");
}

void FenetrePrincipale::on_pagePrincipaleBoutonWiki_clicked() {
    if(this->listeQuoti.isEmpty()){
        QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de série à regarder !");
    } else {
        for (int i = 0; i < listeQuoti.count(); ++i) {
            QMap<QString,QString> list = listeQuoti.value(i);
            if(methodeDiverses.stringToDate(list.value(SAISON_DATE_SORTIE)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(SAISON_DATE_MODIF)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(SAISON_DATE_MODIF)) != QDate::currentDate()) {
                if(!list.value(SAISON_WIKI).isEmpty()) {
                    QDesktopServices::openUrl(QUrl(list.value(SAISON_WIKI)));
                } else if (!list.value(FICHE_SERIE_WIKI).isEmpty()) {
                    QDesktopServices::openUrl(QUrl(list.value(FICHE_SERIE_WIKI)));
                }
            }
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleBoutonDeplacerFichier_clicked() {
    Dialog *dial = new Dialog(this);
    dial->open();
}

/*******************************************************\
*                                                       *
*                PAGE TOUTE LES SERIES                  *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageVosSeriesDisplay_doubleClicked(const QModelIndex &index) {
    on_menuFichierModifier_triggered();
    ui->pageAjoutModifComboFicheSerieNom->setCurrentText(ui->pageVosSeriesDisplay->item(index.row(), 0)->text());
    ui->pageAjoutModifLabelRetour->setText("pageVosSeries");
}

void FenetrePrincipale::on_pageVosSeriesComboBox_currentIndexChanged(const QString &arg1) {
    ui->pageVosSeriesDisplay->setRowCount(0);

    int selection = 1;
    if(getConfig("Configuration/ListeSerie") == "") {
        setConfig("Configuration/ListeSerie", "Toutes");
        selection = 1;
    } else {
        if(arg1 == "Toutes") {
            setConfig("Configuration/ListeSerie", "Toutes");
            selection = 1;
        } else if (arg1 == "Série à venir") {
            setConfig("Configuration/ListeSerie", "Série à venir");
            selection = 2;
        } else if (arg1 == "Série en cours") {
            setConfig("Configuration/ListeSerie", "Série en cours");
            selection = 3;
        }
    }

    int indice = 0;
    for (int i = 0; i < listeGlobal.count(); i++) {
        QMap<QString,QString> list = listeGlobal.value(i);

        if(selection == 2 && methodeDiverses.stringToDate(list.value(SAISON_DATE_SORTIE)) <= QDate::currentDate()) {
            continue;
        } else if (selection == 3 && methodeDiverses.stringToDate(list.value(SAISON_DATE_SORTIE)) > QDate::currentDate()) {
            continue;
        }

        ui->pageVosSeriesDisplay->setRowCount(indice + 1);
        ui->pageVosSeriesDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(list.value(FICHE_SERIE_NOM), false));
        ui->pageVosSeriesDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(list.value(SAISON_SAISON), true));
        ui->pageVosSeriesDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(methodeDiverses.dayToString(list.value(SAISON_JOUR_SORTIE).toInt()), true));

        if(methodeDiverses.stringToDate(list.value(SAISON_DATE_SORTIE)) <= QDate::currentDate() &&
           (selection == 1 || selection == 3)) {
            ui->pageVosSeriesDisplay->setItem(indice, 3, methodeDiverses.itemForTableWidget("En cours", true));
        } else {
            ui->pageVosSeriesDisplay->setItem(indice, 3, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list.value(SAISON_DATE_SORTIE)).toString("dd/MM/yy"), true));
        }

        if(list.value(SAISON_DATE_MODIF) == "0") {
            ui->pageVosSeriesDisplay->setItem(indice, 4, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list.value(SAISON_DATE_SORTIE)).toString("dd/MM/yy"), true));
        } else {
            ui->pageVosSeriesDisplay->setItem(indice, 4, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list.value(SAISON_DATE_MODIF)).addDays(7).toString("dd/MM/yy"), true));
        }

        QSignalMapper* mapper = new QSignalMapper();
        QPushButton* dossier = new QPushButton(i_dossier,"");
        mapper->setMapping(dossier, "/" + list.value(FICHE_SERIE_NOM) + "/Saison " + list.value(SAISON_SAISON));
        QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(bouton_dossier_clicked(QString)));
        QObject::connect(dossier, SIGNAL(clicked(bool)), mapper, SLOT(map()));
        ui->pageVosSeriesDisplay->setCellWidget(indice, 5, dossier);

        QFile file (dossierSerie + list.value(FICHE_SERIE_NOM));
        if(!file.exists()) {
            dossier->setEnabled(false);
        }

        QSignalMapper* mapper1 = new QSignalMapper();
        QPushButton* wiki = new QPushButton(i_wiki,"");
        if(list.value(SAISON_WIKI) != "") {
            mapper1->setMapping(wiki, list.value(SAISON_WIKI));
        } else if (list.value(FICHE_SERIE_WIKI) != "") {
            mapper1->setMapping(wiki, list.value(FICHE_SERIE_WIKI));
        } else {
            mapper1->setMapping(wiki, "");
            wiki->setEnabled(false);
        }
        QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(wiki, SIGNAL(clicked(bool)), mapper1, SLOT(map()));
        ui->pageVosSeriesDisplay->setCellWidget(indice, 6, wiki);


        indice++;
    }

    ui->pageVosSeriesDisplay->resizeColumnsToContents();
}

/*******************************************************\
*                                                       *
*               PAGE AJOUT MODIFICAITON                 *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() {
    int ficheSerieID;
    QString ficheSerieNom = ui->pageAjoutModifComboFicheSerieNom->currentText();
    QString ficheSerieWiki = ui->pageAjoutModifLineEditFicheSerieWiki->text();
    QString ficheSeriePhoto = ui->pageAjoutModifLabelCheminPhoto->text();
    QString ficheSerieAddicted = ui->pageAjoutModifLineEditAddicted->text();
    QString saisonSaison = methodeDiverses.formalismeEntier(ui->pageAjoutModifSpinBoxSaisonSaison->value());
    QString saisonNbEpisode = methodeDiverses.formalismeEntier(ui->pageAjoutModifSpinBoxSaisonNbEpisode->value());
    QString saisonDateSortie = methodeDiverses.dateToString(ui->pageAjoutModifDateEditSaisonDateSortie->date());
    int saisonJourSortie = ui->pageAjoutModifDateEditSaisonDateSortie->date().dayOfWeek();
    QString saisonWiki = ui->pageAjoutModifLineEditSaisonWiki->text();
    QString saisonDateModif;
    bool saisonCreationDossier;
    QString saisonEpisodeCourant;
    QString cheminFichier;

    if(QFile(ficheSeriePhoto).exists() && !QFileInfo("./data/images/" + QFileInfo(ficheSeriePhoto).fileName()).exists()) {
        QFile(ficheSeriePhoto).copy("./data/images/" + QFileInfo(ficheSeriePhoto).fileName());
    }
    if(QFileInfo("./data/images/" + QFileInfo(ficheSeriePhoto).fileName()).isFile()) {
        cheminFichier = QFileInfo("./data/images/" + QFileInfo(ficheSeriePhoto).fileName()).absoluteFilePath();
    } else {
        cheminFichier = "";
    }

    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;

    if(ui->pageAjoutModifBoutonValider->text() == "Ajouter") {
        QList<QString> valeurs;

        saisonDateModif = methodeDiverses.dateToString(ui->pageAjoutModifDateEditSaisonDateSortie->date().addDays(-7));
        saisonCreationDossier = ui->pageAjoutModifCheckBoxCreationDossierAuto->isChecked();

        champs.append(FICHE_SERIE_ID);
        conditions.append(FICHE_SERIE_NOM + " = '" + ficheSerieNom + "'");
        if(this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres).isEmpty()) {
            champs.clear();
            valeurs.clear();
            champs.append(FICHE_SERIE_NOM);
            valeurs.append(ficheSerieNom);
            champs.append(FICHE_SERIE_WIKI);
            valeurs.append(ficheSerieWiki);
            champs.append(FICHE_SERIE_IMAGE);
            valeurs.append(cheminFichier);
            champs.append(FICHE_SERIE_ADDICTED);
            valeurs.append(ficheSerieAddicted);

            this->bdd->requeteInsert(champs, valeurs, FICHE_SERIE_TABLE);
        } else {
            champs.clear();
            conditions.clear();
            champs.append(FICHE_SERIE_WIKI + " = '" + ficheSerieWiki + "'");
            champs.append(FICHE_SERIE_IMAGE + " ='" + ficheSeriePhoto + "'");
            champs.append(FICHE_SERIE_ADDICTED + " ='" + ficheSerieAddicted + "'");
            conditions.append(FICHE_SERIE_NOM + " = '" + ficheSerieNom + "'");
            this->bdd->requeteUpdate(champs, FICHE_SERIE_TABLE, conditions);
        }

        if(ui->pageAjoutModifCheckBoxCreerSaison->isChecked()) {
            // On récupère l'ID de la fiche série
            champs.clear();
            champs.append(FICHE_SERIE_ID);
            ficheSerieID = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres).at(0).value(FICHE_SERIE_ID).toInt();

            champs.clear();
            conditions.clear();
            champs.append(SAISON_ID);
            conditions.append(SAISON_ID + " = '" + QString::number(ficheSerieID) + "'");
            if(this->bdd->requeteSelect(champs, SAISON_TABLE, conditions, ordres).isEmpty()) {
                champs.clear();
                valeurs.clear();
                champs.append(SAISON_ID);
                valeurs.append(QString::number(ficheSerieID));
                champs.append(SAISON_SAISON);
                valeurs.append(saisonSaison);
                champs.append(SAISON_NB_EPISODE);
                valeurs.append(saisonNbEpisode);
                champs.append(SAISON_EPISODE_COURANT);
                valeurs.append("01");
                champs.append(SAISON_DATE_SORTIE);
                valeurs.append(saisonDateSortie);
                champs.append(SAISON_JOUR_SORTIE);
                valeurs.append(QString::number(saisonJourSortie));
                champs.append(SAISON_DATE_MODIF);
                valeurs.append(saisonDateModif);
                champs.append(SAISON_WIKI);
                valeurs.append(saisonWiki);
                this->bdd->requeteInsert(champs, valeurs, SAISON_TABLE);
                ui->statusBar->showMessage("La série " + ficheSerieNom + " a été ajouté.", 5000);
            } else {
                QMessageBox::information(this, this->windowTitle(), "Il existe déjà une saison pour cette série.\nLa saison n'a pas été créé.");
            }

            if(saisonCreationDossier) {
                QDir dir;
                dir.mkpath(dossierSerie + ficheSerieNom + "/Saison " + saisonSaison);
            }
        }
        champs.clear();
        conditions.clear();
        ui->pageAjoutModifComboFicheSerieNom->clear();
        champs.append(FICHE_SERIE_NOM);
        conditions.append(FICHE_SERIE_NOM + " NOT IN (SELECT " + FICHE_SERIE_NOM + " FROM " + FICHE_SERIE_TABLE + " INNER JOIN " + SAISON_TABLE + " ON " + FICHE_SERIE_ID + " = " + SAISON_ID + ")");
        QList<QMap<QString, QString> > serieSansSaison = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres);
        ui->pageAjoutModifComboFicheSerieNom->addItem("");
        for(int i = 0; i < serieSansSaison.count(); i++) {
            ui->pageAjoutModifComboFicheSerieNom->addItem(serieSansSaison.at(i).value(FICHE_SERIE_NOM));
        }
    } else {
        saisonDateModif = methodeDiverses.dateToString(ui->pageAjoutModifDateEditSaisonDateProchain->date().addDays(-7));
        saisonEpisodeCourant = methodeDiverses.formalismeEntier(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->value());
        champs.clear();
        conditions.clear();
        champs.append(FICHE_SERIE_ID);

        conditions.append(FICHE_SERIE_NOM + " = '" + ficheSerieNom + "'");

        ficheSerieID = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres).at(0).value(FICHE_SERIE_ID).toInt();

        champs.clear();
        conditions.clear();
        champs.append(FICHE_SERIE_NOM + " = '" + ficheSerieNom + "'");
        champs.append(FICHE_SERIE_IMAGE + " = '" + cheminFichier + "'");
        champs.append(FICHE_SERIE_WIKI + " = '" + ficheSerieWiki + "'");
        champs.append(FICHE_SERIE_ADDICTED + " = '" + ficheSerieAddicted + "'");
        conditions.append(FICHE_SERIE_ID + " = '" + QString::number(ficheSerieID) + "'");
        this->bdd->requeteUpdate(champs, FICHE_SERIE_TABLE, conditions);

        champs.clear();
        conditions.clear();

        champs.append(SAISON_SAISON + " = '" + saisonSaison + "'");
        champs.append(SAISON_NB_EPISODE + " = '" + saisonNbEpisode + "'");
        champs.append(SAISON_EPISODE_COURANT + " = '" + saisonEpisodeCourant + "'");
        champs.append(SAISON_DATE_SORTIE + " = '" + saisonDateSortie + "'");
        champs.append(SAISON_JOUR_SORTIE + " = '" + QString::number(saisonJourSortie) + "'");
        champs.append(SAISON_DATE_MODIF + " = '" + saisonDateModif + "'");
        champs.append(SAISON_WIKI + " = '" + saisonWiki + "'");
        conditions.append(SAISON_ID + " = '" + QString::number(ficheSerieID) + "'");
        this->bdd->requeteUpdate(champs, SAISON_TABLE, conditions);

        ui->statusBar->showMessage("La série " + ficheSerieNom + " a été modifié.", 5000);
    }
    refresh();
}

void FenetrePrincipale::on_pageAjoutModifPushButtonParcourirPhoto_clicked() {
    QString chemin = QFileDialog::getOpenFileName(this, tr("Choisir une image"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    if(!chemin.isEmpty()) {
        ui->pageAjoutModifLabelCheminPhoto->setText(chemin);
        ui->pageAjoutModifLineEditCheminPhoto->setText(chemin);
        ui->pageAjoutModifLabelPhoto->setPixmap(QPixmap(chemin).scaled(ui->pageAjoutModifLabelPhoto->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->pageAjoutModifLineEditDepuisURL->setText("");
    }
}

void FenetrePrincipale::on_pageAjoutModifBoutonDossier_clicked() {
    if(ui->pageAjoutModifComboFicheSerieNom->currentText() != "" && QDir(dossierSerie + "/" + ui->pageAjoutModifComboFicheSerieNom->currentText()).exists()) {
        QList<QString> champs;
        QList<QString> conditions;
        QList<QString> ordres;

        champs.append(FICHE_SERIE_ID);
        conditions.append(FICHE_SERIE_NOM + " = '" + ui->pageAjoutModifComboFicheSerieNom->currentText() + "'");

        QList<QMap<QString,QString> > liste = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres);
        champs.clear();
        conditions.clear();
        champs.append(SAISON_SAISON);
        conditions.append(SAISON_ID + " = '" + liste.at(0).value(FICHE_SERIE_ID) + "'");

        liste = this->bdd->requeteSelect(champs, SAISON_TABLE, conditions, ordres);

        if(liste.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + "/" + ui->pageAjoutModifComboFicheSerieNom->currentText()));
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + "/" + ui->pageAjoutModifComboFicheSerieNom->currentText() + "/Saison " + liste.at(0).value(SAISON_SAISON)));
        }
    }
}

void FenetrePrincipale::on_pageAjoutModifBoutonRetour_clicked() {
    if(ui->pageAjoutModifBoutonValider->text() == "Ajouter") {
        ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
    }
    if(ui->pageAjoutModifBoutonValider->text() == "Modifier") {
        if(ui->pageAjoutModifLabelRetour->text() == "pagePrincipale") {
            ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
        } else if(ui->pageAjoutModifLabelRetour->text() == "pageVosSeries") {
            ui->stackedWidget->setCurrentWidget(ui->pageVosSeries);
        }
    }
}

void FenetrePrincipale::on_pageAjoutModifDateEditSaisonDateProchain_dateChanged(const QDate &date) {
    ui->pageAjoutModifLabelDateProchain_2->setText(methodeDiverses.formatDate(date));
}

void FenetrePrincipale::on_pageAjoutModifDateEditSaisonDateSortie_dateChanged(const QDate &date) {
    ui->pageAjoutModifLabelDate->setText(methodeDiverses.formatDate(date));
    if(ui->pageAjoutModifDateEditSaisonDateSortie->isEnabled()) {
        ui->pageAjoutModifDateEditSaisonDateProchain->setDate(date);
    }
}

void FenetrePrincipale::on_pageAjoutModifBoutonWiki_clicked() {
    if(ui->pageAjoutModifLineEditSaisonWiki->text() != "") {
        QDesktopServices::openUrl(QUrl(ui->pageAjoutModifLineEditSaisonWiki->text()));
    } else if (ui->pageAjoutModifLineEditFicheSerieWiki->text() != "") {
        QDesktopServices::openUrl(QUrl(ui->pageAjoutModifLineEditFicheSerieWiki->text()));
    } else {
        QDesktopServices::openUrl(QUrl("https://fr.wikipedia.org"));
    }
}

void FenetrePrincipale::on_pageAjoutModifButtonWiki_clicked() {
    QDesktopServices::openUrl(QUrl("https://fr.wikipedia.org/wiki/Wikip%C3%A9dia:Accueil_principal"));
}

void FenetrePrincipale::on_pageAjoutModifCheckBoxCreerSaison_stateChanged(int arg1) {
    ui->pageAjoutModifGroupBoxSaison->setVisible(arg1);/*
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;
    champs.append(FICHE_SERIE_NOM);
    conditions.append(FICHE_SERIE_NOM + " = '" + ui->pageAjoutModifComboFicheSerieNom->currentText() + "'");
    QList<QMap<QString,QString> > liste = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres);
    if(liste.isEmpty()) {
        ui->pageAjoutModifBoutonValider->setEnabled(true);
    } else {
        if(arg1) {
            ui->pageAjoutModifBoutonValider->setEnabled(true);
        } else {
            ui->pageAjoutModifBoutonValider->setEnabled(false);
        }
    }*/
}

void FenetrePrincipale::on_pageAjoutModifPushButtonSupprimerSaison_clicked() {
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;

    champs.append(FICHE_SERIE_ID);
    conditions.append(FICHE_SERIE_NOM + " = '" + ui->pageAjoutModifComboFicheSerieNom->currentText() + "'");
    QString serieID = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres).at(0).value(FICHE_SERIE_ID);


    conditions.clear();
    conditions.append(SAISON_ID + " = '" + serieID + "'");
    this->bdd->requeteDelete(SAISON_TABLE, conditions);

    on_pageAjoutModifComboFicheSerieNom_currentTextChanged(ui->pageAjoutModifComboFicheSerieNom->currentText());
}

void FenetrePrincipale::on_pageAjoutModifComboBoxListeSaison_currentTextChanged(const QString &arg1) {
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;
    QString qualite;
    QString sousTitres;
    QString fin;
    champs.append(HISTORIQUE_NOM);
    champs.append(HISTORIQUE_SAISON);
    champs.append(HISTORIQUE_EPISODE);
    champs.append(HISTORIQUE_DATE_AJOUT);
    conditions.append(HISTORIQUE_NOM + " = '" + ui->pageAjoutModifComboFicheSerieNom->currentText() + "'");
    QString saison = arg1;
    conditions.append(HISTORIQUE_SAISON + " = '" + saison.replace("Saison ", "") + "'");

    switch (getConfig("Configuration/Qualite").toInt()) {
    case 0:
        qualite = "";
        break;
    case 1:
        qualite = "+720p";
        break;
    case 2:
        qualite = "+1080p";
        break;
    }

    switch (getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        sousTitres = "";
        break;
    case 1:
        sousTitres = "+VOSTFR";
        break;
    }

    QList<QMap<QString, QString> > listeEpisodeHistorique = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    ui->pageAjoutModifTableWidgetListeSaison->setRowCount(0);
    for(int i = 0; i < listeEpisodeHistorique.count(); i++) {
        ui->pageAjoutModifTableWidgetListeSaison->setRowCount(ui->pageAjoutModifTableWidgetListeSaison->rowCount() + 1);
        QMap<QString, QString> map = listeEpisodeHistorique.at(i);

        QSignalMapper* mapper = new QSignalMapper();
        QPushButton* url = new QPushButton(i_logo, "");
        url->setToolTip("Ouvre le lien URL de " + map.value(HISTORIQUE_NOM) + " pour l'épisode " + map.value(HISTORIQUE_EPISODE));
        mapper->setMapping(url, lien + QString(map.value(HISTORIQUE_NOM)).replace(" ","+") + "+S" + map.value(HISTORIQUE_SAISON) + "E" + map.value(HISTORIQUE_EPISODE) + sousTitres + qualite + fin);
        QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        QObject::connect(url, SIGNAL(clicked()), mapper, SLOT(map()));

        ui->pageAjoutModifTableWidgetListeSaison->setItem(i, 0, methodeDiverses.itemForTableWidget(map.value(HISTORIQUE_NOM), true));
        ui->pageAjoutModifTableWidgetListeSaison->setItem(i, 1, methodeDiverses.itemForTableWidget(map.value(HISTORIQUE_EPISODE), true));
        ui->pageAjoutModifTableWidgetListeSaison->setItem(i, 2, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(map.value(HISTORIQUE_DATE_AJOUT)).toString("dd/MM/yyyy"), true));
        ui->pageAjoutModifTableWidgetListeSaison->setCellWidget(i, 3 , url);
    }
}

void FenetrePrincipale::on_pageAjoutModifComboFicheSerieNom_currentTextChanged(const QString &arg1) {
    ui->pageAjoutModifTableWidgetListeSaison->setRowCount(0);
    ui->pageAjoutModifComboBoxListeSaison->clear();

    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;
    champs.append(FICHE_SERIE_WIKI);
    champs.append(FICHE_SERIE_IMAGE);
    champs.append(FICHE_SERIE_ADDICTED);
    champs.append(FICHE_SERIE_ID);
    conditions.append("UPPER(" + FICHE_SERIE_NOM + ") = UPPER('" + arg1 + "')");
    QList<QMap<QString, QString> > liste = this->bdd->requeteSelect(champs, FICHE_SERIE_TABLE, conditions, ordres);

    if(arg1 != "") {
        ui->pageAjoutModifBoutonValider->setEnabled(true);
        if(QDir(dossierSerie + "/" + arg1).exists()) {
            ui->pageAjoutModifBoutonDossier->setEnabled(true);
        } else {
            ui->pageAjoutModifBoutonDossier->setEnabled(false);
        }
        ui->pageAjoutModifPushButtonParcourirPhoto->setEnabled(true);
        ui->pageAjoutModifLineEditDepuisURL->setEnabled(true);
        ui->pageAjoutModifBoutonWiki->setEnabled(true);
    } else {
        ui->pageAjoutModifBoutonDossier->setEnabled(false);
        ui->pageAjoutModifBoutonValider->setEnabled(false);
        ui->pageAjoutModifPushButtonParcourirPhoto->setEnabled(false);
        ui->pageAjoutModifLineEditDepuisURL->setEnabled(false);
        ui->pageAjoutModifBoutonWiki->setEnabled(false);
    }

    if(!liste.isEmpty() && ui->pageAjoutModifBoutonValider->text() == "Modifier") {
        ui->pageAjoutModifGroupBoxSaison->setVisible(true);
        ui->pageAjoutModifPushButtonSupprimerSaison->setEnabled(true);
        ui->pageAjoutModifLineEditFicheSerieWiki->setText(liste.at(0).value(FICHE_SERIE_WIKI));
        ui->pageAjoutModifLineEditFicheSerieWiki->setCursorPosition(0);
        ui->pageAjoutModifLineEditAddicted->setText(liste.at(0).value(FICHE_SERIE_ADDICTED));
        ui->pageAjoutModifLineEditAddicted->setCursorPosition(0);
        if(liste.at(0).value(FICHE_SERIE_IMAGE) != "") {
            ui->pageAjoutModifLabelPhoto->setPixmap(QPixmap(liste.at(0).value(FICHE_SERIE_IMAGE)).scaled(ui->pageAjoutModifLabelPhoto->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->pageAjoutModifLabelCheminPhoto->setText(liste.at(0).value(FICHE_SERIE_IMAGE));
        } else {
            ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
        }

        champs.clear();
        conditions.clear();
        champs.append(SAISON_SAISON);
        champs.append(SAISON_NB_EPISODE);
        champs.append(SAISON_JOUR_SORTIE);
        champs.append(SAISON_EPISODE_COURANT);
        champs.append(SAISON_DATE_SORTIE);
        champs.append(SAISON_WIKI);
        champs.append(SAISON_DATE_MODIF);
        conditions.append(SAISON_ID + " = " + liste.at(0).value(FICHE_SERIE_ID));

        liste = this->bdd->requeteSelect(champs, SAISON_TABLE, conditions, ordres);

        if(!liste.isEmpty()) {
            ui->pageAjoutModifSpinBoxSaisonSaison->setValue(liste.at(0).value(SAISON_SAISON).toInt());
            ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(liste.at(0).value(SAISON_NB_EPISODE).toInt());
            ui->pageAjoutModifDateEditSaisonDateSortie->setDate(methodeDiverses.stringToDate(liste.at(0).value(SAISON_DATE_SORTIE)));
            if(ui->pageAjoutModifDateEditSaisonDateSortie->date() >= QDate::currentDate()) {
                ui->pageAjoutModifDateEditSaisonDateSortie->setEnabled(true);
                ui->pageAjoutModifDateEditSaisonDateProchain->setEnabled(false);
            } else {
                ui->pageAjoutModifDateEditSaisonDateSortie->setEnabled(false);
                ui->pageAjoutModifDateEditSaisonDateProchain->setEnabled(true);
            }
            ui->pageAjoutModifLineEditSaisonWiki->setText(liste.at(0).value(SAISON_WIKI));
            ui->pageAjoutModifLineEditSaisonWiki->setCursorPosition(0);
            ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(liste.at(0).value(SAISON_EPISODE_COURANT).toInt());
            ui->pageAjoutModifDateEditSaisonDateProchain->setDate(methodeDiverses.stringToDate(liste.at(0).value(SAISON_DATE_MODIF)).addDays(7));
        } else {
            ui->pageAjoutModifGroupBoxSaison->setVisible(false);
            ui->pageAjoutModifPushButtonSupprimerSaison->setEnabled(false);
            ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
            ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(ui->pageAjoutModifSpinBoxSaisonNbEpisode->minimum());
            ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
            ui->pageAjoutModifLineEditSaisonWiki->setText("");
            ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
            ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());
        }
    } else if(!liste.isEmpty() && ui->pageAjoutModifBoutonValider->text() == "Ajouter") {
        champs.clear();
        conditions.clear();
        champs.append(SAISON_ID);
        conditions.append(SAISON_ID + " = '" + liste.at(0).value(FICHE_SERIE_ID) + "'");

        if(this->bdd->requeteSelect(champs, SAISON_TABLE, conditions, ordres).isEmpty()) {
            ui->pageAjoutModifLineEditFicheSerieWiki->setText(liste.at(0).value(FICHE_SERIE_WIKI));
            ui->pageAjoutModifLineEditAddicted->setText(liste.at(0).value(FICHE_SERIE_ADDICTED));
            if(liste.at(0).value(FICHE_SERIE_IMAGE) != "") {
                ui->pageAjoutModifLabelPhoto->setPixmap(QPixmap(liste.at(0).value(FICHE_SERIE_IMAGE)).scaled(ui->pageAjoutModifLabelPhoto->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->pageAjoutModifLabelCheminPhoto->setText(liste.at(0).value(FICHE_SERIE_IMAGE));
            } else {
                ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
            }

            if(arg1 != "") {
                if(!ui->pageAjoutModifCheckBoxCreerSaison->isChecked()) {
                    ui->pageAjoutModifBoutonValider->setEnabled(false);
                } else {
                    ui->pageAjoutModifBoutonValider->setEnabled(true);
                }
            } else {
                ui->pageAjoutModifBoutonValider->setEnabled(false);
            }
        } else {
            ui->pageAjoutModifBoutonValider->setEnabled(false);
            ui->statusBar->showMessage("Il existe déjà une saison pour " + arg1, 5000);
        }
    } else {
        if(ui->pageAjoutModifBoutonValider->text() == "Modifier") {
            ui->pageAjoutModifGroupBoxSaison->setVisible(false);
            ui->pageAjoutModifPushButtonSupprimerSaison->setEnabled(false);
        }
        ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
        ui->pageAjoutModifLabelCheminPhoto->setText("");
        ui->pageAjoutModifLineEditFicheSerieWiki->setText("");
        ui->pageAjoutModifLineEditAddicted->setText("");
        ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
        ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(ui->pageAjoutModifSpinBoxSaisonNbEpisode->minimum());
        ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
        ui->pageAjoutModifLineEditSaisonWiki->setText("");
        ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
        ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());
    }

    champs.clear();
    conditions.clear();
    champs.append("DISTINCT " + HISTORIQUE_SAISON);
    conditions.append(HISTORIQUE_NOM + " = '" + arg1 + "'");

    QList<QMap<QString, QString> > listeSaison = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    if(listeSaison.isEmpty()) {
        ui->pageAjoutModifComboBoxListeSaison->setEnabled(false);
        ui->pageAjoutModifTableWidgetListeSaison->setEnabled(false);
    } else {
        ui->pageAjoutModifComboBoxListeSaison->setEnabled(true);
        ui->pageAjoutModifTableWidgetListeSaison->setEnabled(true);
        for (int i = 0; i < listeSaison.count(); i++) {
            ui->pageAjoutModifComboBoxListeSaison->addItem("Saison " + listeSaison.at(i).value("DISTINCT " + HISTORIQUE_SAISON));
        }
        ui->pageAjoutModifComboBoxListeSaison->setCurrentIndex(ui->pageAjoutModifComboBoxListeSaison->count() - 1);
    }

    ui->pageAjoutModifTableWidgetListeSaison->resizeColumnsToContents();
    if(ui->pageAjoutModifTableWidgetListeSaison->columnWidth(0) < 60) {
        ui->pageAjoutModifTableWidgetListeSaison->setColumnWidth(0, 60);
    }
    ui->pageAjoutModifTableWidgetListeSaison->setColumnWidth(3, 52);
    redimensionnerTableau();
}

void FenetrePrincipale::on_pageAjoutModifPushButtonTelecharger_clicked() {
    ui->pageAjoutModifLineEditCheminPhoto->setText("");
    QUrl imageURL(ui->pageAjoutModifLineEditDepuisURL->text());
    m_pImgCtrl = new FileDownloader(imageURL, this);

    connect(m_pImgCtrl, SIGNAL(downloaded()), this, SLOT(loadImage()));
}

void FenetrePrincipale::on_pageAjoutModifLineEditDepuisURL_textChanged(const QString &arg1) {
    if(arg1.isEmpty()) {
        ui->pageAjoutModifPushButtonTelecharger->setEnabled(false);
    } else {
        ui->pageAjoutModifPushButtonTelecharger->setEnabled(true);
    }
}

/*******************************************************\
*                                                       *
*                    PAGE REPORTER                      *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageReporterButtonValider_clicked() {
    QList<QString> champs;
    QList<QString> conditions;
    champs.append(SAISON_DATE_MODIF + " = '" + QDate::currentDate().addDays(7 * (ui->pageReporterSpinBox->value() - 1)).toString("yyyy-MM-dd") + "'");
    conditions.append(SAISON_ID + " = (SELECT " + FICHE_SERIE_ID + " FROM " + FICHE_SERIE_TABLE + " WHERE " + FICHE_SERIE_NOM + " = '" + ui->pageReporterLabelNomSerie->text() + "')");

    this->bdd->requeteUpdate(champs, SAISON_TABLE, conditions);
    refresh();
    on_pageReporterButtonRetour_clicked();
}

void FenetrePrincipale::on_pageReporterButtonRetour_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
}

void FenetrePrincipale::on_pageReporterSpinBox_valueChanged(int nbSemaines) {
    ui->pageReporterDate->setText(methodeDiverses.formatDate(QDate::currentDate().addDays(7 * nbSemaines)));
}

/*******************************************************\
*                                                       *
*               PAGE HISTORIQUE RECHERCHER              *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueRechercherPushButtonAjoutNomSerie_clicked() {
    QString nomSerie = ui->pageHistoriqueRechercherComboBoxNomSerie->currentText();
    QList<QString> serieChoisi = ui->pageHistoriqueRechercherLabelAffichage->text().split(", ");
    if(!serieChoisi.contains(nomSerie)) {
        if(ui->pageHistoriqueRechercherLabelAffichage->text() == "") {
            ui->pageHistoriqueRechercherLabelAffichage->setText(ui->pageHistoriqueRechercherLabelAffichage->text() + nomSerie);
        } else {
            ui->pageHistoriqueRechercherLabelAffichage->setText(ui->pageHistoriqueRechercherLabelAffichage->text() + ", " + nomSerie);
        }
    }
    ui->pageHistoriqueRechercherPushButtonRechercher->setEnabled(true);
}

void FenetrePrincipale::on_pageHistoriqueRechercherPushButtonSupprimerNomSerie_clicked() {
    QString nomSerie = ui->pageHistoriqueRechercherComboBoxNomSerie->currentText();
    QList<QString> serieChoisi = ui->pageHistoriqueRechercherLabelAffichage->text().split(", ");
    serieChoisi.removeOne(nomSerie);
    ui->pageHistoriqueRechercherLabelAffichage->setText("");
    for(int i = 0; i < serieChoisi.count(); i++) {
        if(ui->pageHistoriqueRechercherLabelAffichage->text() == "") {
            ui->pageHistoriqueRechercherLabelAffichage->setText(ui->pageHistoriqueRechercherLabelAffichage->text() + serieChoisi.at(i));
        } else {
            ui->pageHistoriqueRechercherLabelAffichage->setText(ui->pageHistoriqueRechercherLabelAffichage->text() + ", " + serieChoisi.at(i));
        }
    }
    if(ui->pageHistoriqueRechercherLabelAffichage->text() == "") {
        ui->pageHistoriqueRechercherPushButtonRechercher->setEnabled(false);
    }
}

void FenetrePrincipale::on_pageHistoriqueRechercherPushButtonSupprimerTout_clicked() {
    ui->pageHistoriqueRechercherLabelAffichage->setText("");
    ui->pageHistoriqueRechercherPushButtonRechercher->setEnabled(false);
}

void FenetrePrincipale::on_pageHistoriqueRechercherPushButtonRechercher_clicked() {
    QList<QString> listeSerie = ui->pageHistoriqueRechercherLabelAffichage->text().split(", ");
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;

    champs.append(HISTORIQUE_NOM);
    champs.append(HISTORIQUE_SAISON);
    champs.append(HISTORIQUE_EPISODE);
    champs.append(HISTORIQUE_DATE_AJOUT);
    QString serie;
    for(int i = 0; i < listeSerie.count(); i++) {
        if(i == 0) {
            serie.append("'" + listeSerie.at(i) + "'");
        } else {
            serie.append(", '" + listeSerie.at(i) + "'");
        }
    }
    conditions.append(HISTORIQUE_NOM + " IN (" + serie + ")");
    ordres.append(HISTORIQUE_DATE_AJOUT);

    QList<QMap<QString, QString> > liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    ui->pageHistoriqueRechercherTableWidget->setRowCount(0);
    for(int i = 0; i < liste.count(); i++) {
        ui->pageHistoriqueRechercherTableWidget->setRowCount(ui->pageHistoriqueRechercherTableWidget->rowCount() + 1);
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 0, methodeDiverses.itemForTableWidget(liste.at(i).value(HISTORIQUE_NOM), false));
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 1, methodeDiverses.itemForTableWidget(liste.at(i).value(HISTORIQUE_SAISON), true));
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 2, methodeDiverses.itemForTableWidget(liste.at(i).value(HISTORIQUE_EPISODE), true));
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 3, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(liste.at(i).value(HISTORIQUE_DATE_AJOUT)).toString("dd/MM/yyyy"), true));
    }
    ui->pageHistoriqueRechercherTableWidget->resizeColumnsToContents();
}

/*******************************************************\
*                                                       *
*                PAGE HISTORIQUE AJOUTER                *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueAjouterPushButtonAjouterLigne_clicked() {
    ui->pageHistoriqueAjouterPushButtonAjouter->setEnabled(true);
    int ligne = ui->pageHistoriqueAjouterTableWidget->rowCount();
    ui->pageHistoriqueAjouterTableWidget->setRowCount(ui->pageHistoriqueAjouterTableWidget->rowCount() + 1);
    QSpinBox *spinBoxSaison = new QSpinBox();
    QSpinBox *spinBoxEpisode = new QSpinBox();
    QDateEdit *dateEditDateDiffusion = new QDateEdit();
    QWidget *widgetSaison = new QWidget();
    QWidget *widgetEpisode = new QWidget();
    QWidget *widgetDateDiffusion = new QWidget();
    QHBoxLayout *layoutSaison = new QHBoxLayout(widgetSaison);
    QHBoxLayout *layoutEpisode = new QHBoxLayout(widgetEpisode);
    QHBoxLayout *layoutDateDiffusion = new QHBoxLayout(widgetDateDiffusion);

    spinBoxSaison->setMinimum(1);
    spinBoxEpisode->setMinimum(1);
    dateEditDateDiffusion->setDate(QDate());

    layoutSaison->addWidget(spinBoxSaison);
    layoutEpisode->addWidget(spinBoxEpisode);
    layoutDateDiffusion->addWidget(dateEditDateDiffusion);

    layoutSaison->setContentsMargins(0,0,0,0);
    layoutEpisode->setContentsMargins(0,0,0,0);
    layoutDateDiffusion->setContentsMargins(0,0,0,0);

    widgetSaison->setLayout(layoutSaison);
    widgetEpisode->setLayout(layoutEpisode);
    widgetDateDiffusion->setLayout(layoutDateDiffusion);

    ui->pageHistoriqueAjouterTableWidget->setCellWidget(ligne, 0, widgetSaison);
    ui->pageHistoriqueAjouterTableWidget->setCellWidget(ligne, 1, widgetEpisode);
    ui->pageHistoriqueAjouterTableWidget->setCellWidget(ligne, 2, widgetDateDiffusion);
}

void FenetrePrincipale::on_pageHistoriqueAjouterPushButtonSupprimerLigne_clicked() {
    ui->pageHistoriqueAjouterTableWidget->setRowCount(ui->pageHistoriqueAjouterTableWidget->rowCount() - 1);
    if(ui->pageHistoriqueAjouterTableWidget->rowCount() == 0)
        ui->pageHistoriqueAjouterPushButtonAjouter->setEnabled(false);
}

void FenetrePrincipale::on_pageHistoriqueAjouterPushButtonAjouter_clicked() {
    if(methodeDiverses.msgBoxQuestion("Etes vous sûr de vouloir ajouter ces informations ?") != 1) {
        for(int i = 0; i < ui->pageHistoriqueAjouterTableWidget->rowCount(); i++) {
            QSpinBox *spinBoxSaison = (QSpinBox *)ui->pageHistoriqueAjouterTableWidget->cellWidget(i, 0)->layout()->itemAt(0)->widget();
            QSpinBox *spinBoxEpisode = (QSpinBox *)ui->pageHistoriqueAjouterTableWidget->cellWidget(i, 1)->layout()->itemAt(0)->widget();
            QDateEdit *dateEditDateDiffusion = (QDateEdit *)ui->pageHistoriqueAjouterTableWidget->cellWidget(i, 2)->layout()->itemAt(0)->widget();

            QList<QString> champs;
            QList<QString> valeurs;
            champs.append(HISTORIQUE_NOM);
            valeurs.append(ui->pageHistoriqueAjouterComboBoxSerie->currentText());
            champs.append(HISTORIQUE_SAISON);
            valeurs.append(methodeDiverses.formalismeEntier(spinBoxSaison->value()));
            champs.append(HISTORIQUE_EPISODE);
            valeurs.append(methodeDiverses.formalismeEntier(spinBoxEpisode->value()));
            champs.append(HISTORIQUE_DATE_AJOUT);
            valeurs.append(dateEditDateDiffusion->date().toString("yyyy-MM-dd"));
            bdd->requeteInsert(champs, valeurs, HISTORIQUE_TABLE);

        }
        ui->pageHistoriqueAjouterTableWidget->setRowCount(0);
    }
}

/*******************************************************\
*                                                       *
*                PAGE HISTORIQUE MODIFIER               *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueModifierPushButtonRechercher_clicked() {
    QString nom = ui->pageHistoriqueModifierComboBoxRechercheNom->currentText();
    QString saison = methodeDiverses.formalismeEntier(ui->pageHistoriqueModifierSpinBoxRechercheSaison->value());
    QString episode = methodeDiverses.formalismeEntier(ui->pageHistoriqueModifierSpinBoxRechercheEpisode->value());

    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;
    champs.append(HISTORIQUE_DATE_AJOUT);
    conditions.append(HISTORIQUE_NOM + " = '" + nom + "'");
    conditions.append(HISTORIQUE_SAISON + " = '" + saison + "'");
    conditions.append(HISTORIQUE_EPISODE + " = '" + episode + "'");

    QList<QMap<QString, QString> > liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);

    if(!liste.isEmpty()) {
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setEnabled(true);
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setDate(methodeDiverses.stringToDate(liste.at(0).value(HISTORIQUE_DATE_AJOUT)));
    } else {
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setEnabled(false);
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setDate(QDate());
    }

}

void FenetrePrincipale::on_pageHistoriqueModifierPushButtonModifier_clicked() {
    QString nom = ui->pageHistoriqueModifierComboBoxRechercheNom->currentText();
    QString saison = methodeDiverses.formalismeEntier(ui->pageHistoriqueModifierSpinBoxRechercheSaison->value());
    QString episode = methodeDiverses.formalismeEntier(ui->pageHistoriqueModifierSpinBoxRechercheEpisode->value());
    QList<QString> champs;
    QList<QString> conditions;
    champs.append(HISTORIQUE_DATE_AJOUT + " = '" + ui->pageHistoriqueModifierDateEditModificationDateDiffusion->date().toString("yyyy-MM-dd") + "'");
    conditions.append(HISTORIQUE_NOM + " = '" + nom + "'");
    conditions.append(HISTORIQUE_SAISON + " = '" + saison + "'");
    conditions.append(HISTORIQUE_EPISODE + " = '" + episode + "'");
    this->bdd->requeteUpdate(champs, HISTORIQUE_TABLE, conditions);
    ui->statusBar->showMessage("La date de l'épisode " + episode + " de la saison " + saison + " de la série " + nom + " a été modifié", 5000);
}

void FenetrePrincipale::on_pageHistoriqueModifierComboBoxRechercheNom_currentTextChanged(const QString &arg1) {
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;

    champs.append("MAX(" + HISTORIQUE_SAISON + ")");
    conditions.append(HISTORIQUE_NOM + " = '" + arg1 + "'");

    QList<QMap<QString,QString> > liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    ui->pageHistoriqueModifierSpinBoxRechercheSaison->setMaximum(liste.at(0).value("MAX(" + HISTORIQUE_SAISON + ")").toInt());

    champs.clear();
    champs.append("MIN(" + HISTORIQUE_SAISON + ")");

    liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    ui->pageHistoriqueModifierSpinBoxRechercheSaison->setMinimum(liste.at(0).value("MIN(" + HISTORIQUE_SAISON + ")").toInt());
}

void FenetrePrincipale::on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged(int arg1) {
    QList<QString> champs;
    QList<QString> conditions;
    QList<QString> ordres;

    champs.append("MAX(" + HISTORIQUE_EPISODE + ")");
    conditions.append(HISTORIQUE_NOM + " = '" + ui->pageHistoriqueModifierComboBoxRechercheNom->currentText() + "'");
    conditions.append(HISTORIQUE_SAISON + " = '" + methodeDiverses.formalismeEntier(arg1) + "'");

    QList<QMap<QString,QString> > liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    ui->pageHistoriqueModifierSpinBoxRechercheEpisode->setMaximum(liste.at(0).value("MAX(" + HISTORIQUE_EPISODE + ")").toInt());

    champs.clear();
    champs.append("MIN(" + HISTORIQUE_EPISODE + ")");

    liste = this->bdd->requeteSelect(champs, HISTORIQUE_TABLE, conditions, ordres);
    ui->pageHistoriqueModifierSpinBoxRechercheEpisode->setMinimum(liste.at(0).value("MIN(" + HISTORIQUE_EPISODE + ")").toInt());

}

/*******************************************************\
*                                                       *
*               PAGE HISTORIQUE CONSULTER               *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueTableWidget_customContextMenuRequested(const QPoint &pos) {
    if(ui->pageHistoriqueTableWidget->rowCount() > 0 && !ui->pageHistoriqueTableWidget->selectedRanges().isEmpty()) {
        QMenu menuContextuel(this);
        QAction *copier = menuContextuel.addAction(QIcon(), "Copier le nom de la série");
        QAction *copierComplet = menuContextuel.addAction(QIcon(), "Copier le nom, la saison et l'épisode de la série");
        QAction *a = menuContextuel.exec(ui->pageHistoriqueTableWidget->viewport()->mapToGlobal(pos));

        if(a == copier) {
            QApplication::clipboard()->setText(ui->pageHistoriqueTableWidget->item(ui->pageHistoriqueTableWidget->selectedRanges().at(0).topRow(), 0)->text());
        } else if(a == copierComplet) {
            QApplication::clipboard()->setText(ui->pageHistoriqueTableWidget->item(ui->pageHistoriqueTableWidget->selectedRanges().at(0).topRow(), 0)->text() + " S" + ui->pageHistoriqueTableWidget->item(ui->pageHistoriqueTableWidget->selectedRanges().at(0).topRow(), 1)->text() + "E" + ui->pageHistoriqueTableWidget->item(ui->pageHistoriqueTableWidget->selectedRanges().at(0).topRow(), 2)->text());
        }
    }
}

/*******************************************************\
*                                                       *
*                PAGE HISTORIQUE IMPORTER               *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueImporterPushButtonParcourir_clicked() {
    QString chemin = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "Fichier CSV (*.csv)");
    if(!chemin.isEmpty()) {
        ui->pageHistoriqueImporterLineEditCheminFichier->setText(chemin);
        ui->pageHistoriqueImporterPushButtonChargerFichier->setEnabled(true);
    }
}

void FenetrePrincipale::on_pageHistoriqueImporterPushButtonChargerFichier_clicked() {
    QFile fichier(ui->pageHistoriqueImporterLineEditCheminFichier->text());
    if(fichier.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QTextStream flux(&fichier);
        while(!flux.atEnd()) {
            QString texte = flux.readLine();
            QList<QString> colonne = texte.split(";");
            if(!colonne.isEmpty()) {
                int ligne = ui->pageHistoriqueImporterTableWidget->rowCount();
                ui->pageHistoriqueImporterTableWidget->setRowCount(ui->pageHistoriqueImporterTableWidget->rowCount() + 1);
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 0, methodeDiverses.itemForTableWidget(colonne.at(0), false));
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 1, methodeDiverses.itemForTableWidget(methodeDiverses.formalismeEntier(colonne.at(1).toInt()), true));
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 2, methodeDiverses.itemForTableWidget(methodeDiverses.formalismeEntier(colonne.at(2).toInt()), true));
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 3, methodeDiverses.itemForTableWidget(methodeDiverses.jmaToDate(colonne.at(3)).toString("dd/MM/yyyy"), true));
                ui->pageHistoriqueImporterPushButtonImporter->setEnabled(true);
            }
        }
    }
}

void FenetrePrincipale::on_pageHistoriqueImporterPushButtonImporter_clicked() {
    for(int i = 0; i < ui->pageHistoriqueImporterTableWidget->rowCount(); i++) {
        QString nom = ui->pageHistoriqueImporterTableWidget->item(i, 0)->text();
        QString saison = ui->pageHistoriqueImporterTableWidget->item(i, 1)->text();
        QString episode = ui->pageHistoriqueImporterTableWidget->item(i, 2)->text();
        QString dateDiffusion = ui->pageHistoriqueImporterTableWidget->item(i, 3)->text();

        QList<QString> champs;
        QList<QString> valeurs;

        champs.append(HISTORIQUE_NOM);
        valeurs.append(nom);
        champs.append(HISTORIQUE_SAISON);
        valeurs.append(saison);
        champs.append(HISTORIQUE_EPISODE);
        valeurs.append(episode);
        champs.append(HISTORIQUE_DATE_AJOUT);
        valeurs.append(methodeDiverses.jmaToDate(dateDiffusion).toString("yyyy-MM-dd"));

        this->bdd->requeteInsert(champs, valeurs, HISTORIQUE_TABLE);
    }
    ui->statusBar->showMessage("Import effectué");
    ui->pageHistoriqueImporterLineEditCheminFichier->setText("");
    ui->pageHistoriqueImporterPushButtonChargerFichier->setEnabled(false);
    ui->pageHistoriqueImporterPushButtonImporter->setEnabled(false);
    ui->pageHistoriqueImporterTableWidget->setRowCount(0);
}
