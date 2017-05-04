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
    void on_bouton_t411_clicked(QString nom);
    void on_bouton_dossier_clicked(QString nom);
    void on_bouton_wiki_clicked(QString nom);
    void on_bouton_reporter_clicked(QString nom);
    void majIndicateur();
    void colorisation();
    void chargementConfiguration();
    void majEpisode();
    void verificationSerieTerminer();
    void nettoyerDossierTelechargement();
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
    // PAGE CONFIGURATION
    void on_pageConfigurationBoutonAccueil_clicked();
    void on_pageConfigurationBoutonParcourir_clicked();
    void on_pageConfigurationBoutonTerminer_clicked();
    // PAGE PRINCIPALE
    void on_pagePrincipaleTableWidgetDisplay_doubleClicked(const QModelIndex &index);
    void on_pagePrincipaleTableWidgetDisplay_2_doubleClicked(const QModelIndex &index);
    void on_pagePrincipaleBoutonActualiser_clicked();
    void on_pagePrincipaleBoutonVosSeries_clicked();
    void on_pagePrincipaleBoutonAjouter_clicked();
    void on_pagePrincipaleBoutonModifier_clicked();
    void on_pagePrincipaleBoutonSupprimer_clicked();
    void on_pagePrincipaleBoutonUrl_clicked();
    void on_pagePrincipaleBoutonDossierSerie_clicked();
    void on_pagePrincipaleBoutonWiki_clicked();
    void on_pagePrincipaleBoutonDeplacerFichier_clicked();
    // PAGE TOUTES LES SERIES
    void on_pageVosSeriesBoutonAccueil_clicked();
    void on_pageVosSeriesComboBox_currentIndexChanged(const QString &arg1);
    // PAGE AJOUT MODIFICATION
    void on_pageAjoutModifBoutonAccueil_clicked();
    void on_pageAjoutModifLineDateSortie_userDateChanged(const QDate &date);
    void on_pageAjoutModifButtonWiki_clicked();
    void on_pageAjoutModifBoutonValider_clicked();
    void on_pageAjoutModifBoutonDossier_clicked();
    void on_pageAjoutModifBoutonRetour_clicked();
    void on_pageAjoutModifBoutonWiki_clicked();
    void on_pageAjoutModifLineDateProchain_userDateChanged(const QDate &date);
    // PAGE LISTE MODIFICATION
    void on_pageListeModificationBoutonAccueil_clicked();
    void on_pageListeModificationBoutonModifier_clicked();
    void on_pageListeModificationBoutonRetour_clicked();
    void on_pageListeModificationDisplay_doubleClicked(const QModelIndex &index);
    // PAGE LISTE SUPPRESSION
    void on_pageListeSupprimerBoutonAccueil_clicked();
    void on_pageListeSupprimerBoutonSupprimer_clicked();
    void on_pageListeSupprimerBoutonRetour_clicked();
    void on_pageListeSupprimerDisplay_doubleClicked(const QModelIndex &index);
    // PAGE REPORTER
    void on_pageReporterBoutonAccueil_clicked();
    void on_pageReporterButtonValider_clicked();
    void on_pageReporterButtonRetour_clicked();
    void on_pageReporterSpinBox_valueChanged(int nbSemaines);
    // PAGE HISTORIQUE
    void on_pageHistoriqueBoutonAccueil_clicked();

private:
    Ui::FenetrePrincipale *ui;
    BaseDeDonnees *bdd;
    MethodeDiverses methodeDiverses;
    QList<QMap<QString, QString> > listeQuoti;
    QList<QMap<QString, QString> > listeGlobal;
    QList<QString> listeSerie;
    QString dossierSerie;
    Log* log;
    // IMAGE
    QPixmap i_seriesManager = QPixmap(":/images/SeriesManager.png");
    QIcon i_ajouter = QIcon(":/images/ajouter.png");//
    QIcon i_valider = QIcon(":/images/valider.png");
    QIcon i_modifier = QIcon(":/images/modifier.png");
    QIcon i_pellicule = QIcon(":/images/pellicule.gif");
    QIcon i_accueil = QIcon(":/images/accueil.png");//
    QIcon i_annuler = QIcon(":/images/annuler.png");//
    QIcon i_t411 = QIcon(":/images/t411.png");
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
