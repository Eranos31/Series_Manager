#include "fenetreprincipale.h"
#include "ui_fenetreprincipale.h"

FenetrePrincipale::FenetrePrincipale(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FenetrePrincipale) {
    ui->setupUi(this);

    log = new Log();

    colorisation();

    QDir dir;
    QFile file("config.ini");
    QSettings settings ("config.ini", QSettings::IniFormat);

    // Refresh automatique
    QTimer *timer = new QTimer();
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
    timer->start(300000);

    QTimer* timerDossierSerie = new QTimer();
    QObject::connect(timerDossierSerie, SIGNAL(timeout()), this, SLOT(majIndicateur()));
    timerDossierSerie->start(500);

    if(!QDir("Log").exists()) {
        dir.mkdir("Log");
    }

    if(!QDir("Lien").exists()) {
        dir.mkdir("Lien");
    }

    if(file.exists()) {
        if(settings.value("Configuration/Chemin").toString() == "") {
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
    ui->pageConfigurationBoutonAccueil->setVisible(false);
    ui->pageConfigLineEditExtension->setReadOnly(true);
    ui->pageConfigLineEditExtension->setText("https://www.t411.");
    ui->pageConfigurationRadioToutes->setChecked(true);
    ui->pageConfigurationRadioAucun->setChecked(true);
    ui->stackedWidget->setCurrentWidget(ui->pageConfig);
    ui->pageConfigurationLineDossierSerie->setReadOnly(true);
    log->ecrire("FenetrePrincipale::premiereConnexion() : Fin du paramétrage de première connexion");
}

void FenetrePrincipale::refresh() {
    log->ecrire("FenetrePrincipale::refresh() : Début de l'actualisation");
    // Met a jour les episodes de la veille
    majEpisode();
    // Verifie si une série est terminé
    verificationSerieTerminer();

    QList<QMap<QString,QString>> listeHier = bdd->requeteListe("SELECT NOM, SAISON, NBEPISODE, EPISODECOURANT, DATESORTIE, JOURSORTIE, DATEMODIF, WIKI "
                                                               "FROM SERIE "
                                                               "WHERE JOURSORTIE = '" + QString::number(QDate::currentDate().addDays(-1).dayOfWeek()) + "' "
                                                               "ORDER BY NOM");

    listeQuoti = bdd->requeteListe("SELECT NOM, SAISON, NBEPISODE, EPISODECOURANT, DATESORTIE, JOURSORTIE, DATEMODIF, WIKI "
                                   "FROM SERIE "
                                   "WHERE JOURSORTIE = '" + QString::number(QDate::currentDate().dayOfWeek()) + "' "
                                   "ORDER BY NOM");

    listeGlobal = bdd->requeteListe("SELECT NOM, SAISON, NBEPISODE, EPISODECOURANT, DATESORTIE, JOURSORTIE, DATEMODIF, WIKI "
                                    "FROM SERIE "
                                    "ORDER BY DATEMODIF, JOURSORTIE");

    QSettings settings ("config.ini", QSettings::IniFormat);
    bool quotiVide = true;

    //Effacement des tableau
    ui->pagePrincipaleTableWidgetDisplay->clear();
    ui->pageListeModificationDisplay->clear();
    ui->pageListeSupprimerDisplay->clear();
    log->ecrire("\tEffacement des tableau");

    on_pageVosSeriesComboBox_currentIndexChanged(settings.value("Configuration/ListeSerie", "Toutes").toString());

    // refresh de l'affichage
    // Mise en place des headers du tableau de la page principale
    QList<QString> liste;
    liste.append("Nom");
    liste.append("Saison");
    liste.append("Episode");
    liste.append("T411");
    liste.append("Wiki");
    liste.append("Reporter");

    ui->pagePrincipaleTableWidgetDisplay->setColumnCount(6);
    ui->pagePrincipaleTableWidgetDisplay->setRowCount(0);
    ui->pagePrincipaleTableWidgetDisplay->setHorizontalHeaderLabels(QStringList(liste));

    int indice = 0;

    // Refresh de l'onglet aujourd'hui
    for (int i = 0; i < listeQuoti.count(); i++) {
        QMap<QString, QString> map = listeQuoti.value(i);

        if(methodeDiverses.stringToDate(map["DATESORTIE"]) <= QDate::currentDate() &&
           methodeDiverses.stringToDate(map["DATEMODIF"]) <= QDate::currentDate() &&
           methodeDiverses.stringToDate(map["DATEMODIF"]) != QDate::currentDate()) {
            quotiVide = false;

            // Page Principale
            QSignalMapper* mapper1 = new QSignalMapper();
            QSignalMapper* mapper2 = new QSignalMapper();
            QSignalMapper* mapper3 = new QSignalMapper();

            QPushButton* t411 = new QPushButton(i_t411, "");
            QPushButton* wiki = new QPushButton(i_wiki, "");
            QPushButton* reporter = new QPushButton(i_reporter, "");

            t411->setToolTip("Ouvre le lien t411 de " + map["NOM"]);
            wiki->setToolTip("Ouvre le lien Wikipédia de " + map["NOM"]);
            reporter->setToolTip("Ouvre la fênetre de report de " + map["NOM"]);

            mapper1->setMapping(t411, map["NOM"] + "+S" + map["SAISON"] + "E" + map["EPISODECOURANT"]);
            mapper2->setMapping(wiki, map["WIKI"]);
            mapper3->setMapping(reporter, map["NOM"]);

            QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(on_bouton_t411_clicked(QString)));
            QObject::connect(t411, SIGNAL(clicked()), mapper1, SLOT(map()));

            QObject::connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(on_bouton_wiki_clicked(QString)));
            QObject::connect(wiki, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

            QObject::connect(mapper3, SIGNAL(mapped(QString)), this, SLOT(on_bouton_reporter_clicked(QString)));
            QObject::connect(reporter, SIGNAL(clicked(bool)), mapper3, SLOT(map()));

            ui->pagePrincipaleTableWidgetDisplay->setRowCount(indice+1);
            ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(map["NOM"], false));
            ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(map["SAISON"], true));
            ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(map["EPISODECOURANT"], true));
            ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 3, t411);
            ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 4, wiki);
            ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 5, reporter);

            indice++;

            if(map["WIKI"] == "") {
                wiki->setEnabled(false);
            }
        }
    }

    log->ecrire("\tActualisation de l'onglet Aujourd'hui");

    if(quotiVide) {
        ui->pagePrincipaleBoutonUrl->setEnabled(false);
        ui->pagePrincipaleBoutonWiki->setEnabled(false);
        log->ecrire("\tDésactivation des boutons globaux");
    } else {
        ui->pagePrincipaleBoutonUrl->setEnabled(true);
        ui->pagePrincipaleBoutonWiki->setEnabled(true);
        log->ecrire("\tActivation des boutons globaux");
    }

    // Refresh de l'onglet hier
    ui->pagePrincipaleTableWidgetDisplay_2->clear();
    ui->pagePrincipaleTableWidgetDisplay_2->setColumnCount(5);
    ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(0);
    QList<QString> list;
    list.append("Nom");
    list.append("Saison");
    list.append("Episode");
    list.append("T411");
    list.append("Wiki");
    ui->pagePrincipaleTableWidgetDisplay_2->setHorizontalHeaderLabels(QStringList(liste));

    indice = 0;

    for(int i = 0; i < listeHier.count(); i++) {
        QMap<QString, QString> mapHier = listeHier.value(i);

        if(methodeDiverses.stringToDate(mapHier["DATESORTIE"]) <= QDate::currentDate() &&
           methodeDiverses.stringToDate(mapHier["DATEMODIF"]) <= QDate::currentDate() &&
           methodeDiverses.stringToDate(mapHier["DATEMODIF"]) != QDate::currentDate()) {
            // Page Principale

            QSignalMapper* mapper1 = new QSignalMapper();
            QSignalMapper* mapper2 = new QSignalMapper();
            QSignalMapper* mapper3 = new QSignalMapper();

            QPushButton* t411Hier = new QPushButton(i_t411, "");
            QPushButton* wikiHier = new QPushButton(i_wiki, "");
            QPushButton* reporterHier = new QPushButton(i_reporter, "");

            t411Hier->setToolTip("Ouvre le lien t411 de " + mapHier["NOM"]);
            wikiHier->setToolTip("Ouvre le lien Wikipédia de " + mapHier["NOM"]);
            reporterHier->setToolTip("Ouvre la fênetre de report de " + mapHier["NOM"]);

            mapper1->setMapping(t411Hier, mapHier["NOM"] + "+S" + mapHier["SAISON"] + "E" + methodeDiverses.formalismeEntier(mapHier["EPISODECOURANT"].toInt()-1));
            mapper2->setMapping(wikiHier, mapHier["WIKI"]);
            mapper3->setMapping(reporterHier, mapHier["NOM"]);

            QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(on_bouton_t411_clicked(QString)));
            QObject::connect(t411Hier, SIGNAL(clicked()), mapper1, SLOT(map()));

            QObject::connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(on_bouton_wiki_clicked(QString)));
            QObject::connect(wikiHier, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

            QObject::connect(mapper3, SIGNAL(mapped(QString)), this, SLOT(on_bouton_reporter_clicked(QString)));
            QObject::connect(reporterHier, SIGNAL(clicked(bool)), mapper3, SLOT(map()));

            ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(indice+1);
            ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 0, methodeDiverses.itemForTableWidget(mapHier["NOM"], false));
            ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 1, methodeDiverses.itemForTableWidget(mapHier["SAISON"], true));
            ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 2, methodeDiverses.itemForTableWidget(methodeDiverses.formalismeEntier(mapHier["EPISODECOURANT"].toInt() - 1), true));
            ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 3, t411Hier);
            ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 4, wikiHier);

            if(mapHier["WIKI"] == "") {
                wikiHier->setEnabled(false);
            }

            indice++;
        }
    }

    log->ecrire("\tActualisation de l'onglet Hier");

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

    log->ecrire("\tDimensionnement des colonnes des tableaux");

    for (int i = 0; i < listeGlobal.count(); i++) {
        QMap<QString, QString> map = listeGlobal.value(i);
        ui->pageListeModificationDisplay->addItem(map["NOM"]);
        ui->pageListeSupprimerDisplay->addItem(map["NOM"]);
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
    foreach (QFileInfo file, QDir(dossierSerie).entryInfoList()) {
        if(file.isDir()) {
            QString nom;
            nom = file.absoluteFilePath();
            nom.replace(dossierSerie, "");
            nom.replace(file.absolutePath(), "");
            nom.replace("/","");

            if(nom != "") {
                ui->pageAjoutModifComboNom->addItem(nom);
                listeSerie.append(nom);
            }
        }
    }

    log->ecrire("\tAjout des noms des séries du dossier des série dans la liste déroulante de l'ajout de série");
    log->ecrire("FenetrePrincipale::refresh() : Fin de l'actualisation");

    ui->statusBar->showMessage("Refresh fait à " + QTime::currentTime().toString("HH:mm:ss"));
}

