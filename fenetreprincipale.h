#ifndef FENETREPRINCIPALE_H
#define FENETREPRINCIPALE_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcess>
#include <QFileDialog>
#include <QInputDialog>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCloseEvent>
#include <QClipboard>
#include <QStandardPaths>
#include <QScrollBar>
#include "basededonnees.h"
#include "methodediverses.h"
#include "log.h"
#include "dialog.h"
#include "filedownloader.h"

namespace Ui {
class FenetrePrincipale;
}

class FenetrePrincipale : public QMainWindow
{
    Q_OBJECT

public:
    explicit FenetrePrincipale(QWidget *parent = 0);
    ~FenetrePrincipale();
    QString getConfig(QString config);
    int getConfig(QString config, int valeur);
    void setConfig(QString config, QString valeur);
    QString getDossierSerie();

private slots:
    void premiereConnexion();
    void refresh();
    void initialisation();
    void bouton_lien_clicked(QString nom);
    void bouton_dossier_clicked(QString nom);
    void bouton_reporter_clicked(QString nom);
    void checkBox_vu_clicked(QString nom);
    void majIndicateur();
    void chargementConfiguration();
    void majEpisode();
    void verificationSerieTerminer();
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *);
    void redimensionnerTableau();
    void loadImage();
    // MENU
    void on_menuFichierAccueil_triggered();
    void on_menuFichierHistoriqueRechercher_triggered();
    void on_menuFichierHistoriqueAjouter_triggered();
    void on_menuFichierHistoriqueModifier_triggered();
    void on_menuFichierHistoriqueConsulter_triggered();
    void on_menuFichierHistoriqueImporter_triggered();
    void on_menuFichierToutes_les_s_ries_triggered();
    void on_menuFichierAjouter_triggered();
    void on_menuFichierModifier_triggered();
    void on_menuFichierQuitter_triggered();
    void on_menuOptionsActualiser_triggered();
    void on_menuOptionsParam_tres_triggered();
    void on_menuOptionsDossier_de_log_triggered();
    void on_menuAideA_Propos_triggered();
    // TOOLBAR
    void on_toolBarAccueil_triggered();
    void on_toolBarParam_tres_triggered();
    void on_toolBarActualiser_triggered();
    void on_toolBarQuitter_triggered();
    void on_toolBarAjouter_triggered();
    void on_toolBarModifier_triggered();
    void on_toolBarDeplcerFichiers_triggered();
    // PAGE CONFIGURATION
    void on_pageConfigurationBoutonParcourir_clicked();
    void on_pageConfigurationBoutonParcourir_2_clicked();
    void on_pageConfigurationBoutonTerminer_clicked();
    // PAGE PRINCIPALE
    void on_pagePrincipaleTableWidgetDisplay_clicked(const QModelIndex &index);
    void on_pagePrincipaleTableWidgetDisplay_doubleClicked(const QModelIndex &index);
    void on_pagePrincipaleTableWidgetDisplay_2_doubleClicked(const QModelIndex &index);
    void on_pagePrincipaleTableWidgetDisplay_customContextMenuRequested(const QPoint &pos);
    void on_pagePrincipaleTableWidgetDisplay_2_customContextMenuRequested(const QPoint &pos);
    void on_pagePrincipaleBoutonVosSeries_clicked();
    void on_pagePrincipaleBoutonAjouter_clicked();
    void on_pagePrincipaleBoutonModifier_clicked();
    void on_pagePrincipaleBoutonLienEtAddicted_clicked();
    void on_pagePrincipaleBoutonDossierSerie_clicked();
    void on_pagePrincipaleBoutonWiki_clicked();
    void on_pagePrincipaleBoutonDeplacerFichier_clicked();
    // PAGE TOUTES LES SERIES
    void on_pageVosSeriesDisplay_doubleClicked(const QModelIndex &index);
    void on_pageVosSeriesComboBox_currentIndexChanged(const QString &arg1);
    // PAGE AJOUT MODIFICATION
    void on_pageAjoutModifButtonWiki_clicked();
    void on_pageAjoutModifBoutonValider_clicked();
    void on_pageAjoutModifBoutonDossier_clicked();
    void on_pageAjoutModifBoutonRetour_clicked();
    void on_pageAjoutModifBoutonWiki_clicked();
    void on_pageAjoutModifPushButtonSupprimerSaison_clicked();
    void on_pageAjoutModifComboBoxListeSaison_currentTextChanged(const QString &arg1);
    void on_pageAjoutModifComboFicheSerieNom_currentTextChanged(const QString &arg1);
    void on_pageAjoutModifCheckBoxCreerSaison_stateChanged(int arg1);
    void on_pageAjoutModifDateEditSaisonDateSortie_dateChanged(const QDate &date);
    void on_pageAjoutModifDateEditSaisonDateProchain_dateChanged(const QDate &date);
    void on_pageAjoutModifPushButtonParcourirPhoto_clicked();
    void on_pageAjoutModifPushButtonTelecharger_clicked();
    void on_pageAjoutModifLineEditDepuisURL_textChanged(const QString &arg1);
    void on_pageAjoutModifCheckBoxDoubleEpisode_stateChanged(int arg1);
    // PAGE REPORTER
    void on_pageReporterButtonValider_clicked();
    void on_pageReporterButtonRetour_clicked();
    void on_pageReporterSpinBox_valueChanged(int nbSemaines);
    // PAGE HISTORIQUE RECHERCHER
    void on_pageHistoriqueRechercherPushButtonAjoutNomSerie_clicked();
    void on_pageHistoriqueRechercherPushButtonSupprimerNomSerie_clicked();
    void on_pageHistoriqueRechercherPushButtonRechercher_clicked();
    void on_pageHistoriqueRechercherPushButtonSupprimerTout_clicked();
    // PAGE HISTORIQUE AJOUTER
    void on_pageHistoriqueAjouterPushButtonAjouterLigne_clicked();
    void on_pageHistoriqueAjouterPushButtonSupprimerLigne_clicked();
    void on_pageHistoriqueAjouterPushButtonAjouter_clicked();
    // PAGE HISTORIQUE MODIFIER
    void on_pageHistoriqueModifierPushButtonRechercher_clicked();
    void on_pageHistoriqueModifierPushButtonModifier_clicked();
    void on_pageHistoriqueModifierComboBoxRechercheNom_currentTextChanged(const QString &arg1);
    void on_pageHistoriqueModifierSpinBoxRechercheSaison_valueChanged(int arg1);
    // PAGE HISTORIQUE CONSULTER
    void on_pageHistoriqueTableWidget_customContextMenuRequested(const QPoint &pos);
    void on_pageHistoriqueTableWidget_clicked(const QModelIndex &index);
    // PAGE HISTORIQUE IMPORTER
    void on_pageHistoriqueImporterPushButtonParcourir_clicked();
    void on_pageHistoriqueImporterPushButtonChargerFichier_clicked();
    void on_pageHistoriqueImporterPushButtonImporter_clicked();




    void on_pagePrincipaleTableWidgetDisplay_2_clicked(const QModelIndex &index);

