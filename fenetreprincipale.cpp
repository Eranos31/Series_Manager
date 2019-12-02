#include "fenetreprincipale.h"
#include "ui_fenetreprincipale.h"

FenetrePrincipale::FenetrePrincipale(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FenetrePrincipale) {
    ui->setupUi(this);
    this->version = "2.8.2";

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

    // Refresh automatique
    QTimer *timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
    timer->start(300000);

    QTimer* timerIndicateur = new QTimer();
    connect(timerIndicateur, SIGNAL(timeout()), this, SLOT(majIndicateur()));
    timerIndicateur->start(1000);

    if(file->exists()) {
        if(MethodeDiverses::getConfig("Configuration/Chemin") == "" ||
           MethodeDiverses::getConfig("Configuration/Telechargement") == "" ||
           MethodeDiverses::getConfig("Configuration/Extension") == "" ||
           MethodeDiverses::getConfig("Configuration/PurgeLog") == "") {
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
    MethodeDiverses::ecrireLog("FenetrePrincipale::premiereConnexion() : Début du paramétrage de première connexion");
    ui->menuBar->setVisible(false);
    ui->mainToolBar->setVisible(false);
    ui->pagePrincipaleLabelDossierSerie->setVisible(false);
    ui->pagePrincipaleLabelDossierSerie_2->setVisible(false);
    ui->pagePrincipaleLabelInternet->setVisible(false);
    ui->pagePrincipaleLabelInternet_2->setVisible(false);
    ui->stackedWidget->setCurrentWidget(ui->pageConfig);
    ui->pageConfigurationSpinBoxLogEfface->setValue(MethodeDiverses::getConfig("Configuration/PurgeLog").toInt());
    ui->pageConfigurationLineDossierTelechargement->setText(MethodeDiverses::getConfig("Configuration/Telechargement"));
    ui->pageConfigurationLineDossierSerie->setText(MethodeDiverses::getConfig("Configuration/Chemin"));
    ui->pageConfigurationLineEditExtension->setText(MethodeDiverses::getConfig("Configuration/Extension"));
    switch (MethodeDiverses::getConfig("Configuration/Qualite").toInt()) {
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
    switch (MethodeDiverses::getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        ui->pageConfigurationRadioButtonSousTitresToutes->setChecked(true);
        break;
    case 1:
        ui->pageConfigurationRadioButtonSousTitresVOSTFR->setChecked(true);
        break;
    }
    MethodeDiverses::ecrireLog("FenetrePrincipale::premiereConnexion() : Fin du paramétrage de première connexion");
}

void FenetrePrincipale::initialisation() {
    MethodeDiverses::ecrireLog("FenetrePrincipale::initialisation() : Début de l'initialisation");
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
    ui->pageAjoutModifSpinBoxEpisodeEnPlus->setEnabled(false);
    // Recuperation de la base de données ou création si elle n'existe pas
    bdd = new BaseDeDonnees("./data");
    // Récupération de la version de l'appli
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
    // Ajout des icones sur le bouton des liens torrent et addicted
    QHBoxLayout *layout = new QHBoxLayout();
    QLabel *l1 = new QLabel();
    QLabel *l2 = new QLabel();

    int h = ui->pagePrincipaleBoutonLienEtAddicted->height() * 0.5;
    l1->setFixedSize(QSize(48, h));
    l2->setFixedSize(QSize(64, h));

    l1->setPixmap(pix_torrent.scaled(l1->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    l2->setPixmap(pix_addicted.scaled(l2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(l1);
    layout->addWidget(l2);
    ui->pagePrincipaleBoutonLienEtAddicted->setLayout(layout);
    // Rafraichis les listes
    refresh();
    MethodeDiverses::ecrireLog("FenetrePrincipale::initialisation() : Fin de l'initialisation");
}

void FenetrePrincipale::refresh() {
    MethodeDiverses::ecrireLog("FenetrePrincipale::refresh() : Début de l'actualisation");
    // Met a jour les episodes de la veille
    majEpisode();

    QStringList champs, jointures, conditions, ordres;
    champs.append(bdd->HISTORIQUE_NOM);
    champs.append(bdd->HISTORIQUE_SAISON);
    champs.append(bdd->HISTORIQUE_EPISODE);
    champs.append(bdd->HISTORIQUE_ETAT);
    conditions.append(bdd->HISTORIQUE_DATE_AJOUT + bdd->EGALE + bdd->entreQuotes(QDate::currentDate().addDays(-1).toString("yyyy-MM-dd")));
    ordres.append(bdd->HISTORIQUE_NOM);
    ordres.append(bdd->HISTORIQUE_EPISODE);
    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les séries de la veille";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::FenetrePrincipale::refresh() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString,QString>> listeHier = retour.liste;

    champs.clear();
    conditions.clear();
    ordres.clear();

    champs.append(bdd->FICHE_SERIE_NOM);
    champs.append(bdd->FICHE_SERIE_WIKI);
    champs.append(bdd->FICHE_SERIE_ADDICTED);
    champs.append(bdd->FICHE_SERIE_TERMINE);
    champs.append(bdd->SAISON_SAISON);
    champs.append(bdd->SAISON_NB_EPISODE);
    champs.append(bdd->SAISON_EPISODE_COURANT);
    champs.append(bdd->SAISON_DATE_SORTIE);
    champs.append(bdd->CASE + bdd->WHEN + bdd->SAISON_DATE_MODIF + bdd->IS_NULL + bdd->THEN + bdd->entreQuotes("3000-01-01") + bdd->ELSE + bdd->SAISON_DATE_MODIF + bdd->END + bdd->SAISON_DATE_MODIF);
    champs.append(bdd->SAISON_WIKI);
    champs.append(bdd->SAISON_EPISODE_EN_PLUS);
    champs.append(bdd->SAISON_ETAT);
    champs.append(bdd->TYPE_SERIE_NOM);
    champs.append(bdd->TYPE_SERIE_RED);
    champs.append(bdd->TYPE_SERIE_GREEN);
    champs.append(bdd->TYPE_SERIE_BLUE);
    jointures.append(bdd->LEFT_JOIN + bdd->TABLE_SAISON + bdd->ON + bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->SAISON_ID);
    jointures.append(bdd->INNER_JOIN + bdd->TABLE_TYPE_SERIE + bdd->ON + bdd->FICHE_SERIE_TYPE_SERIE_ID + bdd->EGALE + bdd->TYPE_SERIE_ID);
    ordres.append(bdd->SAISON_DATE_MODIF);
    ordres.append(bdd->FICHE_SERIE_NOM);

    retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les séries";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::refresh() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    listeGlobal = retour.liste;

    ordres.clear();
    jointures.clear();

    jointures.append(bdd->INNER_JOIN + bdd->TABLE_SAISON + bdd->ON + bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->SAISON_ID);
    jointures.append(bdd->INNER_JOIN + bdd->TABLE_TYPE_SERIE + bdd->ON + bdd->FICHE_SERIE_TYPE_SERIE_ID + bdd->EGALE + bdd->TYPE_SERIE_ID);
    ordres.append(bdd->FICHE_SERIE_NOM);
    conditions.append(bdd->SAISON_DATE_MODIF + bdd->EGALE + bdd->entreQuotes(QDate::currentDate().addDays(-7).toString("yyyy-MM-dd")));

    retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les séries du jour";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::refresh() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    listeQuoti = retour.liste;

    int indice = 0;
    bool activationBoutonWikiGlobal = false;
    QString qualite;
    QString sousTitres;
    QString fin = "&do=search&order=desc&sort=publish_date";

    switch (MethodeDiverses::getConfig("Configuration/Qualite").toInt()) {
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

    switch (MethodeDiverses::getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        sousTitres = "";
        break;
    case 1:
        sousTitres = "+VOSTFR";
        break;
    }

    //Effacement des tableau
    MethodeDiverses::ecrireLog("\tEffacement des liste effectué");

    on_pageVosSeriesComboBox_currentIndexChanged(MethodeDiverses::getConfig("Configuration/ListeSerie"));
    MethodeDiverses::ecrireLog("\tAjout des données dans le tableau vos séries");

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
        QSignalMapper* mapper5 = new QSignalMapper();

        QPushButton* url = new QPushButton(i_torrent, "");
        QPushButton* wiki = new QPushButton(i_wiki, "");
        QPushButton* reporter = new QPushButton(i_reporter, "");
        QPushButton* addicted = new QPushButton(i_addicted, "");
        QComboBox *etat = new QComboBox();

        etat->addItem("Non vu");
        etat->addItem("Téléchargé");
        etat->addItem("Vu");

        url->setToolTip("Ouvre le lien URL de " + map.value(bdd->FICHE_SERIE_NOM));
        wiki->setToolTip("Ouvre le lien Wikipédia de " + map.value(bdd->FICHE_SERIE_NOM));
        reporter->setToolTip("Ouvre la fênetre de report de " + map.value(bdd->FICHE_SERIE_NOM));
        addicted->setToolTip("Ouvre le lien Addic7ed de " + map.value(bdd->FICHE_SERIE_NOM));

        mapper1->setMapping(url, lien + QString(map.value(bdd->FICHE_SERIE_NOM)).replace(" ", "+") + "+S" + map.value(bdd->SAISON_SAISON) + "E" + map.value(bdd->SAISON_EPISODE_COURANT) + sousTitres + qualite + fin);
        if(map.value(bdd->SAISON_WIKI) != "") {
            mapper2->setMapping(wiki, map.value(bdd->SAISON_WIKI));
        } else if (map.value(bdd->FICHE_SERIE_WIKI) != "") {
            mapper2->setMapping(wiki, map.value(bdd->FICHE_SERIE_WIKI));
        } else {
            mapper2->setMapping(wiki, "");
            wiki->setEnabled(false);
        }
        mapper3->setMapping(reporter, map.value(bdd->FICHE_SERIE_NOM));
        if(map.value(bdd->FICHE_SERIE_ADDICTED) != "") {
            mapper4->setMapping(addicted, map.value(bdd->FICHE_SERIE_ADDICTED) + "/" + QString::number(map.value(bdd->SAISON_SAISON).toInt()) + "/" + QString::number(map.value(bdd->SAISON_EPISODE_COURANT).toInt()) + "/8");
        } else {
            addicted->setEnabled(false);
        }

        connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(url, SIGNAL(clicked()), mapper1, SLOT(map()));

        connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(wiki, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

        connect(mapper3, SIGNAL(mapped(QString)), this, SLOT(bouton_reporter_clicked(QString)));
        connect(reporter, SIGNAL(clicked(bool)), mapper3, SLOT(map()));

        connect(mapper4, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(addicted, SIGNAL(clicked(bool)), mapper4, SLOT(map()));

        QWidget *widgetEtat = new QWidget();
        QHBoxLayout *layoutEtat = new QHBoxLayout();
        layoutEtat->addWidget(etat);
        layoutEtat->setAlignment(Qt::AlignCenter);
        layoutEtat->setContentsMargins(0,0,0,0);
        widgetEtat->setLayout(layoutEtat);

        if(map.value(bdd->SAISON_ETAT) == "NV") {
            etat->setCurrentText("Non vu");
        } else if (map.value(bdd->SAISON_ETAT) == "T") {
            etat->setCurrentText("Téléchargé");
        } else {
            etat->setCurrentText("Vu");
        }

        mapper5->setMapping(etat, map.value(bdd->FICHE_SERIE_NOM));
        connect(mapper5, SIGNAL(mapped(QString)), this, SLOT(comboBox_etat_changed(QString)));
        connect(etat, SIGNAL(currentTextChanged(QString)), mapper5, SLOT(map()));

        ui->pagePrincipaleTableWidgetDisplay->setRowCount(indice+1);
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(map.value(bdd->FICHE_SERIE_NOM), false));
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(map.value(bdd->SAISON_SAISON), true));
        ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(map.value(bdd->SAISON_EPISODE_COURANT), true));
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 3, url);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 4, addicted);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 5, wiki);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 6, reporter);
        ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 7, widgetEtat);
        // On met la couleur
        ui->pagePrincipaleTableWidgetDisplay->item(indice, 0)->setBackgroundColor(QColor(map.value(bdd->TYPE_SERIE_RED).toInt(), map.value(bdd->TYPE_SERIE_GREEN).toInt(), map.value(bdd->TYPE_SERIE_BLUE).toInt()));
        ui->pagePrincipaleTableWidgetDisplay->item(indice, 1)->setBackgroundColor(QColor(map.value(bdd->TYPE_SERIE_RED).toInt(), map.value(bdd->TYPE_SERIE_GREEN).toInt(), map.value(bdd->TYPE_SERIE_BLUE).toInt()));
        ui->pagePrincipaleTableWidgetDisplay->item(indice, 2)->setBackgroundColor(QColor(map.value(bdd->TYPE_SERIE_RED).toInt(), map.value(bdd->TYPE_SERIE_GREEN).toInt(), map.value(bdd->TYPE_SERIE_BLUE).toInt()));

        if(map.value(bdd->SAISON_EPISODE_COURANT) == map.value(bdd->SAISON_NB_EPISODE)) {
            for(int j = 0; j < 3; j++) {
                ui->pagePrincipaleTableWidgetDisplay->item(indice, j)->setBackgroundColor(QColor(240, 63, 63));
            }
        }

        indice++;

        for(int j = 1; j <= map.value(bdd->SAISON_EPISODE_EN_PLUS).toInt();j++) {
            QSignalMapper* mapper1Double = new QSignalMapper();
            QSignalMapper* mapper2Double = new QSignalMapper();
            QSignalMapper* mapper3Double = new QSignalMapper();
            QSignalMapper* mapper4Double = new QSignalMapper();

            QPushButton* urlDouble = new QPushButton(i_torrent, "");
            QPushButton* wikiDouble = new QPushButton(i_wiki, "");
            QPushButton* reporterDouble = new QPushButton(i_reporter, "");
            QPushButton* addictedDouble = new QPushButton(i_addicted, "");

            urlDouble->setToolTip("Ouvre le lien URL de " + map.value(bdd->FICHE_SERIE_NOM));
            wikiDouble->setToolTip("Ouvre le lien Wikipédia de " + map.value(bdd->FICHE_SERIE_NOM));
            reporterDouble->setToolTip("Ouvre la fênetre de report de " + map.value(bdd->FICHE_SERIE_NOM));
            addictedDouble->setToolTip("Ouvre le lien Addic7ed de " + map.value(bdd->FICHE_SERIE_NOM));

            mapper1Double->setMapping(urlDouble, lien + QString(map.value(bdd->FICHE_SERIE_NOM)).replace(" ", "+") + "+S" + map.value(bdd->SAISON_SAISON) + "E" + QString::number(map.value(bdd->SAISON_EPISODE_COURANT).toInt() + j) + sousTitres + qualite + fin);
            if(map.value(bdd->SAISON_WIKI) != "") {
                mapper2Double->setMapping(wikiDouble, map.value(bdd->SAISON_WIKI));
            } else if (map.value(bdd->FICHE_SERIE_WIKI) != "") {
                mapper2Double->setMapping(wikiDouble, map.value(bdd->FICHE_SERIE_WIKI));
            } else {
                mapper2Double->setMapping(wikiDouble, "");
                wikiDouble->setEnabled(false);
            }
            mapper3Double->setMapping(reporterDouble, map.value(bdd->FICHE_SERIE_NOM));
            if(map.value(bdd->FICHE_SERIE_ADDICTED) != "") {
                mapper4Double->setMapping(addictedDouble, map.value(bdd->FICHE_SERIE_ADDICTED) + "/" + QString::number(map.value(bdd->SAISON_SAISON).toInt()) + "/" + QString::number(map.value(bdd->SAISON_EPISODE_COURANT).toInt() + j) + "/8");
            } else {
                addictedDouble->setEnabled(false);
            }

            connect(mapper1Double, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
            connect(urlDouble, SIGNAL(clicked()), mapper1Double, SLOT(map()));

            connect(mapper2Double, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
            connect(wikiDouble, SIGNAL(clicked(bool)), mapper2Double, SLOT(map()));

            connect(mapper3Double, SIGNAL(mapped(QString)), this, SLOT(bouton_reporter_clicked(QString)));
            connect(reporterDouble, SIGNAL(clicked(bool)), mapper3Double, SLOT(map()));

            connect(mapper4Double, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
            connect(addictedDouble, SIGNAL(clicked(bool)), mapper4Double, SLOT(map()));

            ui->pagePrincipaleTableWidgetDisplay->setRowCount(indice+1);
            ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(map.value(bdd->FICHE_SERIE_NOM), false));
            ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(map.value(bdd->SAISON_SAISON), true));
            ui->pagePrincipaleTableWidgetDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(methodeDiverses.formalismeEntier(map.value(bdd->SAISON_EPISODE_COURANT).toInt() + j), true));
            ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 3, urlDouble);
            ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 4, addictedDouble);
            ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 5, wikiDouble);
            ui->pagePrincipaleTableWidgetDisplay->setCellWidget(indice, 6, reporterDouble);
            // On met la couleur
            ui->pagePrincipaleTableWidgetDisplay->item(indice, 0)->setBackgroundColor(QColor(map.value(bdd->TYPE_SERIE_RED).toInt(), map.value(bdd->TYPE_SERIE_GREEN).toInt(), map.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pagePrincipaleTableWidgetDisplay->item(indice, 1)->setBackgroundColor(QColor(map.value(bdd->TYPE_SERIE_RED).toInt(), map.value(bdd->TYPE_SERIE_GREEN).toInt(), map.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pagePrincipaleTableWidgetDisplay->item(indice, 2)->setBackgroundColor(QColor(map.value(bdd->TYPE_SERIE_RED).toInt(), map.value(bdd->TYPE_SERIE_GREEN).toInt(), map.value(bdd->TYPE_SERIE_BLUE).toInt()));

            if(methodeDiverses.formalismeEntier(map.value(bdd->SAISON_EPISODE_COURANT).toInt() + j) == map.value(bdd->SAISON_NB_EPISODE)) {
                for(int j = 0; j < 3; j++) {
                    ui->pagePrincipaleTableWidgetDisplay->item(indice, j)->setBackgroundColor(QColor(240, 63, 63));
                }
            }

            indice++;
        }
    }

    MethodeDiverses::ecrireLog("\tActualisation de l'onglet Aujourd'hui effectué");

    if(listeQuoti.isEmpty()) {
        ui->pagePrincipaleBoutonLienEtAddicted->setEnabled(false);
    } else {
        ui->pagePrincipaleBoutonLienEtAddicted->setEnabled(true);
    }

    for (int i = 0; i < listeQuoti.count(); ++i) {
        if(!listeQuoti.at(i).value(bdd->FICHE_SERIE_WIKI).isEmpty() || !listeQuoti.at(i).value(bdd->SAISON_WIKI).isEmpty()) {
            activationBoutonWikiGlobal = true;
        }
    }
    ui->pagePrincipaleBoutonWiki->setEnabled(activationBoutonWikiGlobal);

    MethodeDiverses::ecrireLog("\tParamètrage des boutons globaux");

    // Refresh de l'onglet hier
    ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(0);

    indice = 0;

    for(int i = 0; i < listeHier.count(); i++) {
        QMap<QString, QString> mapHier = listeHier.value(i);
        QStringList jointures;
        champs.clear();
        conditions.clear();
        ordres.clear();
        champs.append(bdd->FICHE_SERIE_WIKI);
        champs.append(bdd->SAISON_WIKI);
        jointures.append(bdd->LEFT_JOIN + bdd->TABLE_SAISON + bdd->ON + bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->SAISON_ID);
        conditions.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(mapHier.value(bdd->HISTORIQUE_NOM)));

        BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer les lien wikipédia de " + mapHier.value(bdd->HISTORIQUE_NOM);
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::refresh() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QList<QMap<QString, QString> >listeWiki = retour.liste;

        QSignalMapper* mapper1 = new QSignalMapper();
        QSignalMapper* mapper2 = new QSignalMapper();
        QSignalMapper* mapper3 = new QSignalMapper();
        QSignalMapper* mapper4 = new QSignalMapper();

        QPushButton* lienHier = new QPushButton(i_torrent, "");
        QPushButton* wikiHier = new QPushButton(i_wiki, "");
        QPushButton* addicted = new QPushButton(i_addicted, "");
        QCheckBox *vuHier = new QCheckBox();
        QComboBox *etatHier = new QComboBox();

        etatHier->addItem("Non vu");
        etatHier->addItem("Téléchargé");
        etatHier->addItem("Vu");

        lienHier->setToolTip("Ouvre le lien URL de " + mapHier.value(bdd->HISTORIQUE_NOM));
        wikiHier->setToolTip("Ouvre le lien Wikipédia de " + mapHier.value(bdd->HISTORIQUE_NOM));
        addicted->setToolTip("Ouvre le lien Addic7ed de " + mapHier.value(bdd->HISTORIQUE_NOM));
        vuHier->setToolTip("Marque l'épisode S" + mapHier.value(bdd->SAISON_SAISON) + "E" + mapHier.value(bdd->SAISON_EPISODE_COURANT) + " de la série " + mapHier.value(bdd->FICHE_SERIE_NOM) + " comme vu");

        QString lienWiki;
        if(!listeWiki.isEmpty()) {
            if(listeWiki.at(0).value(bdd->SAISON_WIKI) != "") {
                lienWiki = listeWiki.at(0).value(bdd->SAISON_WIKI);
            } else if(listeWiki.at(0).value(bdd->FICHE_SERIE_WIKI) != "") {
                lienWiki = listeWiki.at(0).value(bdd->FICHE_SERIE_WIKI);
            }
        } else {
            lienWiki = "";
            wikiHier->setEnabled(false);
        }

        champs.clear();
        jointures.clear();
        conditions.clear();
        ordres.clear();
        champs.append(bdd->FICHE_SERIE_ADDICTED);
        conditions.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(mapHier.value(bdd->HISTORIQUE_NOM)));

        retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer le lien addicted de " + mapHier.value(bdd->HISTORIQUE_NOM);
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::refresh() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QString lienAddicted = retour.liste.at(0).value(bdd->FICHE_SERIE_ADDICTED);

        mapper1->setMapping(lienHier, lien + QString(mapHier.value(bdd->HISTORIQUE_NOM)).replace(" ", "+") + "+S" + mapHier.value(bdd->HISTORIQUE_SAISON) + "E" + mapHier.value(bdd->HISTORIQUE_EPISODE) + sousTitres + qualite + fin);
        mapper2->setMapping(wikiHier, lienWiki);
        if(lienAddicted != "") {
            mapper3->setMapping(addicted, lienAddicted + "/" + QString::number(mapHier.value(bdd->HISTORIQUE_SAISON).toInt()) + "/" + QString::number(mapHier.value(bdd->HISTORIQUE_EPISODE).toInt()) + "/8");
        } else {
            addicted->setEnabled(false);
        }

        connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(lienHier, SIGNAL(clicked()), mapper1, SLOT(map()));

        connect(mapper2, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(wikiHier, SIGNAL(clicked(bool)), mapper2, SLOT(map()));

        connect(mapper3, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(addicted, SIGNAL(clicked(bool)), mapper3, SLOT(map()));

        QWidget *widgetEtatHier = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget(etatHier);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0,0,0,0);
        widgetEtatHier->setLayout(layout);

        if(mapHier.value(bdd->HISTORIQUE_ETAT) == "NV") {
            etatHier->setCurrentText("Non vu");
        } else if (mapHier.value(bdd->HISTORIQUE_ETAT) == "T") {
            etatHier->setCurrentText("Téléchargé");
        } else {
            etatHier->setCurrentText("Vu");
        }

        mapper4->setMapping(etatHier, mapHier.value(bdd->HISTORIQUE_NOM) + 30 + mapHier.value(bdd->HISTORIQUE_SAISON) + 30 + mapHier.value(bdd->HISTORIQUE_EPISODE));
        connect(mapper4, SIGNAL(mapped(QString)), this, SLOT(comboBox_etat_changed(QString)));
        connect(etatHier, SIGNAL(currentTextChanged(QString)), mapper4, SLOT(map()));

        ui->pagePrincipaleTableWidgetDisplay_2->setRowCount(indice+1);
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 0, methodeDiverses.itemForTableWidget(mapHier.value(bdd->HISTORIQUE_NOM), false));
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 1, methodeDiverses.itemForTableWidget(mapHier.value(bdd->HISTORIQUE_SAISON), true));
        ui->pagePrincipaleTableWidgetDisplay_2->setItem(indice, 2, methodeDiverses.itemForTableWidget(mapHier.value(bdd->HISTORIQUE_EPISODE), true));
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 3, lienHier);
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 4, addicted);
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 5, wikiHier);
        ui->pagePrincipaleTableWidgetDisplay_2->setCellWidget(indice, 6, widgetEtatHier);
        // On met la couleur
        // TODO A mettre lors de la nouvelle architecture de BDD
        /*ui->pagePrincipaleTableWidgetDisplay_2->item(indice, 0)->setBackgroundColor(QColor(mapHier.value(bdd->TYPE_SERIE_RED).toInt(), mapHier.value(bdd->TYPE_SERIE_GREEN).toInt(), mapHier.value(bdd->TYPE_SERIE_BLUE).toInt()));
        ui->pagePrincipaleTableWidgetDisplay_2->item(indice, 1)->setBackgroundColor(QColor(mapHier.value(bdd->TYPE_SERIE_RED).toInt(), mapHier.value(bdd->TYPE_SERIE_GREEN).toInt(), mapHier.value(bdd->TYPE_SERIE_BLUE).toInt()));
        ui->pagePrincipaleTableWidgetDisplay_2->item(indice, 2)->setBackgroundColor(QColor(mapHier.value(bdd->TYPE_SERIE_RED).toInt(), mapHier.value(bdd->TYPE_SERIE_GREEN).toInt(), mapHier.value(bdd->TYPE_SERIE_BLUE).toInt()));
        */
        indice++;
    }

    MethodeDiverses::ecrireLog("\tActualisation de l'onglet Hier effectué");

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

    MethodeDiverses::ecrireLog("\tDimensionnement des colonnes des tableaux effectué");

    if(listeGlobal.isEmpty()) {
        ui->pagePrincipaleBoutonVosSeries->setEnabled(false);
        ui->pagePrincipaleBoutonModifier->setEnabled(false);
        ui->toolBarModifier->setEnabled(false);
    } else {
        ui->pagePrincipaleBoutonVosSeries->setEnabled(true);
        ui->pagePrincipaleBoutonModifier->setEnabled(true);
        ui->toolBarModifier->setEnabled(true);
    }

    MethodeDiverses::ecrireLog("\tAjout des noms de série pour les pages de sélection de modification et de suppression");

    MethodeDiverses::ecrireLog("\tTri des liste de modification et de suppression");

    ficheSerie.clear();
    champs.clear();
    jointures.clear();
    conditions.clear();
    ordres.clear();
    champs.append(bdd->FICHE_SERIE_NOM);
    retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);;
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les nom des séries";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::refresh() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString,QString> > listeFicheSerie = retour.liste;
    for(int i = 0; i < listeFicheSerie.count(); i++) {
        ficheSerie.append(listeFicheSerie.at(i).value(bdd->FICHE_SERIE_NOM));
    }

    MethodeDiverses::ecrireLog("\tAjout des noms des séries du dossier des série dans la liste déroulante de l'ajout de série");

    ui->statusBar->showMessage("Actualisé à " + QTime::currentTime().toString("hh:mm:ss"));
    MethodeDiverses::ecrireLog("FenetrePrincipale::refresh() : Fin de l'actualisation");
}

