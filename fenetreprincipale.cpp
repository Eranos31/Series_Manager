#include "fenetreprincipale.h"
#include "ui_fenetreprincipale.h"

FenetrePrincipale::FenetrePrincipale(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FenetrePrincipale) {
    ui->setupUi(this);

    QDir dir;
#ifdef QT_DEBUG
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs").exists())
        dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs");
#else
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs").exists())
        dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs");
#endif

    log = new Log();

#ifdef QT_DEBUG
    QFile *file = new QFile(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini");
#else
    QFile *file = new QFile(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini");
#endif

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

void FenetrePrincipale::refresh() {
    log->ecrire("FenetrePrincipale::refresh() : Début de l'actualisation");
    // Met a jour les episodes de la veille
    majEpisode();
    // Verifie si une série est terminé
    verificationSerieTerminer();

    QList<QMap<QString,QString>> listeHier = bdd->requeteListeHier("SELECT NOM, SAISON, EPISODE "
                                                                   "FROM HISTORIQUE "
                                                                   "WHERE DATEAJOUT = '" + QDate::currentDate().addDays(-1).toString("yyyy-MM-dd") + "' "
                                                                   "ORDER BY DATEAJOUT DESC");

    listeQuoti = bdd->requeteListe("SELECT NOM, SAISON, NBEPISODE, EPISODECOURANT, DATESORTIE, JOURSORTIE, DATEMODIF, WIKI "
                                   "FROM SERIE "
                                   "WHERE DATEMODIF = '" + QDate::currentDate().addDays(-7).toString("yyyy-MM-dd") + "' "
                                   "ORDER BY NOM");

    listeGlobal = bdd->requeteListe("SELECT NOM, SAISON, NBEPISODE, EPISODECOURANT, DATESORTIE, JOURSORTIE, DATEMODIF, WIKI "
                                    "FROM SERIE "
                                    "ORDER BY DATEMODIF, JOURSORTIE");

    int indice = 0;
    bool activationBoutonWikiGlobal = false;
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
    ui->pageListeModificationDisplay->clear();
    ui->pageListeSupprimerDisplay->clear();
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

        QPushButton* url = new QPushButton(i_logo, "");
        QPushButton* wiki = new QPushButton(i_wiki, "");
        QPushButton* reporter = new QPushButton(i_reporter, "");

        url->setToolTip("Ouvre le lien URL de " + map["NOM"]);
        wiki->setToolTip("Ouvre le lien Wikipédia de " + map["NOM"]);
        reporter->setToolTip("Ouvre la fênetre de report de " + map["NOM"]);

        mapper1->setMapping(url, lien + QString(map["NOM"]).replace(" ", "+") + "+S" + map["SAISON"] + "E" + map["EPISODECOURANT"] + sousTitres + qualite + fin);
        mapper2->setMapping(wiki, map["WIKI"]);
        mapper3->setMapping(reporter, map["NOM"]);

        QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(on_bouton_lien_clicked(QString)));
        QObject::connect(url, SIGNAL(clicked()), mapper1, SLOT(map()));

        QObject::connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(on_bouton_lien_clicked(QString)));
        QObject::connect(wiki, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

        QObject::connect(mapper3, SIGNAL(mapped(QString)), this, SLOT(on_bouton_reporter_clicked(QString)));
        QObject::connect(reporter, SIGNAL(clicked(bool)), mapper3, SLOT(map()));

        ui->pagePrincipaleTableWidgetDisplay->setRowCount(indice+1);
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(map["NOM"], false));
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(map["SAISON"], true));
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(map["EPISODECOURANT"], true));
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 3, url);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 4, wiki);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 5, reporter);

        indice++;

        if(map["WIKI"] == "") {
            wiki->setEnabled(false);
        }
    }

    log->ecrire("\tActualisation de l'onglet Aujourd'hui effectué");

    if(listeQuoti.isEmpty()) {
        ui->pagePrincipaleBoutonUrl->setEnabled(false);
    } else {
        ui->pagePrincipaleBoutonUrl->setEnabled(true);
    }

    for (int i = 0; i < listeQuoti.count(); ++i) {
        if(!listeQuoti.at(i)["WIKI"].isEmpty()) {
            activationBoutonWikiGlobal = true;
        }
    }
    ui->pagePrincipaleBoutonWiki->setEnabled(activationBoutonWikiGlobal);
    log->ecrire("\tParamètrage des boutons globaux");



    // Refresh de l'onglet hier
    ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(0);

    indice = 0;

    for(int i = 0; i < listeHier.count(); i++) {
        QMap<QString, QString> mapHier = listeHier.value(i);
        QString lienWiki = this->bdd->requeteUneColonne("SELECT WIKI FROM SERIE WHERE NOM = '" + mapHier["NOM"] + "'");

        QSignalMapper* mapper1 = new QSignalMapper();
        QSignalMapper* mapper2 = new QSignalMapper();

        QPushButton* lienHier = new QPushButton(i_logo, "");
        QPushButton* wikiHier = new QPushButton(i_wiki, "");

        lienHier->setToolTip("Ouvre le lien URL de " + mapHier["NOM"]);
        wikiHier->setToolTip("Ouvre le lien Wikipédia de " + mapHier["NOM"]);

        mapper1->setMapping(lienHier, lien + QString(mapHier["NOM"]).replace(" ", "+") + "+S" + mapHier["SAISON"] + "E" + mapHier["EPISODE"] + sousTitres + qualite + fin);
        mapper2->setMapping(wikiHier, lienWiki);

        QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(on_bouton_lien_clicked(QString)));
        QObject::connect(lienHier, SIGNAL(clicked()), mapper1, SLOT(map()));

        QObject::connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(on_bouton_lien_clicked(QString)));
        QObject::connect(wikiHier, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

        ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(indice+1);
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 0, methodeDiverses.itemForTableWidget(mapHier["NOM"], false));
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 1, methodeDiverses.itemForTableWidget(mapHier["SAISON"], true));
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 2, methodeDiverses.itemForTableWidget(mapHier["EPISODE"], true));
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 3, lienHier);
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 4, wikiHier);

        if(lienWiki == "") {
            wikiHier->setEnabled(false);
        }

        indice++;
    }

    log->ecrire("\tActualisation de l'onglet Hier effectué");

    // Refresh de l'historique
    ui->pageHistoriqueTableWidget->setRowCount(0);

    indice = 0;

    QList<QMap<QString, QString> > listeHistorique = this->bdd->requeteHistorique();
    for(int i = 0; i < listeHistorique.count(); i++) {
        QMap<QString, QString> map = listeHistorique.value(i);

        QSignalMapper* mapper = new QSignalMapper();
        QPushButton* url = new QPushButton(i_logo, "");
        url->setToolTip("Ouvre le lien URL de " + map["NOM"] + "pour l'épisode donné");
        mapper->setMapping(url, lien + QString(map["NOM"]).replace(" ","+") + "+S" + map["SAISON"] + "E" + map["EPISODE"] + sousTitres + qualite + fin);
        QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(on_bouton_lien_clicked(QString)));
        QObject::connect(url, SIGNAL(clicked()), mapper, SLOT(map()));

        ui->pageHistoriqueTableWidget->setRowCount(indice+1);
        ui->pageHistoriqueTableWidget->setItem(indice, 0, methodeDiverses.itemForTableWidget(map["NOM"], false));
        ui->pageHistoriqueTableWidget->setItem(indice, 1, methodeDiverses.itemForTableWidget(map["SAISON"], true));
        ui->pageHistoriqueTableWidget->setItem(indice, 2, methodeDiverses.itemForTableWidget(map["EPISODE"], true));
        ui->pageHistoriqueTableWidget->setItem(indice, 3, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(map["DATEAJOUT"]).toString("dd/MM/yy"), true));
        ui->pageHistoriqueTableWidget->setCellWidget(indice, 4, url);

        indice++;
    }

    log->ecrire("\tActualisation du tableau de l'historique effectué");

    // Dimensionner les colonnes
    ui->pagePrincipaleTableWidgetDisplay->resizeColumnToContents(0);
    ui->pagePrincipaleTableWidgetDisplay->resizeColumnToContents(1);
    ui->pagePrincipaleTableWidgetDisplay->resizeColumnToContents(2);
    ui->pagePrincipaleTableWidgetDisplay->setColumnWidth(3, 52);
    ui->pagePrincipaleTableWidgetDisplay->setColumnWidth(4, 52);
    ui->pagePrincipaleTableWidgetDisplay->setColumnWidth(5, 52);

    ui->pagePrincipaleTableWidgetDisplay_2->resizeColumnToContents(0);
    ui->pagePrincipaleTableWidgetDisplay_2->resizeColumnToContents(1);
    ui->pagePrincipaleTableWidgetDisplay_2->resizeColumnToContents(2);
    ui->pagePrincipaleTableWidgetDisplay_2->setColumnWidth(3, 52);
    ui->pagePrincipaleTableWidgetDisplay_2->setColumnWidth(4, 52);

    ui->pageHistoriqueTableWidget->resizeColumnsToContents();
    ui->pageHistoriqueTableWidget->setColumnWidth(4, 52);

    log->ecrire("\tDimensionnement des colonnes des tableaux effectué");

    if(listeGlobal.isEmpty()) {
        ui->pagePrincipaleBoutonVosSeries->setEnabled(false);
        ui->pagePrincipaleBoutonModifier->setEnabled(false);
        ui->toolBarModifier->setEnabled(false);
        ui->pagePrincipaleBoutonSupprimer->setEnabled(false);
        ui->toolBarSupprimer->setEnabled(false);
    } else {
        ui->pagePrincipaleBoutonVosSeries->setEnabled(true);
        ui->pagePrincipaleBoutonModifier->setEnabled(true);
        ui->toolBarModifier->setEnabled(true);
        ui->pagePrincipaleBoutonSupprimer->setEnabled(true);
        ui->toolBarSupprimer->setEnabled(true);
        for (int i = 0; i < listeGlobal.count(); i++) {
            QMap<QString, QString> map = listeGlobal.value(i);
            ui->pageListeModificationDisplay->addItem(map["NOM"]);
            ui->pageListeSupprimerDisplay->addItem(map["NOM"]);
        }
    }

    log->ecrire("\tAjout des noms de série pour les pages de sélection de modification et de suppression");

    // Tri des affichage
    ui->pageListeModificationDisplay->sortItems(Qt::AscendingOrder);
    ui->pageListeSupprimerDisplay->sortItems(Qt::AscendingOrder);

    log->ecrire("\tTri des liste de modification et de suppression");

    ui->pageAjoutModifComboNom->clear();
    ui->pageAjoutModifComboNom->setEditable(true);
    ui->pageAjoutModifComboNom->addItem("");

    listeSerie.clear();
    foreach (QFileInfo file, QDir(dossierSerie).entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs)) {
        if(file.isDir() && file.absoluteFilePath() != getConfig("Configuration/Telechargement")) {
            QString nom;
            nom = file.absoluteFilePath();
            nom.replace(dossierSerie, "");
            ui->pageAjoutModifComboNom->addItem(nom);
            listeSerie.append(nom);

        }
    }

    log->ecrire("\tAjout des noms des séries du dossier des série dans la liste déroulante de l'ajout de série");

    ui->statusBar->showMessage("Actualisé à " + QTime::currentTime().toString("hh:mm:ss"));
    log->ecrire("FenetrePrincipale::refresh() : Fin de l'actualisation");
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
    // Recuperation de la base de données ou création si elle n'existe pas
    this->bdd = new BaseDeDonnees();
    // Récupération de la version de l'appli
    this->version = "2.4";
    this->setWindowTitle("Series Manager " + this->version);