private:
    Ui::FenetrePrincipale *ui;
    BaseDeDonnees *bdd;
    MethodeDiverses methodeDiverses;
    QList<QMap<QString, QString> > listeQuoti;
    QList<QMap<QString, QString> > listeGlobal;
    QList<QString> ficheSerie;
    QString dossierSerie;
    Log* log;
    QString version;
    QDate dateVersion;
    QTime heureVersion;
    QString lien;
    FileDownloader *m_pImgCtrl;

    // IMAGE
    QPixmap i_seriesManager = QPixmap(":/images/SeriesManager.png");
    QIcon i_ajouter = QIcon(":/images/ajouter.png");
    QIcon i_valider = QIcon(":/images/valider.png");
    QIcon i_modifier = QIcon(":/images/modifier.png");
    QIcon i_pellicule = QIcon(":/images/pellicule.gif");
    QIcon i_accueil = QIcon(":/images/accueil.png");
    QIcon i_annuler = QIcon(":/images/annuler.png");
    QIcon i_torrent = QIcon(":/images/torrent.png");
    QPixmap pix_torrent = QPixmap(":/images/torrent.png");
    QIcon i_addicted = QIcon(":/images/addic7ed.png");
    QPixmap pix_addicted = QPixmap(":/images/addic7ed.png");
    QIcon i_reporter = QIcon(":/images/reporter.png");
    QIcon i_retour = QIcon(":/images/retour.png");
    QIcon i_dossier = QIcon(":/images/dossier.png");
    QIcon i_actualiser = QIcon(":/images/actualiser.png");
    QIcon i_engrenage = QIcon(":/images/engrenage.png");
    QIcon i_interrogation = QIcon(":/images/interrogation.gif");
    QIcon i_wiki = QIcon(":/images/wiki.png");
    QIcon i_exit = QIcon(":/images/exit.png");
    QPixmap i_feuRouge = QPixmap(":/images/FeuRouge.ico");
    QPixmap i_feuVert = QPixmap(":/images/FeuVert.ico");
    QIcon i_deplacement = QIcon(":/images/deplacement.png");
    QIcon i_historique = QIcon(":/images/sablier.png");



};

#endif // FENETREPRINCIPALE_H