void FenetrePrincipale::bouton_lien_clicked(QString nom) {
    MethodeDiverses::ecrireLog("FenetrePrincipale::bouton_lien_clicked() : Début de la génération du lien");
    if(QDesktopServices::openUrl(QUrl(nom))) {
        MethodeDiverses::ecrireLog("\tOuverture du lien " + nom);
    } else {
        QMessageBox::warning(this, this->windowTitle(), "Le lien " + nom + " n'a pu être ouvert");
        MethodeDiverses::ecrireLog("\tLe lien " + nom + " n'a pu être ouvert");
    }

    QTime fin = QTime::currentTime().addMSecs(500);
    while (QTime::currentTime() < fin) {}
    MethodeDiverses::ecrireLog("FenetrePrincipale::bouton_lien_clicked() : Fin de la génération du lien");

}

void FenetrePrincipale::bouton_dossier_clicked(QString nom) {
    MethodeDiverses::ecrireLog("FenetrePrincipale::bouton_dossier_clicked() : Début de l'ouverture du dossier");
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + nom))) {
        QMessageBox::warning(this,this->windowTitle(), "Le dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
        MethodeDiverses::ecrireLog("\tLe dossier " + dossierSerie + nom + " n'a pas pu être ouvert");
    } else {
        MethodeDiverses::ecrireLog("\tLe dossier " + dossierSerie + nom + " a été ouvert");
    }
    MethodeDiverses::ecrireLog("FenetrePrincipale::bouton_dossier_clicked() : Fin de l'ouverture du dossier");
}

void FenetrePrincipale::bouton_reporter_clicked(QString nom) {
    MethodeDiverses::ecrireLog("FenetrePrincipale::bouton_reporter_clicked() : Ouverture de la page de report de série");
    on_pageReporterSpinBox_valueChanged(ui->pageReporterSpinBox->value());
    ui->stackedWidget->setCurrentWidget(ui->pageReporter);
    ui->pageReporterLabel->setText("De combien de semaines voulez vous reporter " + nom + " ?");
    ui->pageReporterLabelNomSerie->setText(nom);
    ui->pageReporterSpinBox->setValue(1);
}