void FenetrePrincipale::initialisation() {
    log->ecrire("FenetrePrincipale::initialisation() : Début de l'initialisation");
    // Mettre le menu en visible
    ui->menuBar->setVisible(true);
    ui->pageReporterSpinBox->setMinimum(1);
    // Recuperation de la base de données ou création si elle n'existe pas
    this->bdd = new BaseDeDonnees();
    // Charge le fichier de configuration
    chargementConfiguration();
    // Place l'appli sur la page principale
    ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
    // Met l'onglet Aujourd'hui
    ui->pagePrincipaleTabWidget->setCurrentWidget(ui->pagePrincipaleTabWidgetTabAuj);
    // Rafraichis les listes
    refresh();
    // Crée le dossier de reception des téléchargement s'il n'existe pas
    if(!QDir(dossierSerie + "/_Telechargement").exists()) {
        QDir dir;
        dir.mkdir(dossierSerie + "/_Telechargement");
    }
    log->ecrire("FenetrePrincipale::initialisation() : Fin de l'initialisation");
}

void FenetrePrincipale::on_bouton_t411_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::on_bouton_t411_clicked() : Début de la génération du lien");
    QSettings settings("config.ini", QSettings::IniFormat);
    QString url = "https://www.t411." + settings.value("Configuration/Extension", "in").toString() + "/torrents/search/?search=";
    QString qualite = "";
    QString sousTitres = "";
    QString fin = "&order=added&type=desc";

    if(settings.value("Configuration/Qualite", "Toutes").toString() == "720p"){
        qualite = "+720p";
    } else if (settings.value("Configuration/Qualite", "Toutes").toString() == "1080p") {
        qualite = "+1080p";
    }

    if(settings.value("Configuration/Sous-Titres", "").toString() == "") {
        sousTitres = "";
    } else {
        sousTitres = "+vostfr";
    }

    QDesktopServices::openUrl(QUrl(url + nom + sousTitres + qualite + fin));
    log->ecrire("\tLien généré : " + url + nom + sousTitres + qualite + fin);

    log->ecrire("FenetrePrincipale::on_bouton_t411_clicked() : Fin de la génération du lien");
}