#ifdef QT_DEBUG
    this->dateVersion = QFileInfo("./debug/Series_Manager.exe").lastModified().date();
    this->heureVersion = QFileInfo("./debug/Series_Manager.exe").lastModified().time();
#else
    this->dateVersion = QFileInfo("Series_Manager.exe").lastModified().date();
    this->heureVersion = QFileInfo("Series_Manager.exe").lastModified().time();
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

void FenetrePrincipale::on_bouton_lien_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::on_bouton_lien_clicked() : Début de la génération du lien");
    if(QDesktopServices::openUrl(QUrl(nom))) {
        log->ecrire("\tOuverture du lien " + nom);
    } else {
        QMessageBox::warning(this, this->windowTitle(), "Le lien " + nom + " n'a pu être ouvert");
        log->ecrire("\tLe lien " + nom + " n'a pu être ouvert");
    }

    log->ecrire("FenetrePrincipale::on_bouton_lien_clicked() : Fin de la génération du lien");
}

void FenetrePrincipale::on_bouton_dossier_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::on_bouton_dossier_clicked() : Début de l'ouverture du dossier");
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + nom))) {
        QMessageBox::warning(this,this->windowTitle(), "Le dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
        log->ecrire("\tLe dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
    } else {
        log->ecrire("\tLe dossier " + dossierSerie + nom + " a été ouvert");
    }
    log->ecrire("FenetrePrincipale::on_bouton_dossier_clicked() : Fin de l'ouverture du dossier");
}

void FenetrePrincipale::on_bouton_reporter_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::on_bouton_reporter_clicked() : Ouverture de la page de report de série");
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
        QList<QMap<QString,QString> > liste = bdd->requeteListe("SELECT NOM, SAISON, NBEPISODE, EPISODECOURANT, DATESORTIE, JOURSORTIE, DATEMODIF, WIKI "
                                                                "FROM SERIE "
                                                                "WHERE JOURSORTIE = '" + QString::number(dateDerniereOuverture.dayOfWeek()) + "' ");

        if(!liste.isEmpty()) {
            for (int i = 0; i < liste.count(); ++i) {
                QMap<QString, QString> list = liste.value(i);

                if(methodeDiverses.stringToDate(list["DATESORTIE"]) <= dateDerniereOuverturePlus1 &&
                   methodeDiverses.stringToDate(list["DATEMODIF"]) != dateDerniereOuverture) {
                    if(methodeDiverses.stringToDate(list["DATEMODIF"]) == QDate() ||
                       (methodeDiverses.stringToDate(list["DATEMODIF"]) >= dateDerniereOuverture.addDays(-7) &&
                        methodeDiverses.stringToDate(list["DATEMODIF"]) < dateDerniereOuverture)) {
                        log->ecrire("\tMise à jour de " + list["NOM"]);
                        bdd->modifier(list["NOM"], list["SAISON"].toInt(), list["NBEPISODE"].toInt(), list["EPISODECOURANT"].toInt() + 1, dateDerniereOuverture.dayOfWeek(), methodeDiverses.stringToDate(list["DATESORTIE"]), list["WIKI"], dateDerniereOuverture, false);

                        this->bdd->requeteInsertUpdate("INSERT INTO HISTORIQUE (NOM, SAISON, EPISODE, DATEAJOUT)"
                                                       "VALUES ('" + list["NOM"] + "', '" + list["SAISON"] + "', '" + list["EPISODECOURANT"] + "', '" + dateDerniereOuverture.toString("yyyy-MM-dd") + "')");
                    }
                }
            }
        }

        dateDerniereOuverture = dateDerniereOuverture.addDays(1);
        dateDerniereOuverturePlus1 = dateDerniereOuverturePlus1.addDays(1);
    }
    bdd->majDerniereOuvertureBDD();
    log->ecrire("FenetrePrincipale::majEpisode() : Fin de mise à jour des episodes de la veille");
}