void FenetrePrincipale::comboBox_etat_changed(QString nom) {
    int ligne = 0;
    QString nouvelEtat, saison, episode;
    QStringList champs, conditions;
    BaseDeDonnees::Retour retour;
    if(ui->stackedWidget->currentWidget() == ui->pagePrincipale && ui->pagePrincipaleTabWidget->currentWidget() == ui->pagePrincipaleTabWidgetTabAuj) {
        for(int i = 0; i < ui->pagePrincipaleTableWidgetDisplay->rowCount(); i++) {
            if(nom == ui->pagePrincipaleTableWidgetDisplay->item(i, 0)->text()) {
                ligne = i;
                break;
            }
        }

        nouvelEtat = ((QComboBox *)ui->pagePrincipaleTableWidgetDisplay->cellWidget(ligne, 7)->layout()->itemAt(0)->widget())->currentText();
    } else if (ui->stackedWidget->currentWidget() == ui->pagePrincipale && ui->pagePrincipaleTabWidget->currentWidget() == ui->pagePrincipaleTabWidgetTabHier){
        QStringList liste = nom.split(30);
        nom = liste.at(0);
        saison = liste.at(1);
        episode = liste.at(2);
        for(int i = 0; i < ui->pagePrincipaleTableWidgetDisplay_2->rowCount(); i++) {
            if(nom == ui->pagePrincipaleTableWidgetDisplay_2->item(i, 0)->text() && ui->pagePrincipaleTableWidgetDisplay_2->item(i, 1)->text() == saison && ui->pagePrincipaleTableWidgetDisplay_2->item(i, 2)->text() == episode) {
                ligne = i;
                break;
            }
        }

        nouvelEtat = ((QComboBox *)ui->pagePrincipaleTableWidgetDisplay_2->cellWidget(ligne, 6)->layout()->itemAt(0)->widget())->currentText();
    } else if (ui->stackedWidget->currentWidget() == ui->pageHistorique) {
        QStringList liste = nom.split(30);
        nom = liste.at(0);
        saison = liste.at(1);
        episode = liste.at(2);
        for(int i = 0; i < ui->pageHistoriqueTableWidget->rowCount(); i++) {
            if(nom == ui->pageHistoriqueTableWidget->item(i, 0)->text() && ui->pageHistoriqueTableWidget->item(i, 1)->text() == saison && ui->pageHistoriqueTableWidget->item(i, 2)->text() == episode) {
                ligne = i;
                break;
            }
        }

        nouvelEtat = ((QComboBox *)ui->pageHistoriqueTableWidget->cellWidget(ligne, 5)->layout()->itemAt(0)->widget())->currentText();
    } else if (ui->stackedWidget->currentWidget() == ui->pageAjoutModification) {
        QStringList liste = nom.split(30);
        nom = liste.at(0);
        saison = liste.at(1);
        episode = liste.at(2);
        for(int i = 0; i < ui->pageAjoutModifTableWidgetListeSaison->rowCount(); i++) {
            if(nom == ui->pageAjoutModifTableWidgetListeSaison->item(i, 0)->text() && ui->pageAjoutModifTableWidgetListeSaison->item(i, 1)->text() == episode) {
                ligne = i;
                break;
            }
        }

        nouvelEtat = ((QComboBox *)ui->pageAjoutModifTableWidgetListeSaison->cellWidget(ligne, 4)->layout()->itemAt(0)->widget())->currentText();
    }

    if(nouvelEtat == "Non vu") {
        nouvelEtat = "NV";
    } else if (nouvelEtat == "Téléchargé") {
        nouvelEtat = "T";
    } else {
        nouvelEtat = "V";
    }


    if(ui->stackedWidget->currentWidget() == ui->pagePrincipale && ui->pagePrincipaleTabWidget->currentWidget() == ui->pagePrincipaleTabWidgetTabAuj) {
        champs.append(bdd->SAISON_ETAT + bdd->EGALE + bdd->entreQuotes(nouvelEtat));
        conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreParentheses(bdd->SELECT + bdd->FICHE_SERIE_ID + bdd->FROM + bdd->TABLE_FICHE_SERIE + bdd->WHERE + bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(nom)));

        retour = bdd->requeteUpdate(champs, bdd->TABLE_SAISON, conditions);
        if(!retour.reussi) {
            QString messageLog = "Impossible de mettre à jour l'état de l'episode courant de " + nom;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::comboBox_etat_changed() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
    } else {
        champs.append(bdd->HISTORIQUE_ETAT + bdd->EGALE + bdd->entreQuotes(nouvelEtat));
        conditions.append(bdd->HISTORIQUE_NOM + bdd->EGALE + bdd->entreQuotes(nom));
        conditions.append(bdd->HISTORIQUE_SAISON + bdd->EGALE + bdd->entreQuotes(saison));
        conditions.append(bdd->HISTORIQUE_EPISODE + bdd->EGALE + bdd->entreQuotes(episode));

        retour = bdd->requeteUpdate(champs, bdd->TABLE_HISTORIQUE, conditions);
        if(!retour.reussi) {
            QString messageLog = "Impossible de mettre à jour l'état de l'episode " + nom + " S" + saison + "E" + episode;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::comboBox_etat_changed() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
    }
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

    if(QDir(MethodeDiverses::getConfig("Configuration/Telechargement")).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() > 0) {
        ui->pagePrincipaleBoutonDeplacerFichier->setEnabled(true);
        ui->toolBarDeplcerFichiers->setEnabled(true);
    } else {
        ui->pagePrincipaleBoutonDeplacerFichier->setEnabled(false);
        ui->toolBarDeplcerFichiers->setEnabled(false);
    }
}

void FenetrePrincipale::chargementConfiguration() {
    MethodeDiverses::ecrireLog("FenetrePrincipale::chargementConfiguration() : Début du chargement du fichier de configuration");
    QString listeSerie = MethodeDiverses::getConfig("Configuration/ListeSerie");

    if(listeSerie == "Toutes") {
        ui->pageVosSeriesComboBox->setCurrentIndex(0);
    } else if (listeSerie == "Série en cours") {
        ui->pageVosSeriesComboBox->setCurrentIndex(1);
    } else if (listeSerie == "Série à venir") {
        ui->pageVosSeriesComboBox->setCurrentIndex(2);
    }

    this->dossierSerie = MethodeDiverses::getConfig("Configuration/Chemin");
    if(MethodeDiverses::getConfig("Dimension/Fullscreen").toInt()) {
        this->setWindowState(this->windowState()|Qt::WindowMaximized);
    } else {
        this->setGeometry(MethodeDiverses::getConfig("Dimension/X", 724), MethodeDiverses::getConfig("Dimension/Y", 303), MethodeDiverses::getConfig("Dimension/W", 472), MethodeDiverses::getConfig("Dimension/H", 434));
    }

    lien = ui->pageConfigurationLineEditSite->text() + MethodeDiverses::getConfig("Configuration/Extension") + "/engine/search?name=";

    MethodeDiverses::ecrireLog("FenetrePrincipale::chargementConfiguration() : Fin du chargement du fichier de configuration");
}

void FenetrePrincipale::majEpisode() {
    MethodeDiverses::ecrireLog("FenetrePrincipale::majEpisode() : Début de mise à jour des episodes de la veille");

    QDate dateDerniereOuverture = bdd->derniereOuvertureBDD();
    QDate dateDerniereOuverturePlus1 = dateDerniereOuverture.addDays(1);

    while(dateDerniereOuverture <= QDate::currentDate().addDays(-1)) {
        QStringList champs, jointures, conditions, ordres, valeurs;
        champs.append(bdd->FICHE_SERIE_ID);
        champs.append(bdd->FICHE_SERIE_NOM);
        champs.append(bdd->FICHE_SERIE_WIKI);
        champs.append(bdd->SAISON_SAISON);
        champs.append(bdd->SAISON_NB_EPISODE);
        champs.append(bdd->SAISON_EPISODE_COURANT);
        champs.append(bdd->SAISON_DATE_SORTIE);
        champs.append(bdd->SAISON_DATE_MODIF);
        champs.append(bdd->SAISON_WIKI);
        champs.append(bdd->SAISON_EPISODE_EN_PLUS);
        champs.append(bdd->SAISON_ETAT);
        conditions.append(bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->SAISON_ID);
        conditions.append(bdd->SAISON_DATE_MODIF + bdd->EGALE + bdd->entreQuotes(methodeDiverses.dateToString(dateDerniereOuverture.addDays(-7))));
        BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE + ", " + bdd->TABLE_SAISON, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer les séries à mettre à jour";
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::majEpisode() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QList<QMap<QString,QString> > liste = retour.liste;

        if(!liste.isEmpty()) {
            for (int i = 0; i < liste.count(); ++i) {
                QMap<QString, QString> list = liste.value(i);

                MethodeDiverses::ecrireLog("\tMise à jour de " + list.value(bdd->FICHE_SERIE_NOM));
                champs.clear();
                conditions.clear();
                if(!list.value(bdd->SAISON_EPISODE_EN_PLUS).toInt()) {
                    champs.append(bdd->SAISON_EPISODE_COURANT + bdd->EGALE + bdd->entreQuotes(methodeDiverses.formalismeEntier(list.value(bdd->SAISON_EPISODE_COURANT).toInt() + 1)));
                } else {
                    champs.append(bdd->SAISON_EPISODE_COURANT + bdd->EGALE + bdd->entreQuotes(methodeDiverses.formalismeEntier(list.value(bdd->SAISON_EPISODE_COURANT).toInt() + list.value(bdd->SAISON_EPISODE_EN_PLUS).toInt() + 1)));
                    champs.append(bdd->SAISON_EPISODE_EN_PLUS + bdd->EGALE + "0");
                }
                champs.append(bdd->SAISON_ETAT + bdd->EGALE + bdd->entreQuotes("NV"));
                champs.append(bdd->SAISON_DATE_MODIF + bdd->EGALE + bdd->entreQuotes(dateDerniereOuverture.toString("yyyy-MM-dd")));
                conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreQuotes(list.value(bdd->FICHE_SERIE_ID)));
                retour = bdd->requeteUpdate(champs, bdd->TABLE_SAISON, conditions);
                if(!retour.reussi) {
                    QString messageLog = "Impossible de mettre à jour l'épisode " + list.value(bdd->FICHE_SERIE_NOM) + " S" + list.value(bdd->SAISON_SAISON) + "E" + list.value(bdd->SAISON_EPISODE_COURANT);
                    QMessageBox::critical(this, this->windowTitle(), messageLog);
                    MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::majEpisode() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                    throw new QException();
                }

                champs.clear();
                valeurs.clear();
                champs.append(bdd->HISTORIQUE_NOM);
                champs.append(bdd->HISTORIQUE_SAISON);
                champs.append(bdd->HISTORIQUE_EPISODE);
                champs.append(bdd->HISTORIQUE_DATE_AJOUT);
                champs.append(bdd->HISTORIQUE_ETAT);
                valeurs.append(bdd->entreQuotes(list.value(bdd->FICHE_SERIE_NOM)));
                valeurs.append(bdd->entreQuotes(list.value(bdd->SAISON_SAISON)));
                valeurs.append(bdd->entreQuotes(list.value(bdd->SAISON_EPISODE_COURANT)));
                valeurs.append(bdd->entreQuotes(dateDerniereOuverture.toString("yyyy-MM-dd")));
                valeurs.append(bdd->entreQuotes(list.value(bdd->SAISON_ETAT)));
                retour = bdd->requeteInsert(champs, valeurs, bdd->TABLE_HISTORIQUE);
                if(!retour.reussi) {
                    QString messageLog = "Impossible d'historiser l'épisode " + list.value(bdd->FICHE_SERIE_NOM) + " S" + list.value(bdd->SAISON_SAISON) + "E" + list.value(bdd->SAISON_EPISODE_COURANT);
                    QMessageBox::critical(this, this->windowTitle(), messageLog);
                    MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::majEpisode() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                    throw new QException();
                }
                for(int j = 1; j <= list.value(bdd->SAISON_EPISODE_EN_PLUS).toInt(); j++) {
                    champs.clear();
                    valeurs.clear();
                    champs.append(bdd->HISTORIQUE_NOM);
                    champs.append(bdd->HISTORIQUE_SAISON);
                    champs.append(bdd->HISTORIQUE_EPISODE);
                    champs.append(bdd->HISTORIQUE_DATE_AJOUT);
                    valeurs.append(bdd->entreQuotes(list.value(bdd->FICHE_SERIE_NOM)));
                    valeurs.append(bdd->entreQuotes(list.value(bdd->SAISON_SAISON)));
                    valeurs.append(bdd->entreQuotes(methodeDiverses.formalismeEntier(list.value(bdd->SAISON_EPISODE_COURANT).toInt() + j)));
                    valeurs.append(bdd->entreQuotes(dateDerniereOuverture.toString("yyyy-MM-dd")));
                    bdd->requeteInsert(champs, valeurs, bdd->TABLE_HISTORIQUE);
                    if(!retour.reussi) {
                        QString messageLog = "Impossible d'historiser l'épisode " + list.value(bdd->FICHE_SERIE_NOM) + " S" + list.value(bdd->SAISON_SAISON) + "E" + methodeDiverses.formalismeEntier(list.value(bdd->SAISON_EPISODE_COURANT).toInt() + j);
                        QMessageBox::critical(this, this->windowTitle(), messageLog);
                        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::majEpisode() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                        throw new QException();
                    }
                }
            }
            bdd->majDerniereOuvertureBDD();
        }
        // Verifie si une série est terminé
        verificationSerieTerminer();

        dateDerniereOuverture = dateDerniereOuverture.addDays(1);
        dateDerniereOuverturePlus1 = dateDerniereOuverturePlus1.addDays(1);
    }
    MethodeDiverses::ecrireLog("FenetrePrincipale::majEpisode() : Fin de mise à jour des episodes de la veille");
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
    MethodeDiverses::ecrireLog("FenetrePrincipale::verificationSerieTerminer() : Début de vérification des séries terminées");
    QStringList champs, jointures, conditions, ordres;
    champs.append(bdd->FICHE_SERIE_ID);
    champs.append(bdd->FICHE_SERIE_NOM);
    champs.append(bdd->SAISON_SAISON);
    champs.append(bdd->SAISON_NB_EPISODE);
    champs.append(bdd->SAISON_EPISODE_COURANT);
    jointures.append(bdd->INNER_JOIN + bdd->TABLE_SAISON + bdd->ON + bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->SAISON_ID);
    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
    QList<QMap<QString,QString> > liste = retour.liste;
    for (int i = 0; i < liste.count(); ++i) {
        QMap<QString,QString> list = liste.value(i);

        if(list.value(bdd->SAISON_NB_EPISODE).toInt() < list.value(bdd->SAISON_EPISODE_COURANT).toInt()){
            MethodeDiverses::ecrireLog("\tSuppression de " + list.value(bdd->FICHE_SERIE_NOM));
            conditions.clear();
            conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreQuotes(list.value(bdd->FICHE_SERIE_ID)));
            retour = bdd->requeteDelete(bdd->TABLE_SAISON, conditions);
            if(!retour.reussi) {
                QString messageLog = "Impossible de supprimer la saison courante de la série " + list.value(bdd->FICHE_SERIE_NOM);
                QMessageBox::critical(this, this->windowTitle(), messageLog);
                MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::verificationSerieTerminer() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                throw new QException();
            }
        }
    }
    MethodeDiverses::ecrireLog("FenetrePrincipale::verificationSerieTerminer() : Fin de vérification des séries terminées");
}

BaseDeDonnees *FenetrePrincipale::getBdd() {
    return bdd;
}

void FenetrePrincipale::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
    if(MethodeDiverses::getConfig("Dimension/W", 472) != this->geometry().width())
        MethodeDiverses::setConfig("Dimension/W", QString::number(this->geometry().width()));
    if(MethodeDiverses::getConfig("Dimension/H", 434) != this->geometry().height())
        MethodeDiverses::setConfig("Dimension/H", QString::number(this->geometry().height()));
    if(MethodeDiverses::getConfig("Dimension/X",724) != this->geometry().x())
        MethodeDiverses::setConfig("Dimension/X", QString::number(this->geometry().x()));
    if(MethodeDiverses::getConfig("Dimension/Y", 303) != this->geometry().y())
        MethodeDiverses::setConfig("Dimension/Y",QString::number(this->geometry().y()));
    if(MethodeDiverses::getConfig("Dimension/Fullscreen", 0) != this->isMaximized())
        MethodeDiverses::setConfig("Dimension/Fullscreen", QString::number(this->isMaximized()));
}

void FenetrePrincipale::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
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