void FenetrePrincipale::on_bouton_dossier_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::on_bouton_dossier_clicked() : Début de l'ouverture du dossier");
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + nom))) {
        QMessageBox::information(this,"Attention", "Le dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
        log->ecrire("\tLe dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
    } else {
        log->ecrire("\tLe dossier " + dossierSerie + nom + " a été ouvert");
    }
    log->ecrire("FenetrePrincipale::on_bouton_dossier_clicked() : Fin de l'ouverture du dossier");
}

void FenetrePrincipale::on_bouton_wiki_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::on_bouton_wiki_clicked() : Début de génération du lien Wikipédia");
    if(QDesktopServices::openUrl(QUrl(nom))) {
        log->ecrire("\tOuverture du lien");
    } else {
        QMessageBox::warning(this, "Attention", "Le lien " + nom + " n'a pu être ouvert");
        log->ecrire("\tLe lien " + nom + " n'a pu être ouvert");
    }

    log->ecrire("FenetrePrincipale::on_bouton_wiki_clicked() : Début de génération du lien Wikipédia");
}

void FenetrePrincipale::on_bouton_reporter_clicked(QString nom) {
    log->ecrire("FenetrePrincipale::on_bouton_reporter_clicked() : Ouverture de la page de report de série");
    on_pageReporterSpinBox_valueChanged(ui->pageReporterSpinBox->value());
    ui->stackedWidget->setCurrentWidget(ui->pageReporter);
    ui->pageReporterLabel->setText("De combien de semaines voulez vous reporter " + nom + " ?");
    ui->pageReporterLabelNomSerie->setVisible(false);
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
}