void FenetrePrincipale::verificationSerieTerminer() {
    log->ecrire("FenetrePrincipale::verificationSerieTerminer() : Début de vérification des séries terminées");
    QList<QMap<QString,QString> > liste;
    liste = this->bdd->requeteListe("SELECT NOM, SAISON, NBEPISODE, EPISODECOURANT FROM SERIE");
    for (int i = 0; i < liste.count(); ++i) {
        QMap<QString,QString> list = liste.value(i);

        if(list["NBEPISODE"].toInt() < list["EPISODECOURANT"].toInt()){
            log->ecrire("\tSuppression de " + list["NOM"]);
            this->bdd->supprimer(list["NOM"], false);
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

QList<QString> FenetrePrincipale::getListeSerie() {
    return this->listeSerie;
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

/*******************************************************\
*                                                       *
*                          MENU                         *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_menuFichierAccueil_triggered() {
    ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
}

void FenetrePrincipale::on_menuFichierHistorique_triggered() {
    ui->stackedWidget->setCurrentWidget(ui->pageHistorique);
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
    ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
    ui->pageAjoutModifTitre->setText("Ajouter série");
    ui->pageAjoutModifComboNom->setEnabled(true);
    ui->pageAjoutModifComboNom->setCurrentText("");
    ui->pageAjoutModifLineSaison->setValue(1);
    ui->pageAjoutModifLineSaison->clear();
    ui->pageAjoutModifLineNbEpisode->setValue(1);
    ui->pageAjoutModifLineNbEpisode->clear();
    ui->pageAjoutModifLabelCreationDossierAuto->setVisible(true);
    ui->pageAjoutModifRadioButtonOui->setVisible(true);
    ui->pageAjoutModifRadioButtonOui->setChecked(true);
    ui->pageAjoutModifRadioButtonNon->setVisible(true);
    ui->pageAjoutModifLineDateSortie->setEnabled(true);
    ui->pageAjoutModifLineDateSortie->setDate(QDate::currentDate());
    ui->pageAjoutModifLineWiki->setText("");
    ui->pageAjoutModifLabelEpisodeCourant->setVisible(false);
    ui->pageAjoutModifLineEpisodeCourant->setVisible(false);
    ui->pageAjoutModifLabelDateProchain->setVisible(false);
    ui->pageAjoutModifLabelDateProchain_2->setVisible(false);
    ui->pageAjoutModifLineDateProchain->setVisible(false);
    ui->pageAjoutModifBoutonValider->setIcon(i_valider);
    ui->pageAjoutModifBoutonValider->setText("Ajouter");
    ui->pageAjoutModifBoutonDossier->setVisible(false);
    ui->pageAjoutModifBoutonWiki->setVisible(false);
}

void FenetrePrincipale::on_menuFichierModifier_triggered() {
    if(listeGlobal.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageListeModification);
        ui->pageAjoutModifComboNom->setEnabled(false);
        ui->pageAjoutModifLabelEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLineEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLabelDateProchain->setVisible(true);
        ui->pageAjoutModifLineDateProchain->setVisible(true);
        ui->pageAjoutModifLabelDateProchain_2->setVisible(true);
        ui->pageAjoutModifBoutonValider->setIcon(i_modifier);
        ui->pageAjoutModifBoutonDossier->setVisible(true);
        ui->pageAjoutModifBoutonWiki->setVisible(true);

        ui->pageListeModificationDisplay->item(0)->setSelected(true);
        ui->pageListeModificationDisplay->setCurrentItem(ui->pageListeModificationDisplay->item(0));
    } else {
        QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de série !");
    }
}

void FenetrePrincipale::on_menuFichierSupprimer_triggered() {
    if(listeGlobal.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageListeSupprimer);
    } else {
        QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de série !");
    }

    if(ui->pageListeSupprimerDisplay->count() != 0){
        ui->pageListeSupprimerDisplay->item(0)->setSelected(true);
        ui->pageListeSupprimerDisplay->setCurrentItem(ui->pageListeSupprimerDisplay->item(0));
    }
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
    if((ui->stackedWidget->currentWidget() == ui->pageAjoutModification && ui->pageAjoutModifTitre->text() != "Ajouter série") || (ui->stackedWidget->currentWidget() != ui->pageAjoutModification)) {
        on_menuFichierAjouter_triggered();
    }
}

void FenetrePrincipale::on_toolBarModifier_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageListeModification) {
        on_menuFichierModifier_triggered();
    }
}

void FenetrePrincipale::on_toolBarSupprimer_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageListeSupprimer) {
        on_menuFichierSupprimer_triggered();
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
    QString serie = ui->pagePrincipaleTableWidgetDisplay->item(index.row(), 0)->text();
    serie.replace("'", "''");
    QMap<QString,QString> liste = this->bdd->requete("SELECT NOM, SAISON, NBEPISODE, JOURSORTIE, EPISODECOURANT, DATESORTIE, WIKI, DATEMODIF "
                                              "FROM SERIE "
                                              "WHERE NOM = '" + serie + "'"
                                              "ORDER BY NOM");
    if(liste.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
        ui->pageAjoutModifTitre->setText("Modifier série");
        ui->pageAjoutModifComboNom->setEnabled(false);
        ui->pageAjoutModifComboNom->setEditText(liste["NOM"]);
        ui->pageAjoutModifLineSaison->setValue(liste["SAISON"].toInt());
        ui->pageAjoutModifLineNbEpisode->setValue(liste["NBEPISODE"].toInt());
        if (methodeDiverses.stringToDate(liste["DATESORTIE"]) < QDate::currentDate()) {
            ui->pageAjoutModifLineDateSortie->setEnabled(false);
            ui->pageAjoutModifLineDateProchain->setEnabled(true);
        } else {
            ui->pageAjoutModifLineDateSortie->setEnabled(true);
            ui->pageAjoutModifLineDateProchain->setEnabled(false);
        }
        ui->pageAjoutModifLineDateSortie->setDate(methodeDiverses.stringToDate(liste["DATESORTIE"]));
        ui->pageAjoutModifLineWiki->setText(liste["WIKI"]);
        ui->pageAjoutModifLineEpisodeCourant->setValue(liste["EPISODECOURANT"].toInt());
        ui->pageAjoutModifLineEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLineDateProchain->setDate(methodeDiverses.stringToDate(liste["DATEMODIF"]).addDays(7));
        ui->pageAjoutModifLineDateProchain->setVisible(true);
        ui->pageAjoutModifLabelDateProchain_2->setVisible(true);
        ui->pageAjoutModifLabelDateProchain->setVisible(true);
        ui->pageAjoutModifLabelCreationDossierAuto->setVisible(false);
        ui->pageAjoutModifRadioButtonOui->setVisible(false);
        ui->pageAjoutModifRadioButtonNon->setVisible(false);
        ui->pageAjoutModifLabelEpisodeCourant->setVisible(true);
        ui->pageAjoutModifBoutonValider->setIcon(i_modifier);
        ui->pageAjoutModifBoutonValider->setText("Modifier");
        ui->pageAjoutModifBoutonDossier->setVisible(true);
        ui->pageAjoutModifBoutonWiki->setVisible(true);
        ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
        if(!QFile(dossierSerie + liste["NOM"]).exists()) {
            ui->pageAjoutModifBoutonDossier->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonDossier->setEnabled(true);
        }

        if(liste["WIKI"] == "") {
            ui->pageAjoutModifBoutonWiki->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonWiki->setEnabled(true);
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleTableWidgetDisplay_2_doubleClicked(const QModelIndex &index) {
    QString serie = ui->pagePrincipaleTableWidgetDisplay_2->item(index.row(), 0)->text();
    serie.replace("'", "''");
    QMap<QString,QString> liste = this->bdd->requete("SELECT NOM, SAISON, NBEPISODE, JOURSORTIE, EPISODECOURANT, DATESORTIE, WIKI, DATEMODIF "
                                              "FROM SERIE "
                                              "WHERE NOM = '" + serie + "'"
                                              "ORDER BY NOM");
    if(liste.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
        ui->pageAjoutModifTitre->setText("Modifier série");
        ui->pageAjoutModifBoutonValider->setText("Modifier");
        ui->pageAjoutModifComboNom->setEditText(liste["NOM"]);
        ui->pageAjoutModifLineSaison->setValue(liste["SAISON"].toInt());
        ui->pageAjoutModifLineNbEpisode->setValue(liste["NBEPISODE"].toInt());
        ui->pageAjoutModifLineEpisodeCourant->setValue(liste["EPISODECOURANT"].toInt());
        if (methodeDiverses.stringToDate(liste["DATESORTIE"]) < QDate::currentDate()) {
            ui->pageAjoutModifLineDateSortie->setEnabled(false);
            ui->pageAjoutModifLineDateProchain->setEnabled(true);
        } else {
            ui->pageAjoutModifLineDateSortie->setEnabled(true);
            ui->pageAjoutModifLineDateProchain->setEnabled(false);
        }
        ui->pageAjoutModifLineDateSortie->setDate(methodeDiverses.stringToDate(liste["DATESORTIE"]));
        ui->pageAjoutModifLineWiki->setText(liste["WIKI"]);
        ui->pageAjoutModifLineDateProchain->setDate(methodeDiverses.stringToDate(liste["DATEMODIF"]).addDays(7));
        ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
        ui->pageAjoutModifBoutonDossier->setVisible(true);
        ui->pageAjoutModifBoutonWiki->setVisible(true);
        ui->pageAjoutModifComboNom->setEnabled(false);
        ui->pageAjoutModifBoutonValider->setIcon(i_modifier);
        ui->pageAjoutModifLabelCreationDossierAuto->setVisible(false);
        ui->pageAjoutModifRadioButtonOui->setVisible(false);
        ui->pageAjoutModifRadioButtonNon->setVisible(false);
        ui->pageAjoutModifLabelEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLineEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLabelDateProchain_2->setVisible(true);
        ui->pageAjoutModifLabelDateProchain->setVisible(true);
        ui->pageAjoutModifLineDateProchain->setVisible(true);
        if(!QFile(dossierSerie + liste["NOM"]).exists()) {
            ui->pageAjoutModifBoutonDossier->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonDossier->setEnabled(true);
        }

        if(liste["WIKI"] == "") {
            ui->pageAjoutModifBoutonWiki->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonWiki->setEnabled(true);
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleTableWidgetDisplay_customContextMenuRequested(const QPoint &pos) {
    if(ui->pagePrincipaleTableWidgetDisplay->rowCount() > 0 && !ui->pagePrincipaleTableWidgetDisplay->selectedRanges().isEmpty()) {
        QMenu menuContextuel(this);
        QAction *copier = menuContextuel.addAction(QIcon(), "Copier le nom de la série");
        QAction *dossier;
        if(QDir(this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 1)->text()).exists()) {
            dossier = menuContextuel.addAction(QIcon(), "Ouvrir le dossier de la série");
        }

        QAction *a = menuContextuel.exec(ui->pagePrincipaleTableWidgetDisplay->viewport()->mapToGlobal(pos));

        if(a == copier) {
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
        QAction *dossier;
        if(QDir(this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 1)->text()).exists()) {
            dossier = menuContextuel.addAction(QIcon(), "Ouvrir le dossier de la série");
        }

        QAction *a = menuContextuel.exec(ui->pagePrincipaleTableWidgetDisplay_2->viewport()->mapToGlobal(pos));

        if(a == copier) {
            QApplication::clipboard()->setText(ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text() + " S" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 1)->text() + "E" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 2)->text());
        }else if(a == dossier) {
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

void FenetrePrincipale::on_pagePrincipaleBoutonSupprimer_clicked() {
    on_menuFichierSupprimer_triggered();
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
            if(methodeDiverses.stringToDate(list["DATESORTIE"]) <= QDate::currentDate() && methodeDiverses.stringToDate(list["DATEMODIF"]) <= QDate::currentDate() && methodeDiverses.stringToDate(list["DATEMODIF"]) != QDate::currentDate()) {
                QDesktopServices::openUrl(lien + QString(list["NOM"]).replace(" ","+") + "+S" + list["SAISON"] + "E" + list["EPISODECOURANT"] + sousTitres + qualite + fin);
            }
        }
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
            if(methodeDiverses.stringToDate(list["DATESORTIE"]) <= QDate::currentDate() && methodeDiverses.stringToDate(list["DATEMODIF"]) <= QDate::currentDate() && methodeDiverses.stringToDate(list["DATEMODIF"]) != QDate::currentDate()) {
                QDesktopServices::openUrl(QUrl(list["WIKI"]));
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
    QString serie = ui->pageVosSeriesDisplay->item(index.row(), 0)->text();
    serie.replace("'", "''");
    QMap<QString,QString> liste = this->bdd->requete("SELECT NOM, SAISON, NBEPISODE, JOURSORTIE, EPISODECOURANT, DATESORTIE, WIKI, DATEMODIF "
                                                     "FROM SERIE "
                                                     "WHERE NOM = '" + serie + "'"
                                                     "ORDER BY NOM");
    if(liste.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
        ui->pageAjoutModifTitre->setText("Modifier série");
        ui->pageAjoutModifBoutonValider->setText("Modifier");
        ui->pageAjoutModifComboNom->setEditText(liste["NOM"]);
        ui->pageAjoutModifLineSaison->setValue(liste["SAISON"].toInt());
        ui->pageAjoutModifLineNbEpisode->setValue(liste["NBEPISODE"].toInt());
        ui->pageAjoutModifLineEpisodeCourant->setValue(liste["EPISODECOURANT"].toInt());
        if (methodeDiverses.stringToDate(liste["DATESORTIE"]) < QDate::currentDate()) {
            ui->pageAjoutModifLineDateSortie->setEnabled(false);
            ui->pageAjoutModifLineDateProchain->setEnabled(true);
        } else {
            ui->pageAjoutModifLineDateSortie->setEnabled(true);
            ui->pageAjoutModifLineDateProchain->setEnabled(false);
        }
        ui->pageAjoutModifLineDateSortie->setDate(methodeDiverses.stringToDate(liste["DATESORTIE"]));
        ui->pageAjoutModifLineWiki->setText(liste["WIKI"]);
        ui->pageAjoutModifLineDateProchain->setDate(methodeDiverses.stringToDate(liste["DATEMODIF"]).addDays(7));
        ui->pageAjoutModifLabelRetour->setText("pageVosSeries");
        ui->pageAjoutModifBoutonDossier->setVisible(true);
        ui->pageAjoutModifBoutonWiki->setVisible(true);
        ui->pageAjoutModifComboNom->setEnabled(false);
        ui->pageAjoutModifBoutonValider->setIcon(i_modifier);
        ui->pageAjoutModifLabelCreationDossierAuto->setVisible(false);
        ui->pageAjoutModifRadioButtonOui->setVisible(false);
        ui->pageAjoutModifRadioButtonNon->setVisible(false);
        ui->pageAjoutModifLabelEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLineEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLabelDateProchain->setVisible(true);
        ui->pageAjoutModifLineDateProchain->setVisible(true);
        ui->pageAjoutModifLabelDateProchain_2->setVisible(true);
        if(!QFile(dossierSerie + liste["NOM"]).exists()) {
            ui->pageAjoutModifBoutonDossier->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonDossier->setEnabled(true);
        }

        if(liste["WIKI"] == "") {
            ui->pageAjoutModifBoutonWiki->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonWiki->setEnabled(true);
        }
    }
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
    for (int i = 0; i < listeGlobal.count(); ++i) {
        QMap<QString,QString> list = listeGlobal.value(i);

        if(selection == 2 && methodeDiverses.stringToDate(list["DATESORTIE"]) <= QDate::currentDate()) {
            continue;
        } else if (selection == 3 && methodeDiverses.stringToDate(list["DATESORTIE"]) > QDate::currentDate()) {
            continue;
        }

        ui->pageVosSeriesDisplay->setRowCount(indice + 1);
        ui->pageVosSeriesDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(list["NOM"], false));
        ui->pageVosSeriesDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(list["SAISON"], true));
        ui->pageVosSeriesDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(methodeDiverses.dayToString(list["JOURSORTIE"].toInt()), true));

        if(methodeDiverses.stringToDate(list["DATESORTIE"]) <= QDate::currentDate() &&
           (selection == 1 || selection == 3)) {
            ui->pageVosSeriesDisplay->setItem(indice, 3, methodeDiverses.itemForTableWidget("En cours", true));
        } else {
            ui->pageVosSeriesDisplay->setItem(indice, 3, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list["DATESORTIE"]).toString("dd/MM/yy"), true));
        }

        if(list["DATEMODIF"] == "0") {
            ui->pageVosSeriesDisplay->setItem(indice, 4, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list["DATESORTIE"]).toString("dd/MM/yy"), true));
        } else {
            ui->pageVosSeriesDisplay->setItem(indice, 4, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list["DATEMODIF"]).addDays(7).toString("dd/MM/yy"), true));
        }

        QSignalMapper* mapper = new QSignalMapper();
        QPushButton* dossier = new QPushButton(i_dossier,"");
        mapper->setMapping(dossier, "/" + list["NOM"] + "/Saison " + list["SAISON"]);
        QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(on_bouton_dossier_clicked(QString)));
        QObject::connect(dossier, SIGNAL(clicked(bool)), mapper, SLOT(map()));
        ui->pageVosSeriesDisplay->setCellWidget(indice, 5, dossier);

        QFile file (dossierSerie + list["NOM"]);
        if(!file.exists()) {
            dossier->setEnabled(false);
        }

        QSignalMapper* mapper1 = new QSignalMapper();
        QPushButton* wiki = new QPushButton(i_wiki,"");
        mapper1->setMapping(wiki, list["WIKI"]);
        QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(on_bouton_lien_clicked(QString)));
        QObject::connect(wiki, SIGNAL(clicked(bool)), mapper1, SLOT(map()));
        ui->pageVosSeriesDisplay->setCellWidget(indice, 6, wiki);

        if(list["WIKI"] == "") {
            wiki->setEnabled(false);
        }

        indice++;
    }

    ui->pageVosSeriesDisplay->resizeColumnsToContents();
}

/*******************************************************\
*                                                       *
*               PAGE AJOUT MODIFICAITON                 *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageAjoutModifLineDateSortie_userDateChanged(const QDate &date) {
    ui->pageAjoutModifLabelDate->setText(methodeDiverses.formatDate(date));
}

void FenetrePrincipale::on_pageAjoutModifButtonWiki_clicked() {
    QDesktopServices::openUrl(QUrl("https://fr.wikipedia.org/wiki/Wikip%C3%A9dia:Accueil_principal"));
}

void FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() {
    if(ui->pageAjoutModifBoutonValider->text() == "Ajouter") {
        if(ui->pageAjoutModifComboNom->currentText() != "" &&
           ui->pageAjoutModifLineSaison->text() != "" &&
           ui->pageAjoutModifLineNbEpisode->text() != "") {
            this->bdd->ajouter(ui->pageAjoutModifComboNom->currentText(), ui->pageAjoutModifLineSaison->text().toInt(), ui->pageAjoutModifLineNbEpisode->text().toInt(), ui->pageAjoutModifLineDateSortie->date().dayOfWeek(), ui->pageAjoutModifLineDateSortie->date(), ui->pageAjoutModifLineWiki->text());
            if (ui->pageAjoutModifRadioButtonOui->isChecked()) {
                QDir dir;
                dir.mkpath(dossierSerie + ui->pageAjoutModifComboNom->currentText() + "/Saison " + methodeDiverses.formalismeEntier(ui->pageAjoutModifLineSaison->text().toInt()));
            }
            ui->pageAjoutModifComboNom->clear();
            ui->pageAjoutModifLineSaison->clear();
            ui->pageAjoutModifLineNbEpisode->clear();
            ui->pageAjoutModifLineDateSortie->setDate(QDate::currentDate());
            ui->pageAjoutModifLineWiki->clear();
            refresh();
        } else {
            QMessageBox::critical(this, this->windowTitle(), "Veuillez remplir tout les champs !");
        }

    } else if(ui->pageAjoutModifBoutonValider->text() == "Modifier") {
        if(ui->pageAjoutModifLineDateSortie->isEnabled()) {
            this->bdd->modifier(ui->pageAjoutModifComboNom->currentText(), ui->pageAjoutModifLineSaison->text().toInt(), ui->pageAjoutModifLineNbEpisode->text().toInt(), ui->pageAjoutModifLineEpisodeCourant->text().toInt(), ui->pageAjoutModifLineDateSortie->date().dayOfWeek(), ui->pageAjoutModifLineDateSortie->date(), ui->pageAjoutModifLineWiki->text(), ui->pageAjoutModifLineDateSortie->date().addDays(-7), true);
        } else {
            this->bdd->modifier(ui->pageAjoutModifComboNom->currentText(), ui->pageAjoutModifLineSaison->text().toInt(), ui->pageAjoutModifLineNbEpisode->text().toInt(), ui->pageAjoutModifLineEpisodeCourant->text().toInt(), ui->pageAjoutModifLineDateProchain->date().dayOfWeek(), ui->pageAjoutModifLineDateSortie->date(), ui->pageAjoutModifLineWiki->text(), ui->pageAjoutModifLineDateProchain->date().addDays(-7), true);
        }
        if(ui->pageAjoutModifLabelRetour->text() == "pagePrincipale") {
            ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
        } else if(ui->pageAjoutModifLabelRetour->text() == "pageVosSeries") {
            ui->stackedWidget->setCurrentWidget(ui->pageVosSeries);
        } else if (ui->pageAjoutModifLabelRetour->text() == "pageListeModification"){
            ui->stackedWidget->setCurrentWidget(ui->pageListeModification);
        }
        refresh();
    }
}

void FenetrePrincipale::on_pageAjoutModifBoutonDossier_clicked() {
    if(ui->pageAjoutModifLineSaison->text().toInt() > 0 &&
       ui->pageAjoutModifLineSaison->text().toInt() < 10) {
        on_bouton_dossier_clicked("/" + ui->pageAjoutModifComboNom->currentText() + "/Saison 0" + ui->pageAjoutModifLineSaison->text());
    } else {
        on_bouton_dossier_clicked("/" + ui->pageAjoutModifComboNom->currentText()+ "/Saison " + ui->pageAjoutModifLineSaison->text());
    }
}

void FenetrePrincipale::on_pageAjoutModifBoutonWiki_clicked() {
    if (ui->pageAjoutModifBoutonValider->text() == "Modifier"){
        if(ui->pageAjoutModifLineWiki->text() != "") {
            if(ui->pageAjoutModifLineWiki->text().contains("wikipedia.org")) {
                QDesktopServices::openUrl(QUrl(ui->pageAjoutModifLineWiki->text()));
            } else {
                QMessageBox::warning(this, this->windowTitle(), "L'adresse saisie n'est pas un lien vers Wikipedia !");
            }
        } else {
            QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de lien !");
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
        } else if (ui->pageAjoutModifLabelRetour->text() == "pageListeModification"){
            ui->stackedWidget->setCurrentWidget(ui->pageListeModification);
        }
    }
}

void FenetrePrincipale::on_pageAjoutModifLineDateProchain_userDateChanged(const QDate &date) {
    ui->pageAjoutModifLabelDateProchain_2->setText(methodeDiverses.formatDate(date));
}


/*******************************************************\
*                                                       *
*               PAGE LISTE MODIFICATION                 *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageListeModificationBoutonModifier_clicked() {
    if(ui->pageListeModificationDisplay->count() != 0){
        QString serie = ui->pageListeModificationDisplay->currentItem()->text();
        serie.replace("'", "''");
        QMap<QString,QString> liste = this->bdd->requete("SELECT NOM, SAISON, NBEPISODE, JOURSORTIE, EPISODECOURANT, DATESORTIE, WIKI, DATEMODIF "
                                                  "FROM SERIE "
                                                  "WHERE NOM = '" + serie + "'"
                                                  "ORDER BY NOM");
        ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
        ui->pageAjoutModifTitre->setText("Modifier série");
        ui->pageAjoutModifComboNom->setEditText(liste["NOM"]);
        ui->pageAjoutModifLineSaison->setValue(liste["SAISON"].toInt());
        ui->pageAjoutModifLineNbEpisode->setValue(liste["NBEPISODE"].toInt());
        if (methodeDiverses.stringToDate(liste["DATESORTIE"]) < QDate::currentDate()) {
            ui->pageAjoutModifLineDateSortie->setEnabled(false);
            ui->pageAjoutModifLineDateProchain->setEnabled(true);
        } else {
            ui->pageAjoutModifLineDateSortie->setEnabled(true);
            ui->pageAjoutModifLineDateProchain->setEnabled(false);
        }
        ui->pageAjoutModifLineDateSortie->setDate(methodeDiverses.stringToDate(liste["DATESORTIE"]));
        ui->pageAjoutModifLabelCreationDossierAuto->setVisible(false);
        ui->pageAjoutModifRadioButtonOui->setVisible(false);
        ui->pageAjoutModifRadioButtonNon->setVisible(false);
        ui->pageAjoutModifLineWiki->setText(liste["WIKI"]);
        ui->pageAjoutModifLineEpisodeCourant->setValue(liste["EPISODECOURANT"].toInt());
        ui->pageAjoutModifLineDateProchain->setDate(methodeDiverses.stringToDate(liste["DATEMODIF"]).addDays(7));
        ui->pageAjoutModifLabelRetour->setText("pageListeModification");
        ui->pageAjoutModifBoutonValider->setText("Modifier");

        if(!QFile(dossierSerie + liste["NOM"]).exists()) {
            ui->pageAjoutModifBoutonDossier->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonDossier->setEnabled(true);
        }

        if(liste["WIKI"] == "") {
            ui->pageAjoutModifBoutonWiki->setEnabled(false);
        } else {
            ui->pageAjoutModifBoutonWiki->setEnabled(true);
        }
    }
}

void FenetrePrincipale::on_pageListeModificationBoutonRetour_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
}

void FenetrePrincipale::on_pageListeModificationDisplay_doubleClicked(const QModelIndex &index) {
    on_pageListeModificationBoutonModifier_clicked();
    index.isValid();
}

/*******************************************************\
*                                                       *
*               PAGE LISTE SUPPRESSION                  *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageListeSupprimerBoutonSupprimer_clicked() {
    if(ui->pageListeSupprimerDisplay->count() != 0){
        QString serie = ui->pageListeSupprimerDisplay->currentItem()->text();
        if(methodeDiverses.msgBoxQuestion("Voulez vous vraiment supprimer " + serie + " ?") == 0) {
            this->bdd->supprimer(serie, true);
            refresh();
        }
        if(ui->pageListeSupprimerDisplay->count() == 0) {
            ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
        }
    }
}

void FenetrePrincipale::on_pageListeSupprimerBoutonRetour_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
}

void FenetrePrincipale::on_pageListeSupprimerDisplay_doubleClicked(const QModelIndex &index) {
    on_pageListeSupprimerBoutonSupprimer_clicked();
    index.isValid();
}

/*******************************************************\
*                                                       *
*                    PAGE REPORTER                      *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageReporterButtonValider_clicked() {
    this->bdd->reporter(ui->pageReporterLabelNomSerie->text(),QDate::currentDate().addDays(7 * (ui->pageReporterSpinBox->value() - 1)));
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
*                   PAGE HISTORIQUE                     *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueTableWidget_customContextMenuRequested(const QPoint &pos) {
    if(ui->pageHistoriqueTableWidget->rowCount() > 0 && !ui->pageHistoriqueTableWidget->selectedRanges().isEmpty()) {
        QMenu menuContextuel(this);
        QAction *copier = menuContextuel.addAction(QIcon(), "Copier le nom de la série");
        QAction *a = menuContextuel.exec(ui->pageHistoriqueTableWidget->viewport()->mapToGlobal(pos));

        if(a == copier) {
            QApplication::clipboard()->setText(ui->pageHistoriqueTableWidget->item(ui->pageHistoriqueTableWidget->selectedRanges().at(0).topRow(), 0)->text());
        }
    }
}