void FenetrePrincipale::fermetureFenetre() {
    refresh();
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
        QStringList champs, jointures, conditions, ordres;
        champs.append(bdd->DISTINCT + bdd->HISTORIQUE_NOM);
        BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer les séries";
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_menuFichierHistoriqueRechercher_triggered() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QList<QMap<QString, QString> > liste = retour.liste;
        for(int i = 0; i < liste.count(); i++) {
            ui->pageHistoriqueRechercherComboBoxNomSerie->addItem(liste.at(i).value(bdd->HISTORIQUE_NOM));
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
        QStringList champs, jointures, conditions, ordres;
        champs.append(bdd->FICHE_SERIE_NOM);
        BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer les séries";
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_menuFichierHistoriqueAjouter_triggered() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QList<QMap<QString,QString> > liste = retour.liste;
        for(int i = 0; i < liste.count(); i++) {
            ui->pageHistoriqueAjouterComboBoxSerie->addItem(liste.at(i).value(bdd->FICHE_SERIE_NOM));
        }
    }
}

void FenetrePrincipale::on_menuFichierHistoriqueModifier_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageHistoriqueModifier) {
        ui->stackedWidget->setCurrentWidget(ui->pageHistoriqueModifier);
        ui->pageHistoriqueModifierComboBoxRechercheNom->setCurrentIndex(0);
        QStringList champs, jointures, conditions, ordres;
        champs.append(bdd->DISTINCT + bdd->HISTORIQUE_NOM);
        BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer les séries";
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_menuFichierHistoriqueModifier_triggered() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QList<QMap<QString, QString> > liste = retour.liste;
        ui->pageHistoriqueModifierComboBoxRechercheNom->clear();
        for(int i = 0; i < liste.count(); i++) {
            ui->pageHistoriqueModifierComboBoxRechercheNom->addItem(liste.at(i).value(bdd->HISTORIQUE_NOM));
        }
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setDate(QDate());
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setEnabled(false);

        on_pageHistoriqueModifierComboBoxRechercheNom_currentTextChanged(ui->pageHistoriqueModifierComboBoxRechercheNom->currentText());
        on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged(ui->pageHistoriqueModifierSpinBoxRechercheSaison->value());
    }
}

void FenetrePrincipale::on_menuFichierHistoriqueConsulter_triggered() {
    if(ui->stackedWidget->currentWidget() != ui->pageHistorique) {
        ui->stackedWidget->setCurrentWidget(ui->pageHistorique);

        on_pageHistoriqueComboBoxEtat_currentTextChanged(ui->pageHistoriqueComboBoxEtat->currentText());
    }
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
    ui->pageAjoutModifCheckBoxTermine->setVisible(false);
    ui->pageAjoutModifLabelTermine->setVisible(false);
    // Ajout des séries dans le comboBox
    QStringList champs, jointures, conditions, ordres;
    champs.append(bdd->FICHE_SERIE_NOM);
    conditions.append(bdd->FICHE_SERIE_NOM + bdd->NOT + bdd->IN + bdd->entreParentheses(bdd->SELECT + bdd->FICHE_SERIE_NOM + bdd->FROM + bdd->TABLE_FICHE_SERIE + bdd->INNER_JOIN + bdd->TABLE_SAISON + bdd->ON + bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->SAISON_ID));
    conditions.append(bdd->FICHE_SERIE_TERMINE + bdd->EGALE + "0");
    ordres.append(bdd->FICHE_SERIE_NOM);
    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les séries";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_menuFichierAjouter_triggered() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString, QString> > serieSansSaison = retour.liste;
    if(!serieSansSaison.isEmpty())
        ui->pageAjoutModifComboFicheSerieNom->addItem("");
    for(int i = 0; i < serieSansSaison.count(); i++) {
        ui->pageAjoutModifComboFicheSerieNom->addItem(serieSansSaison.at(i).value(bdd->FICHE_SERIE_NOM));
    }

    ui->pageAjoutModifComboBoxTypeSerie->clear();
    champs.clear();
    jointures.clear();
    conditions.clear();
    ordres.clear();

    champs.append(bdd->TYPE_SERIE_NOM);

    retour = bdd->requeteSelect(champs, bdd->TABLE_TYPE_SERIE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les types de séries";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_menuFichierAjouter_triggered() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }

    for(int i = 0; i < retour.liste.count(); i++) {
        ui->pageAjoutModifComboBoxTypeSerie->addItem(retour.liste.at(i).value(bdd->TYPE_SERIE_NOM));
    }

    // Mise a zéro des champs
    ui->pageAjoutModifComboFicheSerieNom->setCurrentIndex(0);
    ui->pageAjoutModifLineEditFicheSerieWiki->setText("");
    ui->pageAjoutModifLineEditAddicted->setText("");
    ui->pageAjoutModifCheckBoxCreerSaison->setChecked(true);
    ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
    ui->pageAjoutModifLabelCheminPhoto->setText("");
    ui->pageAjoutModifLineEditDepuisURL->setText("");
    ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
    ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(1);
    ui->pageAjoutModifCheckBoxCreationDossierAuto->setChecked(true);
    ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
    ui->pageAjoutModifLineEditSaisonWiki->setText("");
    ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
    ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());
    ui->pageAjoutModifLineEditCheminPhoto->setText("");

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
    ui->pageAjoutModifLabelDoubleEpisode->setVisible(false);
    ui->pageAjoutModifCheckBoxDoubleEpisode->setVisible(false);
    ui->pageAjoutModifSpinBoxEpisodeEnPlus->setVisible(false);
}

void FenetrePrincipale::on_menuFichierModifier_triggered() {
    ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
    ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
    ui->pageAjoutModifBoutonValider->setText("Modifier");
    ui->pageAjoutModifComboFicheSerieNom->clear();
    ui->pageAjoutModifComboFicheSerieNom->setEditable(false);
    ui->pageAjoutModifCheckBoxTermine->setVisible(true);
    ui->pageAjoutModifLabelTermine->setVisible(true);
    QStringList champs, jointures, conditions, ordres;
    champs.append(bdd->FICHE_SERIE_NOM);
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

    ui->pageAjoutModifComboBoxTypeSerie->clear();
    champs.clear();
    jointures.clear();
    conditions.clear();
    ordres.clear();

    champs.append(bdd->TYPE_SERIE_NOM);

    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_TYPE_SERIE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les types de séries";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_menuFichierAjouter_triggered() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }

    for(int i = 0; i < retour.liste.count(); i++) {
        ui->pageAjoutModifComboBoxTypeSerie->addItem(retour.liste.at(i).value(bdd->TYPE_SERIE_NOM));
    }

    // Mise a zéro des champs
    ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
    ui->pageAjoutModifLabelCheminPhoto->setText("");
    ui->pageAjoutModifComboFicheSerieNom->setCurrentIndex(0);
    ui->pageAjoutModifLineEditFicheSerieWiki->setText("");
    ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
    ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(1);
    ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
    ui->pageAjoutModifLineEditSaisonWiki->setText("");
    ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
    ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());
    ui->pageAjoutModifLineEditCheminPhoto->setText("");

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
    ui->pageAjoutModifLabelDoubleEpisode->setVisible(true);
    ui->pageAjoutModifCheckBoxDoubleEpisode->setVisible(true);
    ui->pageAjoutModifSpinBoxEpisodeEnPlus->setVisible(true);
}

void FenetrePrincipale::on_menuFichierQuitter_triggered() {
    MethodeDiverses::ecrireLog("Fermeture de l'application");
    closeEvent(new QCloseEvent());
    QCoreApplication::quit();
}

void FenetrePrincipale::on_menuOptionsActualiser_triggered() {
    refresh();
}

void FenetrePrincipale::on_menuOptionsParam_tres_triggered() {
    ui->stackedWidget->setCurrentWidget(ui->pageConfig);
    ui->pageConfigurationLabelConsigne->setVisible(false);
    ui->pageConfigurationSpinBoxLogEfface->setValue(MethodeDiverses::getConfig("Configuration/PurgeLog").toInt());
    ui->pageConfigurationLineDossierTelechargement->setText(MethodeDiverses::getConfig("Configuration/Telechargement"));
    ui->pageConfigurationLineDossierTelechargement->setCursorPosition(0);
    ui->pageConfigurationLineDossierSerie->setText(MethodeDiverses::getConfig("Configuration/Chemin"));
    switch (MethodeDiverses::getConfig("Configuration/Qualite").toInt()) {
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
    switch (MethodeDiverses::getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        ui->pageConfigurationRadioButtonSousTitresToutes->setChecked(true);
        break;
    case 1:
        ui->pageConfigurationRadioButtonSousTitresVOSTFR->setChecked(true);
        break;
    }
    ui->pageConfigurationLineEditExtension->setText(MethodeDiverses::getConfig("Configuration/Extension"));
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
        MethodeDiverses::setConfig("Configuration/Chemin", ui->pageConfigurationLineDossierSerie->text());
        MethodeDiverses::setConfig("Configuration/Extension", ui->pageConfigurationLineEditExtension->text());
        lien = ui->pageConfigurationLineEditSite->text() + ui->pageConfigurationLineEditExtension->text() + "/engine/search?name=";
        MethodeDiverses::setConfig("Configuration/ListeSerie", "Toutes");
        MethodeDiverses::setConfig("Configuration/PurgeLog", QString::number(ui->pageConfigurationSpinBoxLogEfface->value()));
        if(ui->pageConfigurationRadioButtonQualiteToutes->isChecked()) {
            MethodeDiverses::setConfig("Configuration/Qualite", "0");
        } else if(ui->pageConfigurationRadioButtonQualite720->isChecked()) {
            MethodeDiverses::setConfig("Configuration/Qualite", "1");
        } else if(ui->pageConfigurationRadioButtonQualite1080->isChecked()) {
            MethodeDiverses::setConfig("Configuration/Qualite", "2");
        }
        if(ui->pageConfigurationRadioButtonSousTitresToutes->isChecked()) {
            MethodeDiverses::setConfig("Configuration/SousTitres", "0");
        } else {
            MethodeDiverses::setConfig("Configuration/SousTitres", "1");
        }
        MethodeDiverses::setConfig("Configuration/Telechargement", ui->pageConfigurationLineDossierTelechargement->text());
        MethodeDiverses::setConfig("Dimension/Fullscreen", QString::number(this->isMaximized()));
        MethodeDiverses::setConfig("Dimension/H", QString::number(this->geometry().height()));
        MethodeDiverses::setConfig("Dimension/W", QString::number(this->geometry().width()));
        MethodeDiverses::setConfig("Dimension/X", QString::number(this->geometry().x()));
        MethodeDiverses::setConfig("Dimension/Y", QString::number(this->geometry().y()));
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
        int ligne = ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow();
        QAction *modifier = menuContextuel.addAction(QIcon(), "Modifier la fiche série");
        menuContextuel.addSeparator();
        QAction *copier = menuContextuel.addAction(QIcon(), "Copier le nom de la série");
        QAction *copierComplet = menuContextuel.addAction(QIcon(), "Copier le nom, la saison et l'épisode de la série");
        QAction *dossier = NULL;
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
                MethodeDiverses::ecrireLog("Ouverture du dossier de " + ui->pagePrincipaleTableWidgetDisplay->item(ui->pagePrincipaleTableWidgetDisplay->selectedRanges().at(0).topRow(), 0)->text());
            } else {
                QMessageBox::warning(this, this->windowTitle(), "Le dossier n'a pas pu être ouvert");
                MethodeDiverses::ecrireLog("Le dossier n'a pas pu être ouvert");
            }
        } else if(a == modifier) {
            on_menuFichierModifier_triggered();
            ui->pageAjoutModifComboFicheSerieNom->setCurrentText(ui->pagePrincipaleTableWidgetDisplay->item(ligne, 0)->text());
            ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
        } else {}
    }
}

void FenetrePrincipale::on_pagePrincipaleTableWidgetDisplay_2_customContextMenuRequested(const QPoint &pos) {
    if(ui->pagePrincipaleTableWidgetDisplay_2->rowCount() > 0 && !ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().isEmpty()) {
        QMenu menuContextuel(this);
        int ligne = ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow();
        QAction *modifier = menuContextuel.addAction(QIcon(), "Modifier la fiche série");
        menuContextuel.addSeparator();
        QAction *copier = menuContextuel.addAction(QIcon(), "Copier le nom de la série");
        QAction *copierComplet = menuContextuel.addAction(QIcon(), "Copier le nom, la saison et l'épisode de la série");
        QAction *dossier = NULL;
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
                MethodeDiverses::ecrireLog("Ouverture du dossier de " + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text());
            } else {
                QMessageBox::warning(this, this->windowTitle(), "Le dossier " + this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 1)->text() + " n'a pas pu être ouvert");
                MethodeDiverses::ecrireLog("Le dossier " + this->dossierSerie + "/" + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 0)->text() + "/Saison " + ui->pagePrincipaleTableWidgetDisplay_2->item(ui->pagePrincipaleTableWidgetDisplay_2->selectedRanges().at(0).topRow(), 1)->text() + " n'a pas pu être ouvert");
            }
        } else if(a == modifier) {
            on_menuFichierModifier_triggered();
            ui->pageAjoutModifComboFicheSerieNom->setCurrentText(ui->pagePrincipaleTableWidgetDisplay_2->item(ligne, 0)->text());
            ui->pageAjoutModifLabelRetour->setText("pagePrincipale");
        } else {}
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

void FenetrePrincipale::on_pagePrincipaleBoutonLienEtAddicted_clicked() {
    if(this->listeQuoti.isEmpty()) {
        QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de série à regarder !");
    } else {
        QString qualite;
        QString sousTitres;
        QString fin = "&do=search&order=desc&sort=publish_date";

        switch (MethodeDiverses::getConfig("Configuration/Qualite").toInt()) {
        case 0: qualite = ""; break;
        case 1: qualite = "+720p"; break;
        case 2: qualite = "+1080p"; break;
        }

        switch (MethodeDiverses::getConfig("Configuration/SousTitres").toInt()) {
        case 0: sousTitres = ""; break;
        case 1: sousTitres = "+VOSTFR"; break;
        }

        for (int i = 0; i < listeQuoti.count(); ++i) {
            QMap<QString,QString> list = listeQuoti.value(i);
            if(methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_SORTIE)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_MODIF)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_MODIF)) != QDate::currentDate())
                bouton_lien_clicked(lien + QString(list.value(bdd->FICHE_SERIE_NOM)).replace(" ","+") + "+S" + list.value(bdd->SAISON_SAISON) + "E" + list.value(bdd->SAISON_EPISODE_COURANT) + sousTitres + qualite + fin);
            if(list.value(bdd->FICHE_SERIE_ADDICTED) != "")
                bouton_lien_clicked(list.value(bdd->FICHE_SERIE_ADDICTED) + "/" + QString::number(list.value(bdd->SAISON_SAISON).toInt()) + "/" + QString::number(list.value(bdd->SAISON_EPISODE_COURANT).toInt()) + "/8");
            for(int j = 1; j <= list.value(bdd->SAISON_EPISODE_EN_PLUS).toInt(); j++) {
                bouton_lien_clicked(lien + QString(list.value(bdd->FICHE_SERIE_NOM)).replace(" ","+") + "+S" + list.value(bdd->SAISON_SAISON) + "E" + methodeDiverses.formalismeEntier(list.value(bdd->SAISON_EPISODE_COURANT).toInt() + j) + sousTitres + qualite + fin);
                if(list.value(bdd->FICHE_SERIE_ADDICTED) != "")
                    bouton_lien_clicked(list.value(bdd->FICHE_SERIE_ADDICTED) + "/" + QString::number(list.value(bdd->SAISON_SAISON).toInt()) + "/" + QString::number(list.value(bdd->SAISON_EPISODE_COURANT).toInt() + j) + "/8");
            }
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() {
    MethodeDiverses::ecrireLog("FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() : Début de l'ouverture du dossier de téléchargement");
    if(QDesktopServices::openUrl(QUrl::fromLocalFile(MethodeDiverses::getConfig("Configuration/Telechargement")))) {
        MethodeDiverses::ecrireLog("\tOuverture du dossier de téléchargement");
    } else {
        QMessageBox::warning(this, this->windowTitle(), "Le dossier de téléchargement n'a pas pu être ouvert");
        MethodeDiverses::ecrireLog("\tLe dossier de téléchargement n'a pas pu être ouvert");
    }
    MethodeDiverses::ecrireLog("FenetrePrincipale::on_pagePrincipaleBoutonDossierSerie_clicked() : Début de l'ouverture du dossier de téléchargement");
}