void FenetrePrincipale::colorisation() {
    log->ecrire("FenetrePrincipale::colorisation() : Début de la colorisation de l'application");
    // bleu clair RGB 98, 206, 245| #5CCEF5
    // Noir RGB 0, 0, 0
    //qApp->setStyleSheet("FenetrePrincipale {background-color: #0000CC}");

    // Mettre les images sur les boutons
    // Menu Fichier
    ui->menuFichierAccueil->setIcon(i_accueil);
    ui->menuFichierAjouter->setIcon(i_valider);
    ui->menuFichierModifier->setIcon(i_modifier);
    ui->menuFichierSupprimer->setIcon(i_annuler);
    ui->menuFichierToutes_les_s_ries->setIcon(i_pellicule);
    ui->menuFichierQuitter->setIcon(i_exit);
    // Menu Options
    ui->menuOptionsActualiser->setIcon(i_actualiser);
    ui->menuOptionsParam_tres->setIcon(i_engrenage);
    ui->menuOptionsDossier_de_lien->setIcon(i_dossier);
    ui->menuOptionsDossier_de_log->setIcon(i_dossier);
    // Menu Aide
    ui->menuAideA_Propos->setIcon(i_interrogation);
    // Page Configuration
    ui->pageConfigurationBoutonAccueil->setIcon(i_accueil);
    ui->pageConfigurationBoutonTerminer->setIcon(i_valider);
    // Page Principale
    ui->pagePrincipaleBoutonActualiser->setIcon(i_actualiser);
    ui->pagePrincipaleBoutonAjouter->setIcon(i_valider);
    ui->pagePrincipaleBoutonDossierSerie->setIcon(i_dossier);
    ui->pagePrincipaleBoutonModifier->setIcon(i_modifier);
    ui->pagePrincipaleBoutonSupprimer->setIcon(i_annuler);
    ui->pagePrincipaleBoutonUrl->setIcon(i_t411);
    ui->pagePrincipaleBoutonWiki->setIcon(i_wiki);
    ui->pagePrincipaleBoutonVosSeries->setIcon(i_pellicule);
    ui->pagePrincipaleBoutonDeplacerFichier->setIcon(i_deplacement);
    // Page Vos Series
    ui->pageVosSeriesBoutonAccueil->setIcon(i_accueil);
    // Page Ajout Modification
    ui->pageAjoutModifBoutonAccueil->setIcon(i_accueil);
    ui->pageAjoutModifBoutonValider->setIcon(i_valider);
    ui->pageAjoutModifBoutonDossier->setIcon(i_dossier);
    ui->pageAjoutModifBoutonRetour->setIcon(i_retour);
    ui->pageAjoutModifBoutonWiki->setIcon(i_wiki);
    ui->pageAjoutModifButtonWiki->setIcon(i_wiki);
    // Page Liste Modification
    ui->pageListeModificationBoutonAccueil->setIcon(i_accueil);
    ui->pageListeModificationBoutonModifier->setIcon(i_modifier);
    ui->pageListeModificationBoutonRetour->setIcon(i_retour);
    // Page Liste Supprimer
    ui->pageListeSupprimerBoutonAccueil->setIcon(i_accueil);
    ui->pageListeSupprimerBoutonRetour->setIcon(i_retour);
    ui->pageListeSupprimerBoutonSupprimer->setIcon(i_annuler);
    // Page Reporter
    ui->pageReporterBoutonAccueil->setIcon(i_accueil);
    ui->pageReporterButtonRetour->setIcon(i_retour);
    ui->pageReporterButtonValider->setIcon(i_valider);

    log->ecrire("FenetrePrincipale::colorisation() : Fin de la colorisation de l'application");
}

void FenetrePrincipale::chargementConfiguration() {
    log->ecrire("FenetrePrincipale::chargementConfiguration() : Début du chargement du fichier de configuration");
    QSettings settings("config.ini", QSettings::IniFormat);
    QString listeSerie = settings.value("Configuration/ListeSerie", "Toutes").toString();

    if(listeSerie == "Toutes") {
        ui->pageVosSeriesComboBox->setCurrentIndex(0);
    } else if (listeSerie == "Série en cours") {
        ui->pageVosSeriesComboBox->setCurrentIndex(1);
    } else if (listeSerie == "Série à venir") {
        ui->pageVosSeriesComboBox->setCurrentIndex(2);
    }

    this->dossierSerie = settings.value("Configuration/Chemin", "C:/").toString();
    this->setGeometry(settings.value("Position/X_position", 724).toInt(), settings.value("Position/Y_position", 303).toInt(), settings.value("Taille/Width", 472).toInt(), settings.value("Taille/Height", 434).toInt());

    log->ecrire("FenetrePrincipale::chargementConfiguration() : Fin du chargement du fichier de configuration");
}

