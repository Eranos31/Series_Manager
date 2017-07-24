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
#include "basededonnees.h"
#include "methodediverses.h"
#include "log.h"

namespace Ui {
class FenetrePrincipale;
}

class FenetrePrincipale : public QMainWindow
{
    Q_OBJECT

public:
    explicit FenetrePrincipale(QWidget *parent = 0);
    ~FenetrePrincipale();

private slots:
    void premiereConnexion();
    void refresh();
    void initialisation();
    void on_bouton_lien_clicked(QString nom);
    void on_bouton_dossier_clicked(QString nom);
    void on_bouton_reporter_clicked(QString nom);
    void majIndicateur();
    void chargementConfiguration();
    void majEpisode();
    void verificationSerieTerminer();
    void nettoyerDossierTelechargement();
    QString getConfig(QString config);
    int getConfig(QString config, int valeur);
    void setConfig(QString config, QString valeur);
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    // MENU
    void on_menuFichierAccueil_triggered();
    void on_menuFichierHistorique_triggered();
    void on_menuFichierToutes_les_s_ries_triggered();
    void on_menuFichierAjouter_triggered();
    void on_menuFichierModifier_triggered();
    void on_menuFichierSupprimer_triggered();
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
    void on_toolBarSupprimer_triggered();
    void on_toolBarDeplcerFichiers_triggered();
    // PAGE CONFIGURATION
    void on_pageConfigurationBoutonParcourir_clicked();
    void on_pageConfigurationBoutonParcourir_2_clicked();
    void on_pageConfigurationBoutonTerminer_clicked();
    // PAGE PRINCIPALE
    void on_pagePrincipaleTableWidgetDisplay_doubleClicked(const QModelIndex &index);
    void on_pagePrincipaleTableWidgetDisplay_2_doubleClicked(const QModelIndex &index);
    void on_pagePrincipaleBoutonVosSeries_clicked();
    void on_pagePrincipaleBoutonAjouter_clicked();
    void on_pagePrincipaleBoutonModifier_clicked();
    void on_pagePrincipaleBoutonSupprimer_clicked();
    void on_pagePrincipaleBoutonUrl_clicked();
    void on_pagePrincipaleBoutonDossierSerie_clicked();
    void on_pagePrincipaleBoutonWiki_clicked();
    void on_pagePrincipaleBoutonDeplacerFichier_clicked();
    // PAGE TOUTES LES SERIES
    void on_pageVosSeriesDisplay_doubleClicked(const QModelIndex &index);
    void on_pageVosSeriesComboBox_currentIndexChanged(const QString &arg1);
    // PAGE AJOUT MODIFICATION
    void on_pageAjoutModifLineDateSortie_userDateChanged(const QDate &date);
    void on_pageAjoutModifButtonWiki_clicked();
    void on_pageAjoutModifBoutonValider_clicked();
    void on_pageAjoutModifBoutonDossier_clicked();
    void on_pageAjoutModifBoutonRetour_clicked();
    void on_pageAjoutModifBoutonWiki_clicked();
    void on_pageAjoutModifLineDateProchain_userDateChanged(const QDate &date);
    // PAGE LISTE MODIFICATION
    void on_pageListeModificationBoutonModifier_clicked();
    void on_pageListeModificationBoutonRetour_clicked();
    void on_pageListeModificationDisplay_doubleClicked(const QModelIndex &index);
    // PAGE LISTE SUPPRESSION
    void on_pageListeSupprimerBoutonSupprimer_clicked();
    void on_pageListeSupprimerBoutonRetour_clicked();
    void on_pageListeSupprimerDisplay_doubleClicked(const QModelIndex &index);
    // PAGE REPORTER
    void on_pageReporterButtonValider_clicked();
    void on_pageReporterButtonRetour_clicked();
    void on_pageReporterSpinBox_valueChanged(int nbSemaines);

private:
    Ui::FenetrePrincipale *ui;
    BaseDeDonnees *bdd;
    MethodeDiverses methodeDiverses;
    QList<QMap<QString, QString> > listeQuoti;
    QList<QMap<QString, QString> > listeGlobal;
    QList<QString> listeSerie;
    QString dossierSerie;
    Log* log;
    QString version;
    QDate dateVersion;
    QString heureVersion;
    QString lienParDefaut;
    // IMAGE
    QPixmap i_seriesManager = QPixmap(":/images/SeriesManager.png");
    QIcon i_ajouter = QIcon(":/images/ajouter.png");//
    QIcon i_valider = QIcon(":/images/valider.png");
    QIcon i_modifier = QIcon(":/images/modifier.png");
    QIcon i_pellicule = QIcon(":/images/pellicule.gif");
    QIcon i_accueil = QIcon(":/images/accueil.png");//
    QIcon i_annuler = QIcon(":/images/annuler.png");//
    QIcon i_logo = QIcon(":/images/logo.ico");
    QIcon i_reporter = QIcon(":/images/reporter.png");
    QIcon i_retour = QIcon(":/images/retour.png");
    QIcon i_dossier = QIcon(":/images/dossier.png");//
    QIcon i_actualiser = QIcon(":/images/actualiser.png");//
    QIcon i_engrenage = QIcon(":/images/engrenage.png");//
    QIcon i_interrogation = QIcon(":/images/interrogation.gif");
    QIcon i_wiki = QIcon(":/images/wiki.png");
    QIcon i_exit = QIcon(":/images/exit.png");//
    QPixmap i_feuRouge = QPixmap(":/images/FeuRouge.ico");
    QPixmap i_feuVert = QPixmap(":/images/FeuVert.ico");
    QIcon i_deplacement = QIcon(":/images/deplacement.png");//
};

#endif // FENETREPRINCIPALE_H