void FenetrePrincipale::on_pagePrincipaleBoutonWiki_clicked() {
    if(this->listeQuoti.isEmpty()){
        QMessageBox::information(this, this->windowTitle(), "Il n'y a pas de série à regarder !");
    } else {
        for (int i = 0; i < listeQuoti.count(); ++i) {
            QMap<QString,QString> list = listeQuoti.value(i);
            if(methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_SORTIE)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_MODIF)) <= QDate::currentDate() && methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_MODIF)) != QDate::currentDate()) {
                if(!list.value(bdd->SAISON_WIKI).isEmpty()) {
                    QDesktopServices::openUrl(QUrl(list.value(bdd->SAISON_WIKI)));
                } else if (!list.value(bdd->FICHE_SERIE_WIKI).isEmpty()) {
                    QDesktopServices::openUrl(QUrl(list.value(bdd->FICHE_SERIE_WIKI)));
                }
            }
        }
    }
}

void FenetrePrincipale::on_pagePrincipaleBoutonDeplacerFichier_clicked() {
    Dialog *dial = new Dialog(this);
    dial->open();
    connect(dial, SIGNAL(finished(int)), this, SLOT(fermetureFenetre()));
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

void FenetrePrincipale::on_pageVosSeriesDisplay_customContextMenuRequested(const QPoint &pos) {
    if(ui->pageVosSeriesDisplay->rowCount() > 0 && !ui->pageVosSeriesDisplay->selectedRanges().isEmpty()) {
        QMenu menuContextuel(this);
        int ligne = ui->pageVosSeriesDisplay->selectedRanges().at(0).topRow();
        QString serie = ui->pageVosSeriesDisplay->item(ligne, 0)->text();
        QAction *ajouter = menuContextuel.addAction(QIcon(), "Ajouter une nouvelle saison");
        QAction *modifier = menuContextuel.addAction(QIcon(), "Modifier une fiche série");;
        if(ui->pageVosSeriesDisplay->item(ligne, 1)->backgroundColor() == QColor("black")) {
            ajouter->setEnabled(true);
        } else {
            ajouter->setEnabled(false);
        }

        QAction *a = menuContextuel.exec(ui->pageVosSeriesDisplay->viewport()->mapToGlobal(pos));

        if(a == ajouter) {
            ui->stackedWidget->setCurrentWidget(ui->pageAjoutModification);
            on_menuFichierAjouter_triggered();
            ui->pageAjoutModifComboFicheSerieNom->setCurrentText(serie);
        } else if (a == modifier){
            on_menuFichierModifier_triggered();
            ui->pageAjoutModifComboFicheSerieNom->setCurrentText(ui->pageVosSeriesDisplay->item(ligne, 0)->text());
        } else {}
        ui->pageAjoutModifLabelRetour->setText("pageVosSeries");
    }
}

void FenetrePrincipale::on_pageVosSeriesComboBox_currentIndexChanged(const QString &arg1) {
    ui->pageVosSeriesDisplay->setRowCount(0);

    int selection = 1;
    if(MethodeDiverses::getConfig("Configuration/ListeSerie") == "") {
        MethodeDiverses::setConfig("Configuration/ListeSerie", "Toutes");
        selection = 1;
    } else {
        if(arg1 == "Toutes") {
            MethodeDiverses::setConfig("Configuration/ListeSerie", "Toutes");
            selection = 1;
        } else if (arg1 == "Série à venir") {
            MethodeDiverses::setConfig("Configuration/ListeSerie", "Série à venir");
            selection = 2;
        } else if (arg1 == "Série en cours") {
            MethodeDiverses::setConfig("Configuration/ListeSerie", "Série en cours");
            selection = 3;
        }
    }

    int indice = 0;
    for (int i = 0; i < listeGlobal.count(); i++) {
        QMap<QString,QString> list = listeGlobal.value(i);

        if(selection == 2 && methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_SORTIE)) <= QDate::currentDate()) {
            continue;
        } else if (selection == 3 && methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_SORTIE)) > QDate::currentDate()) {
            continue;
        } else if(list.value(bdd->SAISON_DATE_MODIF) == "3000-01-01" && (selection == 2 || selection == 3)) {
            continue;
        }

        ui->pageVosSeriesDisplay->setRowCount(indice + 1);

        if(list.value(bdd->SAISON_DATE_MODIF) == "3000-01-01" && list.value(bdd->FICHE_SERIE_TERMINE) == "0") {
            ui->pageVosSeriesDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(list.value(bdd->FICHE_SERIE_NOM), false));
            ui->pageVosSeriesDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget("", true));
            ui->pageVosSeriesDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget("", true));
            ui->pageVosSeriesDisplay->setItem(indice, 3, methodeDiverses.itemForTableWidget("", true));
            ui->pageVosSeriesDisplay->setItem(indice, 4, methodeDiverses.itemForTableWidget("", true));
            ui->pageVosSeriesDisplay->setItem(indice, 5, methodeDiverses.itemForTableWidget("", true));
            ui->pageVosSeriesDisplay->item(indice, 0)->setBackgroundColor(QColor(list.value(bdd->TYPE_SERIE_RED).toInt(), list.value(bdd->TYPE_SERIE_GREEN).toInt(), list.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pageVosSeriesDisplay->item(indice, 1)->setBackgroundColor(QColor("black"));
            ui->pageVosSeriesDisplay->item(indice, 2)->setBackgroundColor(QColor("black"));
            ui->pageVosSeriesDisplay->item(indice, 3)->setBackgroundColor(QColor("black"));
            ui->pageVosSeriesDisplay->item(indice, 4)->setBackgroundColor(QColor("black"));
            ui->pageVosSeriesDisplay->item(indice, 5)->setBackgroundColor(QColor("black"));
        } else if(list.value(bdd->SAISON_DATE_MODIF) != "3000-01-01"){
            ui->pageVosSeriesDisplay->setItem(indice, 0, methodeDiverses.itemForTableWidget(list.value(bdd->FICHE_SERIE_NOM), false));
            ui->pageVosSeriesDisplay->setItem(indice, 1, methodeDiverses.itemForTableWidget(list.value(bdd->SAISON_SAISON), true));
            ui->pageVosSeriesDisplay->setItem(indice, 2, methodeDiverses.itemForTableWidget(list.value(bdd->SAISON_EPISODE_COURANT), true));
            ui->pageVosSeriesDisplay->setItem(indice, 3, methodeDiverses.itemForTableWidget(methodeDiverses.dayToString(methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_MODIF)).dayOfWeek()), true));

            if(methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_SORTIE)) <= QDate::currentDate() &&
               (selection == 1 || selection == 3)) {
                ui->pageVosSeriesDisplay->setItem(indice, 4, methodeDiverses.itemForTableWidget("En cours", true));
            } else {
                ui->pageVosSeriesDisplay->setItem(indice, 4, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_SORTIE)).toString("dd/MM/yy"), true));
            }

            if(list.value(bdd->SAISON_DATE_MODIF) == "0") {
                ui->pageVosSeriesDisplay->setItem(indice, 5, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_SORTIE)).toString("dd/MM/yy"), true));
            } else {
                ui->pageVosSeriesDisplay->setItem(indice, 5, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(list.value(bdd->SAISON_DATE_MODIF)).addDays(7).toString("dd/MM/yy"), true));
            }

            // On met la couleur
            ui->pageVosSeriesDisplay->item(indice, 0)->setBackgroundColor(QColor(list.value(bdd->TYPE_SERIE_RED).toInt(), list.value(bdd->TYPE_SERIE_GREEN).toInt(), list.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pageVosSeriesDisplay->item(indice, 1)->setBackgroundColor(QColor(list.value(bdd->TYPE_SERIE_RED).toInt(), list.value(bdd->TYPE_SERIE_GREEN).toInt(), list.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pageVosSeriesDisplay->item(indice, 2)->setBackgroundColor(QColor(list.value(bdd->TYPE_SERIE_RED).toInt(), list.value(bdd->TYPE_SERIE_GREEN).toInt(), list.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pageVosSeriesDisplay->item(indice, 3)->setBackgroundColor(QColor(list.value(bdd->TYPE_SERIE_RED).toInt(), list.value(bdd->TYPE_SERIE_GREEN).toInt(), list.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pageVosSeriesDisplay->item(indice, 4)->setBackgroundColor(QColor(list.value(bdd->TYPE_SERIE_RED).toInt(), list.value(bdd->TYPE_SERIE_GREEN).toInt(), list.value(bdd->TYPE_SERIE_BLUE).toInt()));
            ui->pageVosSeriesDisplay->item(indice, 5)->setBackgroundColor(QColor(list.value(bdd->TYPE_SERIE_RED).toInt(), list.value(bdd->TYPE_SERIE_GREEN).toInt(), list.value(bdd->TYPE_SERIE_BLUE).toInt()));

            if(list.value(bdd->SAISON_EPISODE_COURANT) == list.value(bdd->SAISON_NB_EPISODE)) {
                for(int j = 1; j <= 5; j++) {
                    ui->pageVosSeriesDisplay->item(indice, j)->setBackgroundColor(QColor(240,63,63));
                }
            }
        }

        QSignalMapper* mapper = new QSignalMapper();
        QPushButton* dossier = new QPushButton(i_dossier,"");
        if(list.value(bdd->SAISON_DATE_MODIF) == "3000-01-01") {
            mapper->setMapping(dossier, "/" + list.value(bdd->FICHE_SERIE_NOM));
        } else {
            mapper->setMapping(dossier, "/" + list.value(bdd->FICHE_SERIE_NOM) + "/Saison " + list.value(bdd->SAISON_SAISON));
        }
        connect(mapper, SIGNAL(mapped(QString)), this, SLOT(bouton_dossier_clicked(QString)));
        connect(dossier, SIGNAL(clicked(bool)), mapper, SLOT(map()));
        ui->pageVosSeriesDisplay->setCellWidget(indice, 6, dossier);

        QFile file (dossierSerie + list.value(bdd->FICHE_SERIE_NOM));
        if(!file.exists()) {
            dossier->setEnabled(false);
        }

        QSignalMapper* mapper1 = new QSignalMapper();
        QPushButton* wiki = new QPushButton(i_wiki,"");
        if(list.value(bdd->SAISON_WIKI) != "") {
            mapper1->setMapping(wiki, list.value(bdd->SAISON_WIKI));
        } else if (list.value(bdd->FICHE_SERIE_WIKI) != "") {
            mapper1->setMapping(wiki, list.value(bdd->FICHE_SERIE_WIKI));
        } else {
            mapper1->setMapping(wiki, "");
            wiki->setEnabled(false);
        }
        connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(wiki, SIGNAL(clicked(bool)), mapper1, SLOT(map()));
        ui->pageVosSeriesDisplay->setCellWidget(indice, 7, wiki);

        if((list.value(bdd->SAISON_DATE_MODIF) == "3000-01-01" && list.value(bdd->FICHE_SERIE_TERMINE) == "0" && selection == 1) || (list.value(bdd->SAISON_DATE_MODIF) != "3000-01-01"))
            indice++;
    }

    ui->pageVosSeriesDisplay->resizeColumnsToContents();
}

/*******************************************************\
*                                                       *
*               PAGE AJOUT MODIFICATION                 *
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
    QString saisonWiki = ui->pageAjoutModifLineEditSaisonWiki->text();
    QString type = ui->pageAjoutModifComboBoxTypeSerie->currentText();
    int termine = ui->pageAjoutModifCheckBoxTermine->isChecked();
    int saisonEpisodeEnPlus;
    QString saisonDateModif;
    bool saisonCreationDossier;
    QString saisonEpisodeCourant;
    QString cheminFichier;
    BaseDeDonnees::Retour retour;

    if(ui->pageAjoutModifCheckBoxDoubleEpisode->isChecked()) {
        saisonEpisodeEnPlus = ui->pageAjoutModifSpinBoxEpisodeEnPlus->value();
    } else {
        saisonEpisodeEnPlus = 0;
    }

    if(QFile(ficheSeriePhoto).exists() && !QFileInfo("./data/images/" + QFileInfo(ficheSeriePhoto).fileName()).exists()) {
        QFile(ficheSeriePhoto).copy("./data/images/" + QFileInfo(ficheSeriePhoto).fileName());
    }
    if(QFileInfo("./data/images/" + QFileInfo(ficheSeriePhoto).fileName()).isFile()) {
        cheminFichier = QFileInfo("./data/images/" + QFileInfo(ficheSeriePhoto).fileName()).absoluteFilePath();
    } else {
        cheminFichier = "";
    }

    QStringList champs, jointures, conditions, ordres;

    if(ui->pageAjoutModifBoutonValider->text() == "Ajouter") {
        QStringList valeurs;

        saisonDateModif = methodeDiverses.dateToString(ui->pageAjoutModifDateEditSaisonDateSortie->date().addDays(-7));
        saisonCreationDossier = ui->pageAjoutModifCheckBoxCreationDossierAuto->isChecked();

        champs.append(bdd->FICHE_SERIE_ID);
        conditions.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(ficheSerieNom));
        retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer l'ID de " + ficheSerieNom;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QStringList champsType, jointuresType, conditionsType, ordresType;
        if(retour.liste.isEmpty()) {
            champs.clear();
            valeurs.clear();
            champs.append(bdd->FICHE_SERIE_NOM);
            valeurs.append(ficheSerieNom);
            champs.append(bdd->FICHE_SERIE_WIKI);
            valeurs.append(ficheSerieWiki);
            champs.append(bdd->FICHE_SERIE_IMAGE);
            valeurs.append(cheminFichier);
            champs.append(bdd->FICHE_SERIE_ADDICTED);
            valeurs.append(ficheSerieAddicted);
            champs.append(bdd->FICHE_SERIE_TERMINE);
            valeurs.append(QString::number(termine));
            champs.append(bdd->FICHE_SERIE_TYPE_SERIE_ID);
            champsType.append(bdd->TYPE_SERIE_ID);
            conditionsType.append(bdd->TYPE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(type));
            valeurs.append(bdd->getRequeteSelect(champsType, bdd->TABLE_TYPE_SERIE, jointuresType, conditionsType, ordresType));

            retour = bdd->requeteInsert(champs, valeurs, bdd->TABLE_FICHE_SERIE);
            if(!retour.reussi) {
                QString messageLog = "Impossible d'ajouter la série " + ficheSerieNom;
                QMessageBox::critical(this, this->windowTitle(), messageLog);
                MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                throw new QException();
            }
        } else {
            champs.clear();
            conditions.clear();
            champs.append(bdd->FICHE_SERIE_WIKI + bdd->EGALE + bdd->entreQuotes(ficheSerieWiki));
            champs.append(bdd->FICHE_SERIE_IMAGE + bdd->EGALE + bdd->entreQuotes(ficheSeriePhoto));
            champs.append(bdd->FICHE_SERIE_ADDICTED + bdd->EGALE + bdd->entreQuotes(ficheSerieAddicted));
            champs.append(bdd->FICHE_SERIE_TERMINE + bdd->EGALE + bdd->entreQuotes(QString::number(termine)));
            champsType.append(bdd->TYPE_SERIE_ID);
            conditionsType.append(bdd->TYPE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(type));
            champs.append(bdd->FICHE_SERIE_TYPE_SERIE_ID + bdd->EGALE + bdd->entreParentheses(bdd->getRequeteSelect(champsType, bdd->TABLE_TYPE_SERIE, jointuresType, conditionsType, ordresType)));
            conditions.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(ficheSerieNom));
            retour = bdd->requeteUpdate(champs, bdd->TABLE_FICHE_SERIE, conditions);
            if(!retour.reussi) {
                QString messageLog = "Impossible de mettre à jour la série " + ficheSerieNom;
                QMessageBox::critical(this, this->windowTitle(), messageLog);
                MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                throw new QException();
            }
        }

        if(ui->pageAjoutModifCheckBoxCreerSaison->isChecked()) {
            // On récupère l'ID de la fiche série
            champs.clear();
            champs.append(bdd->FICHE_SERIE_ID);
            retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
            if(!retour.reussi) {
                QString messageLog = "Impossible de récupérer l'ID de la série " + ficheSerieNom;
                QMessageBox::critical(this, this->windowTitle(), messageLog);
                MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                throw new QException();
            }
            ficheSerieID = retour.liste.at(0).value(bdd->FICHE_SERIE_ID).toInt();

            champs.clear();
            conditions.clear();
            champs.append(bdd->SAISON_ID);
            conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreQuotes(QString::number(ficheSerieID)));
            retour = bdd->requeteSelect(champs, bdd->TABLE_SAISON, jointures, conditions, ordres);
            if(!retour.reussi) {
                QString messageLog = "Impossible de récupérer l'ID de la saison courante de la série " + ficheSerieNom;
                QMessageBox::critical(this, this->windowTitle(), messageLog);
                MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                throw new QException();
            }
            if(retour.liste.isEmpty()) {
                champs.clear();
                valeurs.clear();
                champs.append(bdd->SAISON_ID);
                valeurs.append(bdd->entreQuotes(QString::number(ficheSerieID)));
                champs.append(bdd->SAISON_SAISON);
                valeurs.append(bdd->entreQuotes(saisonSaison));
                champs.append(bdd->SAISON_NB_EPISODE);
                valeurs.append(bdd->entreQuotes(saisonNbEpisode));
                champs.append(bdd->SAISON_EPISODE_COURANT);
                valeurs.append(bdd->entreQuotes("01"));
                champs.append(bdd->SAISON_DATE_SORTIE);
                valeurs.append(bdd->entreQuotes(saisonDateSortie));
                champs.append(bdd->SAISON_DATE_MODIF);
                valeurs.append(bdd->entreQuotes(saisonDateModif));
                champs.append(bdd->SAISON_WIKI);
                valeurs.append(bdd->entreQuotes(saisonWiki));
                retour = bdd->requeteInsert(champs, valeurs, bdd->TABLE_SAISON);
                if(!retour.reussi) {
                    QString messageLog = "Impossible d'ajouter la saison de la série " + ficheSerieNom;
                    QMessageBox::critical(this, this->windowTitle(), messageLog);
                    MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                    throw new QException();
                }
                ui->statusBar->showMessage("La série " + ficheSerieNom + " a été ajouté.", 5000);
            } else {
                QMessageBox::information(this, this->windowTitle(), "Il existe déjà une saison pour cette série.\nLa saison n'a pas été créé.");
            }

            if(saisonCreationDossier) {
                if(!QDir(dossierSerie + "/" + ficheSerieNom + "/Saison " + saisonSaison).exists()) {
                    QDir dir;
                    if(!dir.mkpath(dossierSerie + "/" + ficheSerieNom + "/Saison " + saisonSaison)) {
                        QMessageBox::information(this, this->windowTitle(), "Impossible de créer le dossier suivant :" + dossierSerie + "/" + ficheSerieNom + "/Saison " + saisonSaison);
                    }
                }
            }
        }
        champs.clear();
        conditions.clear();
        ui->pageAjoutModifComboFicheSerieNom->clear();
        champs.append(bdd->FICHE_SERIE_NOM);
        conditions.append(bdd->FICHE_SERIE_NOM + bdd->NOT + bdd->IN + bdd->entreParentheses(bdd->SELECT + bdd->FICHE_SERIE_NOM + bdd->FROM + bdd->TABLE_FICHE_SERIE + bdd->INNER_JOIN + bdd->TABLE_SAISON + bdd->ON + bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->SAISON_ID));
        conditions.append(bdd->FICHE_SERIE_TERMINE + bdd->EGALE + "0");
        ordres.append(bdd->FICHE_SERIE_NOM);
        retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de de récupérer les séries";
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QList<QMap<QString, QString> > serieSansSaison = retour.liste;
        ui->pageAjoutModifComboFicheSerieNom->addItem("");
        for(int i = 0; i < serieSansSaison.count(); i++) {
            ui->pageAjoutModifComboFicheSerieNom->addItem(serieSansSaison.at(i).value(bdd->FICHE_SERIE_NOM));
        }
    } else {
        saisonDateModif = methodeDiverses.dateToString(ui->pageAjoutModifDateEditSaisonDateProchain->date().addDays(-7));
        saisonEpisodeCourant = methodeDiverses.formalismeEntier(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->value());
        champs.clear();
        conditions.clear();
        champs.append(bdd->FICHE_SERIE_ID);

        conditions.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(ficheSerieNom));

        retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer l'ID de la série " + ficheSerieNom;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        ficheSerieID = retour.liste.at(0).value(bdd->FICHE_SERIE_ID).toInt();

        champs.clear();
        conditions.clear();
        champs.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(ficheSerieNom));
        champs.append(bdd->FICHE_SERIE_IMAGE + bdd->EGALE + bdd->entreQuotes(cheminFichier));
        champs.append(bdd->FICHE_SERIE_WIKI + bdd->EGALE + bdd->entreQuotes(ficheSerieWiki));
        champs.append(bdd->FICHE_SERIE_ADDICTED + bdd->EGALE + bdd->entreQuotes(ficheSerieAddicted));
        champs.append(bdd->FICHE_SERIE_TERMINE + bdd->EGALE + bdd->entreQuotes(QString::number(termine)));
        conditions.append(bdd->FICHE_SERIE_ID + bdd->EGALE + bdd->entreQuotes(QString::number(ficheSerieID)));
        bdd->requeteUpdate(champs, bdd->TABLE_FICHE_SERIE, conditions);
        if(!retour.reussi) {
            QString messageLog = "Impossible de mettre à jour la série " + ficheSerieNom;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }

        champs.clear();
        conditions.clear();

        champs.append(bdd->SAISON_SAISON + bdd->EGALE + bdd->entreQuotes(saisonSaison));
        champs.append(bdd->SAISON_NB_EPISODE + bdd->EGALE + bdd->entreQuotes(saisonNbEpisode));
        champs.append(bdd->SAISON_EPISODE_COURANT + bdd->EGALE + bdd->entreQuotes(saisonEpisodeCourant));
        champs.append(bdd->SAISON_DATE_SORTIE + bdd->EGALE + bdd->entreQuotes(saisonDateSortie));
        champs.append(bdd->SAISON_DATE_MODIF + bdd->EGALE + bdd->entreQuotes(saisonDateModif));
        champs.append(bdd->SAISON_WIKI + bdd->EGALE + bdd->entreQuotes(saisonWiki));
        champs.append(bdd->SAISON_EPISODE_EN_PLUS + bdd->EGALE + bdd->entreQuotes(QString::number(saisonEpisodeEnPlus)));
        conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreQuotes(QString::number(ficheSerieID)));
        bdd->requeteUpdate(champs, bdd->TABLE_SAISON, conditions);
        if(!retour.reussi) {
            QString messageLog = "Impossible de mettre à jour la saison de la série " + ficheSerieNom;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }

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
        QStringList champs, jointures, conditions, ordres;

        champs.append(bdd->FICHE_SERIE_ID);
        conditions.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(ui->pageAjoutModifComboFicheSerieNom->currentText()));

        BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer l'ID de la série " + ui->pageAjoutModifComboFicheSerieNom->currentText();
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonDossier_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        QList<QMap<QString,QString> > liste = retour.liste;
        champs.clear();
        conditions.clear();
        champs.append(bdd->SAISON_SAISON);
        conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreQuotes(liste.at(0).value(bdd->FICHE_SERIE_ID)));

        retour = bdd->requeteSelect(champs, bdd->TABLE_SAISON, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer le numéro de saison de la série " + ui->pageAjoutModifComboFicheSerieNom->currentText();
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifBoutonDossier_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        liste = retour.liste;

        if(liste.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + "/" + ui->pageAjoutModifComboFicheSerieNom->currentText()));
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dossierSerie + "/" + ui->pageAjoutModifComboFicheSerieNom->currentText() + "/Saison " + liste.at(0).value(bdd->SAISON_SAISON)));
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
    ui->pageAjoutModifGroupBoxSaison->setVisible(arg1);
}

void FenetrePrincipale::on_pageAjoutModifPushButtonSupprimerSaison_clicked() {
    QStringList champs, jointures, conditions, ordres;

    champs.append(bdd->FICHE_SERIE_ID);
    conditions.append(bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(ui->pageAjoutModifComboFicheSerieNom->currentText()));
    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer l'ID de la série " + ui->pageAjoutModifComboFicheSerieNom->currentText();
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifPushButtonSupprimerSaison_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QString serieID = retour.liste.at(0).value(bdd->FICHE_SERIE_ID);


    conditions.clear();
    conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreQuotes(serieID));
    bdd->requeteDelete(bdd->TABLE_SAISON, conditions);
    if(!retour.reussi) {
        QString messageLog = "Impossible de mettre à jour la saison de la série " + ui->pageAjoutModifComboFicheSerieNom->currentText();
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifPushButtonSupprimerSaison_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }

    on_pageAjoutModifComboFicheSerieNom_currentTextChanged(ui->pageAjoutModifComboFicheSerieNom->currentText());

    refresh();
}

void FenetrePrincipale::on_pageAjoutModifComboBoxListeSaison_currentTextChanged(const QString &arg1) {
    QStringList champs, jointures, conditions, ordres;
    QString qualite, sousTitres, fin = "&do=search&order=desc&sort=publish_date", saison = arg1;
    champs.append(bdd->HISTORIQUE_NOM);
    champs.append(bdd->HISTORIQUE_SAISON);
    champs.append(bdd->HISTORIQUE_EPISODE);
    champs.append(bdd->HISTORIQUE_DATE_AJOUT);
    champs.append(bdd->HISTORIQUE_ETAT);
    conditions.append(bdd->HISTORIQUE_NOM + bdd->EGALE + bdd->entreQuotes(ui->pageAjoutModifComboFicheSerieNom->currentText()));
    conditions.append(bdd->HISTORIQUE_SAISON + bdd->EGALE + bdd->entreQuotes(saison.replace("Saison ", "")));

    switch (MethodeDiverses::getConfig("Configuration/Qualite").toInt()) {
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

    switch (MethodeDiverses::getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        sousTitres = "";
        break;
    case 1:
        sousTitres = "+VOSTFR";
        break;
    }


    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer l'historique de la saison " + saison.replace("Saison ", "") + " de la série " + ui->pageAjoutModifComboFicheSerieNom->currentText();
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifComboBoxListeSaison_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString, QString> > listeEpisodeHistorique = retour.liste;
    ui->pageAjoutModifTableWidgetListeSaison->setRowCount(0);
    for(int i = 0; i < listeEpisodeHistorique.count(); i++) {
        ui->pageAjoutModifTableWidgetListeSaison->setRowCount(ui->pageAjoutModifTableWidgetListeSaison->rowCount() + 1);
        QMap<QString, QString> map = listeEpisodeHistorique.at(i);

        QSignalMapper* mapper = new QSignalMapper();
        QSignalMapper* mapper1 = new QSignalMapper();
        QPushButton* url = new QPushButton(i_torrent, "");
        QComboBox *etat = new QComboBox();

        etat->addItem("Non vu");
        etat->addItem("Téléchargé");
        etat->addItem("Vu");

        url->setToolTip("Ouvre le lien URL de " + map.value(bdd->HISTORIQUE_NOM) + " pour l'épisode " + map.value(bdd->HISTORIQUE_EPISODE));
        mapper->setMapping(url, lien + QString(map.value(bdd->HISTORIQUE_NOM)).replace(" ","+") + "+S" + map.value(bdd->HISTORIQUE_SAISON) + "E" + map.value(bdd->HISTORIQUE_EPISODE) + sousTitres + qualite + fin);
        connect(mapper, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(url, SIGNAL(clicked()), mapper, SLOT(map()));

        QWidget* widgetEtat = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->addWidget(etat);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0,0,0,0);
        widgetEtat->setLayout(layout);

        if(map.value(bdd->HISTORIQUE_ETAT) == "NV") {
            etat->setCurrentText("Non vu");
        } else if (map.value(bdd->HISTORIQUE_ETAT) == "T") {
            etat->setCurrentText("Téléchargé");
        } else {
            etat->setCurrentText("Vu");
        }

        mapper1->setMapping(etat, map.value(bdd->HISTORIQUE_NOM) + 30 + map.value(bdd->HISTORIQUE_SAISON) + 30 + map.value(bdd->HISTORIQUE_EPISODE));
        connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(comboBox_etat_changed(QString)));
        connect(etat, SIGNAL(currentTextChanged(QString)), mapper1, SLOT(map()));

        ui->pageAjoutModifTableWidgetListeSaison->setItem(i, 0, methodeDiverses.itemForTableWidget(map.value(bdd->HISTORIQUE_NOM), true));
        ui->pageAjoutModifTableWidgetListeSaison->setItem(i, 1, methodeDiverses.itemForTableWidget(map.value(bdd->HISTORIQUE_EPISODE), true));
        ui->pageAjoutModifTableWidgetListeSaison->setItem(i, 2, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(map.value(bdd->HISTORIQUE_DATE_AJOUT)).toString("dd/MM/yyyy"), true));
        ui->pageAjoutModifTableWidgetListeSaison->setCellWidget(i, 3 , url);
        ui->pageAjoutModifTableWidgetListeSaison->setCellWidget(i, 4 , widgetEtat);
    }
}

void FenetrePrincipale::on_pageAjoutModifComboFicheSerieNom_currentTextChanged(const QString &arg1) {
    ui->pageAjoutModifTableWidgetListeSaison->setRowCount(0);
    ui->pageAjoutModifComboBoxListeSaison->clear();

    QStringList champs, jointures, conditions, ordres;
    champs.append(bdd->FICHE_SERIE_WIKI);
    champs.append(bdd->FICHE_SERIE_IMAGE);
    champs.append(bdd->FICHE_SERIE_ADDICTED);
    champs.append(bdd->FICHE_SERIE_ID);
    champs.append(bdd->FICHE_SERIE_TERMINE);
    champs.append(bdd->TYPE_SERIE_NOM);
    jointures.append(bdd->INNER_JOIN + bdd->TABLE_TYPE_SERIE + bdd->ON + bdd->FICHE_SERIE_TYPE_SERIE_ID + bdd->EGALE + bdd->TYPE_SERIE_ID);
    conditions.append(bdd->UPPER + bdd->entreParentheses(bdd->FICHE_SERIE_NOM) + bdd->EGALE + bdd->UPPER + bdd->entreParentheses(bdd->entreQuotes(arg1)));
    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_FICHE_SERIE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les informations de la série " + arg1;
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifComboFicheSerieNom_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString, QString> > liste = retour.liste;

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
        ui->pageAjoutModifLineEditFicheSerieWiki->setText(liste.at(0).value(bdd->FICHE_SERIE_WIKI));
        ui->pageAjoutModifLineEditFicheSerieWiki->setCursorPosition(0);
        ui->pageAjoutModifLineEditAddicted->setText(liste.at(0).value(bdd->FICHE_SERIE_ADDICTED));
        ui->pageAjoutModifLineEditAddicted->setCursorPosition(0);
        ui->pageAjoutModifCheckBoxTermine->setChecked(liste.at(0).value(bdd->FICHE_SERIE_TERMINE).toInt());
        ui->pageAjoutModifComboBoxTypeSerie->setCurrentText(liste.at(0).value(bdd->TYPE_SERIE_NOM));
        if(liste.at(0).value(bdd->FICHE_SERIE_IMAGE) != "") {
            ui->pageAjoutModifLabelPhoto->setPixmap(QPixmap(liste.at(0).value(bdd->FICHE_SERIE_IMAGE)).scaled(ui->pageAjoutModifLabelPhoto->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->pageAjoutModifLabelCheminPhoto->setText(liste.at(0).value(bdd->FICHE_SERIE_IMAGE));
        } else {
            ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
            ui->pageAjoutModifLabelCheminPhoto->setText("");
        }

        champs.clear();
        jointures.clear();
        conditions.clear();
        champs.append(bdd->SAISON_SAISON);
        champs.append(bdd->SAISON_NB_EPISODE);
        champs.append(bdd->SAISON_EPISODE_COURANT);
        champs.append(bdd->SAISON_DATE_SORTIE);
        champs.append(bdd->SAISON_WIKI);
        champs.append(bdd->SAISON_DATE_MODIF);
        champs.append(bdd->SAISON_EPISODE_EN_PLUS);
        conditions.append(bdd->SAISON_ID + bdd->EGALE + liste.at(0).value(bdd->FICHE_SERIE_ID));

        retour = bdd->requeteSelect(champs, bdd->TABLE_SAISON, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer la saison de la série " + arg1;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifComboFicheSerieNom_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        liste = retour.liste;

        if(!liste.isEmpty()) {
            ui->pageAjoutModifSpinBoxSaisonSaison->setValue(liste.at(0).value(bdd->SAISON_SAISON).toInt());
            ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(liste.at(0).value(bdd->SAISON_NB_EPISODE).toInt());
            ui->pageAjoutModifDateEditSaisonDateSortie->setDate(methodeDiverses.stringToDate(liste.at(0).value(bdd->SAISON_DATE_SORTIE)));
            if(ui->pageAjoutModifDateEditSaisonDateSortie->date() >= QDate::currentDate()) {
                ui->pageAjoutModifDateEditSaisonDateSortie->setEnabled(true);
                ui->pageAjoutModifDateEditSaisonDateProchain->setEnabled(false);
            } else {
                ui->pageAjoutModifDateEditSaisonDateSortie->setEnabled(false);
                ui->pageAjoutModifDateEditSaisonDateProchain->setEnabled(true);
            }
            ui->pageAjoutModifLineEditSaisonWiki->setText(liste.at(0).value(bdd->SAISON_WIKI));
            ui->pageAjoutModifLineEditSaisonWiki->setCursorPosition(0);
            ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(liste.at(0).value(bdd->SAISON_EPISODE_COURANT).toInt());
            ui->pageAjoutModifDateEditSaisonDateProchain->setDate(methodeDiverses.stringToDate(liste.at(0).value(bdd->SAISON_DATE_MODIF)).addDays(7));
            if(liste.at(0).value(bdd->SAISON_EPISODE_EN_PLUS).toInt()) {
                ui->pageAjoutModifCheckBoxDoubleEpisode->setChecked(true);
                ui->pageAjoutModifSpinBoxEpisodeEnPlus->setValue(liste.at(0).value(bdd->SAISON_EPISODE_EN_PLUS).toInt());
            } else {
                ui->pageAjoutModifCheckBoxDoubleEpisode->setChecked(false);
                ui->pageAjoutModifSpinBoxEpisodeEnPlus->setValue(ui->pageAjoutModifSpinBoxEpisodeEnPlus->minimum());
            }
            ui->pageAjoutModifSpinBoxEpisodeEnPlus->setMaximum(liste.at(0).value(bdd->SAISON_NB_EPISODE).toInt() - liste.at(0).value(bdd->SAISON_EPISODE_COURANT).toInt());
        } else {
            ui->pageAjoutModifGroupBoxSaison->setVisible(false);
            ui->pageAjoutModifPushButtonSupprimerSaison->setEnabled(false);
            ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
            ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(1);
            ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
            ui->pageAjoutModifLineEditSaisonWiki->setText("");
            ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
            ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());
        }
    } else if(!liste.isEmpty() && ui->pageAjoutModifBoutonValider->text() == "Ajouter") {
        champs.clear();
        jointures.clear();
        conditions.clear();
        ordres.clear();
        champs.append(bdd->SAISON_ID);
        conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreQuotes(liste.at(0).value(bdd->FICHE_SERIE_ID)));

        retour = bdd->requeteSelect(champs, bdd->TABLE_SAISON, jointures, conditions, ordres);
        if(!retour.reussi) {
            QString messageLog = "Impossible de récupérer l'ID de la saison courante de la série " + arg1;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifComboFicheSerieNom_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
        if(retour.liste.isEmpty()) {
            ui->pageAjoutModifLineEditFicheSerieWiki->setText(liste.at(0).value(bdd->FICHE_SERIE_WIKI));
            ui->pageAjoutModifLineEditAddicted->setText(liste.at(0).value(bdd->FICHE_SERIE_ADDICTED));
            if(liste.at(0).value(bdd->FICHE_SERIE_IMAGE) != "") {
                ui->pageAjoutModifLabelPhoto->setPixmap(QPixmap(liste.at(0).value(bdd->FICHE_SERIE_IMAGE)).scaled(ui->pageAjoutModifLabelPhoto->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->pageAjoutModifLabelCheminPhoto->setText(liste.at(0).value(bdd->FICHE_SERIE_IMAGE));
            } else {
                ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
            }

            if(arg1 != "") {
                ui->pageAjoutModifBoutonValider->setEnabled(ui->pageAjoutModifCheckBoxCreerSaison->isChecked());
            } else {
                ui->pageAjoutModifBoutonValider->setEnabled(false);
            }
        } else {
            ui->pageAjoutModifBoutonValider->setEnabled(false);
            ui->statusBar->showMessage("Il existe déjà une saison pour " + arg1, 5000);
        }
        ui->pageAjoutModifDateEditSaisonDateSortie->setEnabled(true);
    } else {
        if(ui->pageAjoutModifBoutonValider->text() == "Modifier") {
            ui->pageAjoutModifGroupBoxSaison->setVisible(false);
            ui->pageAjoutModifPushButtonSupprimerSaison->setEnabled(false);
        }
        ui->pageAjoutModifDateEditSaisonDateSortie->setEnabled(true);
        ui->pageAjoutModifLineEditFicheSerieWiki->setText("");
        ui->pageAjoutModifLineEditAddicted->setText("");
        ui->pageAjoutModifLabelPhoto->setPixmap(i_seriesManager);
        ui->pageAjoutModifLabelCheminPhoto->setText("");
        ui->pageAjoutModifLineEditDepuisURL->setText("");
        ui->pageAjoutModifSpinBoxSaisonSaison->setValue(ui->pageAjoutModifSpinBoxSaisonSaison->minimum());
        ui->pageAjoutModifSpinBoxSaisonNbEpisode->setValue(1);
        ui->pageAjoutModifCheckBoxCreationDossierAuto->setChecked(true);
        ui->pageAjoutModifDateEditSaisonDateSortie->setDate(QDate::currentDate());
        ui->pageAjoutModifLineEditSaisonWiki->setText("");
        ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->setValue(ui->pageAjoutModifSpinBoxSaisonEpisodeCourant->minimum());
        ui->pageAjoutModifDateEditSaisonDateProchain->setDate(QDate::currentDate());
    }

    champs.clear();
    jointures.clear();
    conditions.clear();
    champs.append(bdd->DISTINCT + bdd->HISTORIQUE_SAISON);
    conditions.append(bdd->HISTORIQUE_NOM + bdd->EGALE + bdd->entreQuotes(arg1));

    retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer les saisons de la série " + arg1 + " dans l'historique";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageAjoutModifComboFicheSerieNom_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString, QString> > listeSaison = retour.liste;
    if(listeSaison.isEmpty()) {
        ui->pageAjoutModifComboBoxListeSaison->setEnabled(false);
        ui->pageAjoutModifTableWidgetListeSaison->setEnabled(false);
    } else {
        ui->pageAjoutModifComboBoxListeSaison->setEnabled(true);
        ui->pageAjoutModifTableWidgetListeSaison->setEnabled(true);
        for (int i = 0; i < listeSaison.count(); i++) {
            ui->pageAjoutModifComboBoxListeSaison->addItem("Saison " + listeSaison.at(i).value(bdd->HISTORIQUE_SAISON));
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

void FenetrePrincipale::on_pageAjoutModifCheckBoxDoubleEpisode_stateChanged(int arg1) {
    ui->pageAjoutModifSpinBoxEpisodeEnPlus->setEnabled(arg1);
}

/*******************************************************\
*                                                       *
*                    PAGE REPORTER                      *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageReporterButtonValider_clicked() {
    QStringList champs, conditions;
    champs.append(bdd->SAISON_DATE_MODIF + bdd->EGALE + bdd->entreQuotes(QDate::currentDate().addDays(7 * (ui->pageReporterSpinBox->value() - 1)).toString("yyyy-MM-dd")));
    conditions.append(bdd->SAISON_ID + bdd->EGALE + bdd->entreParentheses(bdd->SELECT + bdd->FICHE_SERIE_ID + bdd->FROM + bdd->TABLE_FICHE_SERIE + bdd->WHERE + bdd->FICHE_SERIE_NOM + bdd->EGALE + bdd->entreQuotes(ui->pageReporterLabelNomSerie->text())));

    BaseDeDonnees::Retour retour = bdd->requeteUpdate(champs, bdd->TABLE_SAISON, conditions);
    if(!retour.reussi) {
        QString messageLog = "Impossible de mettre à jour la série " + ui->pageReporterLabelNomSerie->text();
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageReporterButtonValider_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
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
*               PAGE HISTORIQUE CONSULTER               *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueComboBoxEtat_currentTextChanged(const QString &arg1) {
    QString condition;
    if(arg1 == "Tous") {
        condition = "";
    } else if(arg1 == "Non vu") {
        condition = bdd->HISTORIQUE_ETAT + " = 'NV'";
    } else if(arg1 == "Téléchargé") {
        condition = bdd->HISTORIQUE_ETAT + " = 'T'";
    } else {
        condition = bdd->HISTORIQUE_ETAT + " = 'V'";
    }

    // Refresh de l'historique
    ui->pageHistoriqueTableWidget->setRowCount(0);

    QStringList champs, jointures, conditions, ordres;
    int indice = 0;
    QString qualite;
    QString sousTitres;
    QString fin = "&do=search&order=desc&sort=publish_date";

    switch (MethodeDiverses::getConfig("Configuration/Qualite").toInt()) {
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

    switch (MethodeDiverses::getConfig("Configuration/SousTitres").toInt()) {
    case 0:
        sousTitres = "";
        break;
    case 1:
        sousTitres = "+VOSTFR";
        break;
    }

    champs.append(bdd->HISTORIQUE_NOM);
    champs.append(bdd->HISTORIQUE_SAISON);
    champs.append(bdd->HISTORIQUE_EPISODE);
    champs.append(bdd->HISTORIQUE_DATE_AJOUT);
    champs.append(bdd->HISTORIQUE_ETAT);
    if(!condition.isEmpty())
        conditions.append(condition);
    ordres.append(bdd->HISTORIQUE_DATE_AJOUT + bdd->DESC);
    ordres.append(bdd->HISTORIQUE_NOM);
    ordres.append(bdd->HISTORIQUE_SAISON + bdd->DESC);
    ordres.append(bdd->HISTORIQUE_EPISODE + bdd->DESC);
    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer l'historique des séries";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueComboBoxEtat_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString, QString> > listeHistorique = retour.liste;
    for(int i = 0; i < listeHistorique.count(); i++) {
        QMap<QString, QString> map = listeHistorique.value(i);

        QSignalMapper* mapper = new QSignalMapper();
        QSignalMapper* mapper1 = new QSignalMapper();

        QPushButton* url = new QPushButton(i_torrent, "");
        QComboBox *etat = new QComboBox();

        etat->addItem("Non vu");
        etat->addItem("Téléchargé");
        etat->addItem("Vu");

        url->setToolTip("Ouvre le lien URL de " + map.value(bdd->HISTORIQUE_NOM) + " pour l'épisode donné");

        mapper->setMapping(url, lien + QString(map.value(bdd->HISTORIQUE_NOM)).replace(" ","+") + "+S" + map.value(bdd->HISTORIQUE_SAISON) + "E" + map.value(bdd->HISTORIQUE_EPISODE) + sousTitres + qualite + fin);
        connect(mapper, SIGNAL(mapped(QString)), this, SLOT(bouton_lien_clicked(QString)));
        connect(url, SIGNAL(clicked()), mapper, SLOT(map()));

        QWidget *widgetEtat = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widgetEtat);
        layout->addWidget(etat);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0,0,0,0);
        widgetEtat->setLayout(layout);

        if(map.value(bdd->HISTORIQUE_ETAT) == "NV") {
            etat->setCurrentText("Non vu");
        } else if (map.value(bdd->HISTORIQUE_ETAT) == "T") {
            etat->setCurrentText("Téléchargé");
        } else {
            etat->setCurrentText("Vu");
        }

        mapper1->setMapping(etat, map.value(bdd->HISTORIQUE_NOM) + 30 + map.value(bdd->HISTORIQUE_SAISON) + 30 + map.value(bdd->HISTORIQUE_EPISODE));
        connect(mapper1, SIGNAL(mapped(QString)), this, SLOT(comboBox_etat_changed(QString)));
        connect(etat, SIGNAL(currentTextChanged(QString)), mapper1, SLOT(map()));

        ui->pageHistoriqueTableWidget->setRowCount(indice+1);
        ui->pageHistoriqueTableWidget->setItem(indice, 0, methodeDiverses.itemForTableWidget(map.value(bdd->HISTORIQUE_NOM), false));
        ui->pageHistoriqueTableWidget->setItem(indice, 1, methodeDiverses.itemForTableWidget(map.value(bdd->HISTORIQUE_SAISON), true));
        ui->pageHistoriqueTableWidget->setItem(indice, 2, methodeDiverses.itemForTableWidget(map.value(bdd->HISTORIQUE_EPISODE), true));
        ui->pageHistoriqueTableWidget->setItem(indice, 3, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(map.value(bdd->HISTORIQUE_DATE_AJOUT)).toString("dd/MM/yy"), true));
        ui->pageHistoriqueTableWidget->setCellWidget(indice, 4, url);
        ui->pageHistoriqueTableWidget->setCellWidget(indice, 5, widgetEtat);

        indice++;
    }

    MethodeDiverses::ecrireLog("\tActualisation du tableau de l'historique effectué");

    ui->pageHistoriqueTableWidget->resizeColumnsToContents();
    ui->pageHistoriqueTableWidget->setColumnWidth(4, 60);


}

/*******************************************************\
*                                                       *
*               PAGE HISTORIQUE RECHERCHER              *
*                                                       *
\*******************************************************/

void FenetrePrincipale::on_pageHistoriqueRechercherPushButtonAjoutNomSerie_clicked() {
    QString nomSerie = ui->pageHistoriqueRechercherComboBoxNomSerie->currentText();
    QStringList serieChoisi = ui->pageHistoriqueRechercherLabelAffichage->text().split(", ");
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
    QStringList serieChoisi = ui->pageHistoriqueRechercherLabelAffichage->text().split(", ");
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
    QStringList listeSerie = ui->pageHistoriqueRechercherLabelAffichage->text().split(", ");
    QStringList champs, jointures, conditions, ordres;

    champs.append(bdd->HISTORIQUE_NOM);
    champs.append(bdd->HISTORIQUE_SAISON);
    champs.append(bdd->HISTORIQUE_EPISODE);
    champs.append(bdd->HISTORIQUE_DATE_AJOUT);
    QString serie;
    for(int i = 0; i < listeSerie.count(); i++) {
        if(i == 0) {
            serie.append(bdd->entreQuotes(listeSerie.at(i)));
        } else {
            serie.append(bdd->VIRGULE + bdd->entreQuotes(listeSerie.at(i)));
        }
    }
    conditions.append(bdd->HISTORIQUE_NOM + bdd->IN + bdd->entreParentheses(serie));
    ordres.append(bdd->HISTORIQUE_DATE_AJOUT);

    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer l'historique des séries : " + ui->pageHistoriqueRechercherLabelAffichage->text();
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueRechercherPushButtonRechercher_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString, QString> > liste = retour.liste;
    ui->pageHistoriqueRechercherTableWidget->setRowCount(0);
    for(int i = 0; i < liste.count(); i++) {
        ui->pageHistoriqueRechercherTableWidget->setRowCount(ui->pageHistoriqueRechercherTableWidget->rowCount() + 1);
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 0, methodeDiverses.itemForTableWidget(liste.at(i).value(bdd->HISTORIQUE_NOM), false));
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 1, methodeDiverses.itemForTableWidget(liste.at(i).value(bdd->HISTORIQUE_SAISON), true));
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 2, methodeDiverses.itemForTableWidget(liste.at(i).value(bdd->HISTORIQUE_EPISODE), true));
        ui->pageHistoriqueRechercherTableWidget->setItem(i, 3, methodeDiverses.itemForTableWidget(methodeDiverses.stringToDate(liste.at(i).value(bdd->HISTORIQUE_DATE_AJOUT)).toString("dd/MM/yyyy"), true));
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
    QComboBox *etat = new QComboBox();
    QWidget *widgetSaison = new QWidget();
    QWidget *widgetEpisode = new QWidget();
    QWidget *widgetDateDiffusion = new QWidget();
    QWidget *widgetEtat = new QWidget();
    QHBoxLayout *layoutSaison = new QHBoxLayout(widgetSaison);
    QHBoxLayout *layoutEpisode = new QHBoxLayout(widgetEpisode);
    QHBoxLayout *layoutDateDiffusion = new QHBoxLayout(widgetDateDiffusion);
    QHBoxLayout *layoutEtat = new QHBoxLayout(widgetEtat);

    etat->addItem("Non vu");
    etat->addItem("Téléchargé");
    etat->addItem("Vu");

    spinBoxSaison->setMinimum(1);
    spinBoxEpisode->setMinimum(1);
    dateEditDateDiffusion->setDate(QDate());

    layoutSaison->addWidget(spinBoxSaison);
    layoutEpisode->addWidget(spinBoxEpisode);
    layoutDateDiffusion->addWidget(dateEditDateDiffusion);
    layoutEtat->addWidget(etat);

    layoutSaison->setContentsMargins(0,0,0,0);
    layoutEpisode->setContentsMargins(0,0,0,0);
    layoutDateDiffusion->setContentsMargins(0,0,0,0);
    layoutEtat->setContentsMargins(0,0,0,0);

    layoutEtat->setAlignment(Qt::AlignCenter);

    widgetSaison->setLayout(layoutSaison);
    widgetEpisode->setLayout(layoutEpisode);
    widgetDateDiffusion->setLayout(layoutDateDiffusion);
    widgetEtat->setLayout(layoutEtat);

    ui->pageHistoriqueAjouterTableWidget->setCellWidget(ligne, 0, widgetSaison);
    ui->pageHistoriqueAjouterTableWidget->setCellWidget(ligne, 1, widgetEpisode);
    ui->pageHistoriqueAjouterTableWidget->setCellWidget(ligne, 2, widgetDateDiffusion);
    ui->pageHistoriqueAjouterTableWidget->setCellWidget(ligne, 3, widgetEtat);
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
            QComboBox *etat = (QComboBox *)ui->pageHistoriqueAjouterTableWidget->cellWidget(i, 3)->layout()->itemAt(0)->widget();

            QStringList champs, valeurs;
            champs.append(bdd->HISTORIQUE_NOM);
            valeurs.append(ui->pageHistoriqueAjouterComboBoxSerie->currentText());
            champs.append(bdd->HISTORIQUE_SAISON);
            valeurs.append(methodeDiverses.formalismeEntier(spinBoxSaison->value()));
            champs.append(bdd->HISTORIQUE_EPISODE);
            valeurs.append(methodeDiverses.formalismeEntier(spinBoxEpisode->value()));
            champs.append(bdd->HISTORIQUE_DATE_AJOUT);
            valeurs.append(dateEditDateDiffusion->date().toString("yyyy-MM-dd"));
            champs.append(bdd->HISTORIQUE_ETAT);
            if(etat->currentText() == "Non vu") {
                valeurs.append("NV");
            } else if (etat->currentText() == "Téléchargé") {
                valeurs.append("T");
            } else {
                valeurs.append("V");
            }
            BaseDeDonnees::Retour retour = bdd->requeteInsert(champs, valeurs, bdd->TABLE_HISTORIQUE);
            if(!retour.reussi) {
                QString messageLog = "Impossible d'ajouter dans l'historique l'épisode " + ui->pageHistoriqueAjouterComboBoxSerie->currentText() + " S" + methodeDiverses.formalismeEntier(spinBoxSaison->value()) + "E" + methodeDiverses.formalismeEntier(spinBoxEpisode->value());
                QMessageBox::critical(this, this->windowTitle(), messageLog);
                MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueAjouterPushButtonAjouter_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
                throw new QException();
            }

        }
        ui->pageHistoriqueAjouterTableWidget->setRowCount(0);
    }
    refresh();
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

    QStringList champs, jointures, conditions, ordres;
    champs.append(bdd->HISTORIQUE_DATE_AJOUT);
    conditions.append(bdd->HISTORIQUE_NOM + bdd->EGALE + bdd->entreQuotes(nom));
    conditions.append(bdd->HISTORIQUE_SAISON + bdd->EGALE + bdd->entreQuotes(saison));
    conditions.append(bdd->HISTORIQUE_EPISODE + bdd->EGALE + bdd->entreQuotes(episode));

    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer l'épisode " + nom + " S" + saison + "E" + episode;
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueModifierPushButtonRechercher_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString, QString> > liste = retour.liste;

    if(!liste.isEmpty()) {
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setEnabled(true);
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setDate(methodeDiverses.stringToDate(liste.at(0).value(bdd->HISTORIQUE_DATE_AJOUT)));
    } else {
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setEnabled(false);
        ui->pageHistoriqueModifierDateEditModificationDateDiffusion->setDate(QDate());
    }

}

void FenetrePrincipale::on_pageHistoriqueModifierPushButtonModifier_clicked() {
    QString nom = ui->pageHistoriqueModifierComboBoxRechercheNom->currentText();
    QString saison = methodeDiverses.formalismeEntier(ui->pageHistoriqueModifierSpinBoxRechercheSaison->value());
    QString episode = methodeDiverses.formalismeEntier(ui->pageHistoriqueModifierSpinBoxRechercheEpisode->value());
    QStringList champs, conditions;
    champs.append(bdd->HISTORIQUE_DATE_AJOUT + bdd->EGALE + bdd->entreQuotes(ui->pageHistoriqueModifierDateEditModificationDateDiffusion->date().toString("yyyy-MM-dd")));
    conditions.append(bdd->HISTORIQUE_NOM + bdd->EGALE + bdd->entreQuotes(nom));
    conditions.append(bdd->HISTORIQUE_SAISON + bdd->EGALE + bdd->entreQuotes(saison));
    conditions.append(bdd->HISTORIQUE_EPISODE + bdd->EGALE + bdd->entreQuotes(episode));
    BaseDeDonnees::Retour retour = bdd->requeteUpdate(champs, bdd->TABLE_HISTORIQUE, conditions);
    if(!retour.reussi) {
        QString messageLog = "Impossible de mettre à jour l'épisode " + nom + " S" + saison + "E" + episode;
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueModifierPushButtonModifier_clicked() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    ui->statusBar->showMessage("La date de l'épisode " + episode + " de la saison " + saison + " de la série " + nom + " a été modifié", 5000);
}

void FenetrePrincipale::on_pageHistoriqueModifierComboBoxRechercheNom_currentTextChanged(const QString &arg1) {
    QStringList champs, jointures, conditions, ordres;

    champs.append(bdd->MAX + bdd->entreParentheses(bdd->HISTORIQUE_SAISON));
    conditions.append(bdd->HISTORIQUE_NOM + bdd->EGALE + bdd->entreQuotes(arg1));

    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer la derniere saison de la série " + arg1 + " dans l'historique";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueModifierComboBoxRechercheNom_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString,QString> > liste = retour.liste;
    ui->pageHistoriqueModifierSpinBoxRechercheSaison->setMaximum(liste.at(0).value(bdd->MAX + bdd->entreParentheses(bdd->HISTORIQUE_SAISON)).toInt());

    champs.clear();
    champs.append(bdd->MIN + bdd->entreParentheses(bdd->HISTORIQUE_SAISON));

    retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer la première saison de la série " + arg1 + " dans l'historique";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueModifierComboBoxRechercheNom_currentTextChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    liste = retour.liste;
    ui->pageHistoriqueModifierSpinBoxRechercheSaison->setMinimum(liste.at(0).value("MIN(" + bdd->HISTORIQUE_SAISON + ")").toInt());
}

void FenetrePrincipale::on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged(int arg1) {
    QStringList champs, jointures, conditions, ordres;

    champs.append(bdd->MAX + bdd->entreParentheses(bdd->HISTORIQUE_EPISODE));
    conditions.append(bdd->HISTORIQUE_NOM + bdd->EGALE + bdd->entreQuotes(ui->pageHistoriqueModifierComboBoxRechercheNom->currentText()));
    conditions.append(bdd->HISTORIQUE_SAISON + bdd->EGALE + bdd->entreQuotes(methodeDiverses.formalismeEntier(arg1)));

    BaseDeDonnees::Retour retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer la derniere saison de la série " + ui->pageHistoriqueModifierComboBoxRechercheNom->currentText() + " dans l'historique";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    QList<QMap<QString,QString> > liste = retour.liste;
    ui->pageHistoriqueModifierSpinBoxRechercheEpisode->setMaximum(liste.at(0).value(bdd->MAX + bdd->entreParentheses(bdd->HISTORIQUE_EPISODE)).toInt());

    champs.clear();
    champs.append(bdd->MIN + bdd->entreParentheses(bdd->HISTORIQUE_EPISODE));

    retour = bdd->requeteSelect(champs, bdd->TABLE_HISTORIQUE, jointures, conditions, ordres);
    if(!retour.reussi) {
        QString messageLog = "Impossible de récupérer la première saison de la série " + ui->pageHistoriqueModifierComboBoxRechercheNom->currentText() + " dans l'historique";
        QMessageBox::critical(this, this->windowTitle(), messageLog);
        MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
        throw new QException();
    }
    liste = retour.liste;
    ui->pageHistoriqueModifierSpinBoxRechercheEpisode->setMinimum(liste.at(0).value("MIN(" + bdd->HISTORIQUE_EPISODE + ")").toInt());

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
            QStringList colonne = texte.split(";");
            if(!colonne.isEmpty()) {
                int ligne = ui->pageHistoriqueImporterTableWidget->rowCount();
                ui->pageHistoriqueImporterTableWidget->setRowCount(ui->pageHistoriqueImporterTableWidget->rowCount() + 1);
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 0, methodeDiverses.itemForTableWidget(colonne.at(0), false));
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 1, methodeDiverses.itemForTableWidget(methodeDiverses.formalismeEntier(colonne.at(1).toInt()), true));
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 2, methodeDiverses.itemForTableWidget(methodeDiverses.formalismeEntier(colonne.at(2).toInt()), true));
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 3, methodeDiverses.itemForTableWidget(methodeDiverses.jmaToDate(colonne.at(3)).toString("dd/MM/yyyy"), true));
                ui->pageHistoriqueImporterTableWidget->setItem(ligne, 4, methodeDiverses.itemForTableWidget(colonne.at(4), true));
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
        QString etat = ui->pageHistoriqueImporterTableWidget->item(i, 4)->text();

        QStringList champs, valeurs;

        champs.append(bdd->HISTORIQUE_NOM);
        valeurs.append(nom);
        champs.append(bdd->HISTORIQUE_SAISON);
        valeurs.append(saison);
        champs.append(bdd->HISTORIQUE_EPISODE);
        valeurs.append(episode);
        champs.append(bdd->HISTORIQUE_DATE_AJOUT);
        valeurs.append(methodeDiverses.jmaToDate(dateDiffusion).toString("yyyy-MM-dd"));
        champs.append(bdd->HISTORIQUE_ETAT);
        if(etat.toLower() == "vu") {
            valeurs.append("V");
        } else if(etat.toLower() == "téléchargé" || etat.toLower() == "telecharge") {
            valeurs.append("T");
        } else {
            valeurs.append("NV");
        }

        BaseDeDonnees::Retour retour = bdd->requeteInsert(champs, valeurs, bdd->TABLE_HISTORIQUE);
        if(!retour.reussi) {
            QString messageLog = "Impossible d'ajouter dans l'historique l'épisode " + nom + " S" + saison + "E" + episode;
            QMessageBox::critical(this, this->windowTitle(), messageLog);
            MethodeDiverses::ecrireLog("ERREUR - FenetrePrincipale::on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged() : " + messageLog + " " + retour.erreur + " requete : " + retour.requete);
            throw new QException();
        }
    }
    ui->statusBar->showMessage("Import effectué");
    ui->pageHistoriqueImporterLineEditCheminFichier->setText("");
    ui->pageHistoriqueImporterPushButtonChargerFichier->setEnabled(false);
    ui->pageHistoriqueImporterPushButtonImporter->setEnabled(false);
    ui->pageHistoriqueImporterTableWidget->setRowCount(0);
}