void FenetrePrincipale::majEpisode() {
    log->ecrire("FenetrePrincipale::majEpisode() : Début de mise à jour des episodes de la veille");

    QSettings settings("config.ini", QSettings::IniFormat);
    QString url = "https://www.t411." + settings.value("Configuration/Extension", "in").toString() + "/torrents/search/?search=";
    QString qualite = "";
    QString sousTitres = "";
    QDate dateDerniereOuverture = bdd->derniereOuvertureBDD();
    QDate dateDerniereOuverturePlus1 = dateDerniereOuverture.addDays(1);

    if(settings.value("Configuration/Qualite", "Toutes").toString() == "720p"){
        qualite = "+720p";
    } else if (settings.value("Configuration/Qualite", "Toutes").toString() == "1080p") {
        qualite = "+1080p";
    }

    if(settings.value("Configuration/Sous-Titres", "").toString() == "vostfr") {
        sousTitres = "+vostfr";
    } else if(settings.value("Configuration/Sous-Titres", "").toString() == "") {
        sousTitres = "";
    }

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

                        QDir dir;
                        dir.mkdir("Lien/" + dateDerniereOuverture.toString("yyyy_MM_dd"));

                        QFile file("Lien/" + dateDerniereOuverture.toString("yyyy_MM_dd") + "/" + list["NOM"] + " S" + list["SAISON"] + "E" + list["EPISODECOURANT"] + ".url");

                        if(!file.exists()) {
                            if(file.open(QIODevice::WriteOnly|QIODevice::Text)) {
                                log->ecrire("\t\tCréation du raccourci internet de " + list["NOM"]);
                                QTextStream flux(&file);
                                flux << "[InternetShortcut]" << endl;
                                flux <<"URL=" + url + list["NOM"].replace(" ", "+") + "+S" + list["SAISON"] + "E" + list["EPISODECOURANT"] + sousTitres + qualite + "&order=added&type=desc" << endl;
                            }
                        }
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

void FenetrePrincipale::nettoyerDossierTelechargement() {
    log->ecrire("FenetrePrincipale::nettoyerDossierTelechargement() : Début du nettoyage du dossier de téléchargement");
    foreach (QFileInfo info, QDir(dossierSerie + "/_Telechargement").entryInfoList()) {
        if(info.isFile()) {
            if(!QFile(info.absoluteFilePath()).remove()) {
                log->ecrire("WARNING : Le dossier de téléchargement ne s'est pas purgé correctement");
            } else {
                log->ecrire("\tLe fichier " + info.fileName() + " a été supprimé");
            }
        }
    }
    log->ecrire("FenetrePrincipale::nettoyerDossierTelechargement() : Fin du nettoyage du dossier de téléchargement");
}

void FenetrePrincipale::resizeEvent(QResizeEvent *) {
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setValue("Taille/Width", QString::number(this->geometry().width()));
    settings.setValue("Taille/Height", QString::number(this->geometry().height()));
}

void FenetrePrincipale::moveEvent(QMoveEvent *) {
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setValue("Position/X_position", QString::number(this->geometry().x()));
    settings.setValue("Position/Y_position",QString::number(this->geometry().y()));
}

/*******************************************************\
*                                                       *
*                          MENU                         *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_menuFichierAccueil_triggered() {
    ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
}

void FenetrePrincipale::on_menuFichierToutes_les_s_ries_triggered() {
    if(listeGlobal.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageVosSeries);
        on_pageVosSeriesComboBox_currentIndexChanged(ui->pageVosSeriesComboBox->currentText());
    } else {
        QMessageBox::information(this, "Pas de série", "Il n'y a pas de série !");
    }
}

void FenetrePrincipale::on_menuFichierAjouter_triggered() {
    ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
    ui->pageAjoutModifTitre->setText("Ajouter série");
    ui->pageAjoutModifBoutonValider->setText("Ajouter");
    ui->pageAjoutModifComboNom->setCurrentText("");
    ui->pageAjoutModifLineSaison->setValue(1);
    ui->pageAjoutModifLineSaison->clear();
    ui->pageAjoutModifLineNbEpisode->setValue(1);
    ui->pageAjoutModifLineNbEpisode->clear();
    ui->pageAjoutModifLabelEpisodeCourant->setVisible(false);
    ui->pageAjoutModifLineEpisodeCourant->setVisible(false);
    ui->pageAjoutModifComboNom->setEnabled(true);
    ui->pageAjoutModifBoutonValider->setIcon(i_valider);
    ui->pageAjoutModifLineDateSortie->setEnabled(true);
    ui->pageAjoutModifLineDateSortie->setDate(QDate::currentDate());
    ui->pageAjoutModifBoutonDossier->setVisible(false);
    ui->pageAjoutModifBoutonWiki->setVisible(false);
    ui->pageAjoutModifLabelDateProchain->setVisible(false);
    ui->pageAjoutModificationLabelProchain->setVisible(false);
    ui->pageAjoutModifLineDateProchain->setVisible(false);
    ui->pageAjoutModifLineWiki->setText("");
}

void FenetrePrincipale::on_menuFichierModifier_triggered() {
    if(listeGlobal.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageListeModification);
        ui->pageAjoutModifLabelEpisodeCourant->setVisible(true);
        ui->pageAjoutModifLineEpisodeCourant->setVisible(true);
        ui->pageAjoutModifComboNom->setEnabled(false);
        ui->pageAjoutModifBoutonValider->setIcon(i_modifier);
        ui->pageAjoutModifBoutonDossier->setVisible(true);
        ui->pageAjoutModifBoutonWiki->setVisible(true);
        ui->pageAjoutModifLabelDateProchain->setVisible(true);
        ui->pageAjoutModificationLabelProchain->setVisible(true);
        ui->pageAjoutModifLineDateProchain->setVisible(true);
    } else {
        QMessageBox::information(this, "Pas de série", "Il n'y a pas de série !");
    }

    if(ui->pageListeModificationDisplay->count() != 0) {
        ui->pageListeModificationDisplay->item(0)->setSelected(true);
        ui->pageListeModificationDisplay->setCurrentItem(ui->pageListeModificationDisplay->item(0));
    }
}

void FenetrePrincipale::on_menuFichierSupprimer_triggered() {
    if(listeGlobal.count() != 0) {
        ui->stackedWidget->setCurrentWidget(ui->pageListeSupprimer);
    } else {
        QMessageBox::information(this, "Pas de série", "Il n'y a pas de série !");
    }

    if(ui->pageListeSupprimerDisplay->count() != 0){
        ui->pageListeSupprimerDisplay->item(0)->setSelected(true);
        ui->pageListeSupprimerDisplay->setCurrentItem(ui->pageListeSupprimerDisplay->item(0));
    }
}

void FenetrePrincipale::on_menuFichierQuitter_triggered() {
    log->ecrire("Fermeture de l'application");
    QCoreApplication::quit();
}

void FenetrePrincipale::on_menuOptionsActualiser_triggered() {
    refresh();
}

void FenetrePrincipale::on_menuOptionsParam_tres_triggered() {
    QSettings settings("config.ini", QSettings::IniFormat);
    QString qualite = settings.value("Configuration/Qualite", "Toutes").toString();

    if(qualite == "Toutes") {
        ui->pageConfigurationRadioToutes->setChecked(true);
    } else if (qualite == "720p") {
        ui->pageConfigurationRadio720p->setChecked(true);
    } else if(qualite == "1080p") {
        ui->pageConfigurationRadio1080p->setChecked(true);
    }

    ui->pageConfigLineEditExtension_2->setText(settings.value("Configuration/Extension", "in").toString());
    ui->pageConfigurationLineDossierSerie->setText(settings.value("Configuration/Chemin", "C:/").toString());
    ui->pageConfigurationBoutonAccueil->setVisible(true);
    ui->stackedWidget->setCurrentWidget(ui->pageConfig);
    ui->pageConfigurationLabelConsigne->setText("");
    ui->pageConfigLineEditExtension->setReadOnly(true);
}

void FenetrePrincipale::on_menuOptionsDossier_de_lien_triggered() {
    QDesktopServices::openUrl(QUrl::fromLocalFile("./Lien"));
}

void FenetrePrincipale::on_menuOptionsDossier_de_log_triggered() {
    QDesktopServices::openUrl(QUrl::fromLocalFile("./Log"));
}

void FenetrePrincipale::on_menuAideA_Propos_triggered() {
    QMessageBox msgBox;
    msgBox.setText("Version 2.1\nBuild du 12/01/2016 - 21:43:00<br/><br/>"
                   "Le projet Series Manager est venu le jour où le nombre de séries "
                   "que je regardais commençait à atteindre un nombre impressionnant.<br/><br/>"
                   "J'ai donc eu l'idée de créé un programme permettant de lister les "
                   "séries de la saison et de créé automatiquement les liens de recherches "
                   "sur le site de téléchargement T411.<br/><br/>"
                   "&copy;2014-2016 Eranos Corporation. Tout droits réservés.<br/><br/>"
                   "Ce projet a été développé sous Qt Creator 3.1.2<br/>"
                   "Plus d'info sur Qt : <a href='https://www.qt.io/'>https://www.qt.io/</a><br/>");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setIconPixmap(i_seriesManager);
    msgBox.exec();
}

/*******************************************************\
*                                                       *
*                  PAGE CONFIGURATION                   *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageConfigurationBoutonAccueil_clicked() {
    on_menuFichierAccueil_triggered();
}

void FenetrePrincipale::on_pageConfigurationBoutonParcourir_clicked() {
    ui->pageConfigurationLineDossierSerie->setText(QFileDialog::getExistingDirectory(this, tr("Choisir un dossier"), "Ordinateur", QFileDialog::ShowDirsOnly));
}

void FenetrePrincipale::on_pageConfigurationBoutonTerminer_clicked() {
    QString qualite;
    QString sousTitres;

    if(ui->pageConfigurationRadioToutes->isChecked()){
        qualite = "Toutes";
    } else if (ui->pageConfigurationRadio720p->isChecked()) {
        qualite = "720p";
    } else if (ui->pageConfigurationRadio1080p->isChecked()) {
        qualite = "1080p";
    }

    if(ui->pageConfigurationRadioVOSTFR->isChecked()) {
        sousTitres = "vostfr";
    } else if(ui->pageConfigurationRadioAucun->isChecked()) {
        sousTitres = "";
    }

    if(ui->pageConfigurationLineDossierSerie->text() != "") {
        QSettings settings ("config.ini", QSettings::IniFormat);
        settings.beginGroup("Configuration");
        settings.setValue("Extension", ui->pageConfigLineEditExtension_2->text());
        settings.setValue("Qualite", qualite);
        settings.setValue("Chemin", ui->pageConfigurationLineDossierSerie->text());
        settings.setValue("ListeSerie", "Toutes");
        settings.setValue("Sous-Titres", sousTitres);
        settings.endGroup();
        settings.beginGroup("Taille");
        settings.setValue("Width", QString::number(this->geometry().width()));
        settings.setValue("Height", QString::number(this->geometry().height()));
        settings.endGroup();
        settings.beginGroup("Position");
        settings.setValue("X_position", QString::number(this->geometry().x()));
        settings.setValue("Y_position", QString::number(this->geometry().y()));
        if(!ui->pageConfigurationBoutonAccueil->isVisible()) {
            initialisation();
        }
        dossierSerie =  ui->pageConfigurationLineDossierSerie->text();
        ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
    } else {
        QMessageBox::warning(this, "Attention", "Veuillez remplir toutes les informations demandées !");
    }
}

/*******************************************************\
*                                                       *
*                   PAGE PRINCIPALE                     *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pagePrincipaleBoutonActualiser_clicked() {
    on_menuOptionsActualiser_triggered();
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
    QSettings settings("config.ini", QSettings::IniFormat);
    QString url = "https://www.t411." + settings.value("Configuration/Extension", "in").toString() + "/torrents/search/?search=";
    QString qualite = "";
    QString sousTitres = "";

    if(settings.value("Configuration/Qualite", "Toutes").toString() == "720p"){
        qualite = "+720p";
    } else if (settings.value("Configuration/Qualite", "Toutes").toString() == "1080p") {
        qualite = "+1080p";
    }

    if(settings.value("Configuration/Sous-Titres", "").toString() == "vostfr") {
        sousTitres = "+vostfr";
    } else if(settings.value("Configuration/Sous-Titres", "").toString() == "") {
        sousTitres = "";
    }

    if(this->listeQuoti.isEmpty()){
        QMessageBox::information(this, "Erreur", "Il n'y a pas de série à regarder !");
    } else {
        bool trouve = false;

        for (int i = 0; i < listeQuoti.count(); ++i) {
            QMap<QString,QString> list = listeQuoti.value(i);
            if(methodeDiverses.stringToDate(list["DATESORTIE"]) <= QDate::currentDate() &&
               methodeDiverses.stringToDate(list["DATEMODIF"]) <= QDate::currentDate() &&
               methodeDiverses.stringToDate(list["DATEMODIF"]) != QDate::currentDate()) {
                QString nomSerie = list["NOM"];
                QDesktopServices::openUrl(QUrl(url + nomSerie.replace(" ", "+") + "+S" + list["SAISON"] + "E" + list["EPISODECOURANT"] + sousTitres + qualite + "&order=added&type=desc"));
                trouve = true;
            }
        }

        if(!trouve) {
            QMessageBox::information(this, "Erreur", "Il n'y a pas de série à regarder !");
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() {
    log->ecrire("FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() : Début de l'ouverture du dossier de téléchargement");
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + "/_Telechargement"))) {
        log->ecrire("\tOuverture du dossier de téléchargement");
    } else {
        QMessageBox::warning(this, "Attention", "Le dossier de téléchargement n'a pas pu être ouvert");
        log->ecrire("\tLe dossier de téléchargement n'a pas pu être ouvert");
    }
    log->ecrire("FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() : Début de l'ouverture du dossier de téléchargement");
}

void FenetrePrincipale::on_pagePrincipaleBoutonWiki_clicked() {
    if(this->listeQuoti.isEmpty()){
        QMessageBox::information(this, "Erreur", "Il n'y a pas de série à regarder !");
    } else {
        bool trouve = false;

        for (int i = 0; i < listeQuoti.count(); ++i) {
            QMap<QString,QString> list = listeQuoti.value(i);
            if(methodeDiverses.stringToDate(list["DATESORTIE"]) <= QDate::currentDate() &&
               methodeDiverses.stringToDate(list["DATEMODIF"]) <= QDate::currentDate() &&
               methodeDiverses.stringToDate(list["DATEMODIF"]) != QDate::currentDate()) {
                QDesktopServices::openUrl(QUrl(list["WIKI"]));
                trouve = true;
            }
        }

        if(!trouve) {
            QMessageBox::information(this, "Erreur", "Il n'y a pas de série à regarder !");
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleBoutonDeplacerFichier_clicked() {
    log->ecrire("FenetrePrincipale::on_pagePrincipaleBoutonDeplacerFichier_clicked() : Début du déplacement des dossiers du dossier de téléchargement");

    int fichierTotal = 0;
    int fichierDeplace = 0;

    if(QDir(dossierSerie + "/_Telechargement").entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() != 0) {
        if(!listeGlobal.isEmpty()) {
            log->ecrire("Déplacement des fichiers téléchargés");
            foreach (QFileInfo info, QDir(dossierSerie + "/_Telechargement").entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries)) {
                for(int i = 0; i < listeSerie.count(); i++) {

                    if(info.fileName().contains(listeSerie.at(i), Qt::CaseInsensitive) || info.fileName().contains(QString(listeSerie.at(i)).replace(" ", "."), Qt::CaseInsensitive)) {
                        fichierTotal++;

                        QFile fichier (info.absoluteFilePath());
                        QString saison = info.fileName().mid(info.fileName().indexOf(QRegularExpression("S[0-9]{2}E[0-9]{2}")) + 1, 2);
                        QFile *newFichier = new QFile(dossierSerie + "\\" + listeSerie.at(i) + "\\Saison " + saison + "\\" + listeSerie.at(i) + " " + info.fileName().mid(info.fileName().indexOf(QRegularExpression("S[0-9]{2}E[0-9]{2}"), 0), 6) + "." + info.suffix());

                        if(newFichier->exists()) {
                            if(methodeDiverses.msgBoxQuestion("Le fichier " + newFichier->fileName() + " existe déjà voulez vous le remplacer ?\nSi vous cliquer sur non le fichier sera supprimé") == 0) {
                                newFichier->remove();
                            } else {
                                fichier.remove();
                                continue;
                            }
                        }

                        if(fichier.rename(dossierSerie + "\\" + listeSerie.at(i) + "\\Saison " + saison + "\\" + listeSerie.at(i) + " " + info.fileName().mid(info.fileName().indexOf(QRegularExpression("S[0-9]{2}E[0-9]{2}"), 0), 6) + "." + info.suffix())) {
                            log->ecrire("\tEmplacement du nouveau fichier : " + dossierSerie + "\\" + listeSerie.at(i) + "\\Saison " + saison + "\\" + listeSerie.at(i) + " " + info.fileName().mid(info.fileName().indexOf(QRegularExpression("S[0-9]{2}E[0-9]{2}"), 0), 6) + "." + info.suffix());
                            fichierDeplace++;
                        } else {
                            log->ecrire("\tLe fichier " + listeSerie.at(i) + " " + info.fileName().mid(info.fileName().indexOf(QRegularExpression("S[0-9]{2}E[0-9]{2}"), 0), 6) + "." + info.suffix() + " n'a pas été déplacé");
                        }
                    }
                }
            }

            if(fichierDeplace == fichierTotal) {
                if(fichierDeplace == 1) {
                    QMessageBox::information(this, "Fichier déplacé", "Le fichier a été déplacé");
                    log->ecrire("Le fichier a été déplacé");
                } else if (fichierDeplace > 1) {
                    QMessageBox::information(this, "Fichiers déplacés", "Les " + QString::number(fichierTotal) + " fichiers ont été déplacés");
                    log->ecrire("Les " + QString::number(fichierTotal) + " fichiers ont été déplacés");
                } else  if(fichierDeplace == 0){
                    QMessageBox::information(this, "Fichiers déplacés", "Il n'y a pas de fichier à déplacer");
                }

                nettoyerDossierTelechargement();
            }

            log->ecrire("FenetrePrincipale::on_pagePrincipaleBoutonDeplacerFichier_clicked() : Fin du déplacement des dossiers du dossier de téléchargement");

        }
    } else {
        QMessageBox::information(this, "Information", "Il n'y a pas de fichiers à déplacer");
    }
}

/*******************************************************\
*                                                       *
*                PAGE TOUTE LES SERIES                  *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageVosSeriesBoutonAccueil_clicked() {
    on_menuFichierAccueil_triggered();
}

void FenetrePrincipale::on_pageVosSeriesComboBox_currentIndexChanged(const QString &arg1) {
    QSettings settings ("config.ini", QSettings::IniFormat);

    ui->pageVosSeriesDisplay->clear();
    ui->pageVosSeriesDisplay->setColumnCount(7);

    QList<QString> liste;
    liste.append("Nom");
    liste.append("Saison");
    liste.append("Jour de sortie");
    liste.append("Diffusion");
    liste.append("Prochain le");
    liste.append("Dossier");
    liste.append("Wikipédia");

    ui->pageVosSeriesDisplay->setHorizontalHeaderLabels(QStringList(liste));

    int selection;

    if(arg1 == "Toutes") {
        settings.setValue("Configuration/ListeSerie", "Toutes");
        selection = 1;
    } else if (arg1 == "Série à venir") {
        settings.setValue("Configuration/ListeSerie", "Série à venir");
        selection = 2;
    } else if (arg1 == "Série en cours") {
        settings.setValue("Configuration/ListeSerie", "Série en cours");
        selection = 3;
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

        QSignalMapper* mapper1 = new QSignalMapper();
        QPushButton* wiki = new QPushButton(i_wiki,"");
        mapper1->setMapping(wiki, list["WIKI"]);
        QObject::connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(on_bouton_wiki_clicked(QString)));
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

void FenetrePrincipale::on_pageAjoutModifBoutonAccueil_clicked() {
    on_menuFichierAccueil_triggered();
}

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
            QDir dir;
            dir.mkdir(dossierSerie + "/" + ui->pageAjoutModifComboNom->currentText());
            dir.mkdir(dossierSerie + "/" + ui->pageAjoutModifComboNom->currentText() + "/Saison " + methodeDiverses.formalismeEntier(ui->pageAjoutModifLineSaison->text().toInt()));
            ui->pageAjoutModifComboNom->clear();
            ui->pageAjoutModifLineSaison->clear();
            ui->pageAjoutModifLineNbEpisode->clear();
            ui->pageAjoutModifLineDateSortie->setDate(QDate::currentDate());
            ui->pageAjoutModifLineWiki->clear();
            refresh();
        } else {
            QMessageBox::information(this, "Erreur", "Veuillez remplir tout les champs !");
        }

    } else if(ui->pageAjoutModifBoutonValider->text() == "Modifier") {
        if(ui->pageAjoutModifLineDateSortie->isEnabled()) {
            this->bdd->modifier(ui->pageAjoutModifComboNom->currentText(), ui->pageAjoutModifLineSaison->text().toInt(), ui->pageAjoutModifLineNbEpisode->text().toInt(), ui->pageAjoutModifLineEpisodeCourant->text().toInt(), ui->pageAjoutModifLineDateSortie->date().dayOfWeek(), ui->pageAjoutModifLineDateSortie->date(), ui->pageAjoutModifLineWiki->text(), ui->pageAjoutModifLineDateSortie->date().addDays(-7), true);
        } else {
            this->bdd->modifier(ui->pageAjoutModifComboNom->currentText(), ui->pageAjoutModifLineSaison->text().toInt(), ui->pageAjoutModifLineNbEpisode->text().toInt(), ui->pageAjoutModifLineEpisodeCourant->text().toInt(), ui->pageAjoutModifLineDateProchain->date().dayOfWeek(), ui->pageAjoutModifLineDateSortie->date(), ui->pageAjoutModifLineWiki->text(), ui->pageAjoutModifLineDateProchain->date().addDays(-7), true);
        }
        ui->stackedWidget->setCurrentWidget(ui->pageListeModification);
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
            if(ui->pageAjoutModifLineWiki->text().contains(QRegExp("fr\.wikipedia\.org.*"))) {
                QDesktopServices::openUrl(QUrl(ui->pageAjoutModifLineWiki->text()));
            } else {
                QMessageBox::warning(this, "Attention", "L'adresse saisie n'est pas un lien vers Wikipedia !");
            }
        } else {
            QMessageBox::information(this, "Pas de lien", "Il n'y a pas de lien !");
        }
    }
}

void FenetrePrincipale::on_pageAjoutModifBoutonRetour_clicked() {
    if(ui->pageAjoutModifBoutonValider->text() == "Ajouter") {
        ui->stackedWidget->setCurrentWidget(ui->pagePrincipale);
    }
    if(ui->pageAjoutModifBoutonValider->text() == "Modifier") {
        ui->stackedWidget->setCurrentWidget(ui->pageListeModification);
    }
}

void FenetrePrincipale::on_pageAjoutModifLineDateProchain_userDateChanged(const QDate &date) {
    ui->pageAjoutModifLabelDateProchain->setText(methodeDiverses.formatDate(date));
}


/*******************************************************\
*                                                       *
*               PAGE LISTE MODIFICATION                 *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageListeModificationBoutonAccueil_clicked() {
    on_menuFichierAccueil_triggered();
}

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

void FenetrePrincipale::on_pageListeSupprimerBoutonAccueil_clicked() {
    on_menuFichierAccueil_triggered();
}

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

void FenetrePrincipale::on_pageReporterBoutonAccueil_clicked() {
    on_menuFichierAccueil_triggered();
}

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
