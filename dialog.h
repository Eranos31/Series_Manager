#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QButtonGroup>
#include <QTimer>
#include <QFileIconProvider>
#include <QTreeWidget>
#include "fenetreprincipale.h"
#include "methodediverses.h"

class FenetrePrincipale;
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    bool event(QEvent *event);

    const int TREE_WIDGET_NOM =                 0;
    const int TREE_WIDGET_DATE_CREATION =       1;
    const int TREE_WIDGET_DATE_MODIFICATION =   2;
    const int TREE_WIDGET_TAILLE =              3;
    const int TREE_WIDGET_DEPLACER =            4;
    const int TREE_WIDGET_NE_RIEN_FAIRE =       5;
    const int TREE_WIDGET_SUPPRIMER =           6;


private slots:
    QString tailleFichier(double taille, QString unite);
    void on_pushButtonDeplacerTout_clicked();
    void on_pushButtonSupprimerTout_clicked();
    void on_pushButtonNeRienFaire_clicked();
    void modifierTouteCheckBox(QTreeWidgetItem *itemPere, int colonne);
    void on_pushButtonDeplacerFichier_clicked();
    void preTraitementDeplacerFichier(QTreeWidgetItem *itemPere, QStringList *listeFichierADeplacer, QStringList *listeFichierASupprimer);
    QString getChemin(QTreeWidgetItem *item);
    void on_pushButtonRetour_clicked();
    void deplacerFichier(QList<QString> liste);
    void supprimerFichier(QList<QString> liste);
    void majTreeWidget();
    void remplirTreeWidget(QTreeWidgetItem *itemPere, QString chemin);
    void verificationChangementDossierTelechargement();
    void nombreFichier(QString chemin);
    bool contenuIdentique();
    void activationBoutonDeplacer();
    bool verificationCheckBox(QTreeWidgetItem *itemPere);
    void supprimerDossierEtContenu(QString dossier);
    QList<QString> getListeSerie();

private:
    Ui::Dialog *ui;
    BaseDeDonnees *bdd;
    MethodeDiverses methodeDiverses;
    QFileInfoList listeInfoFichier;
    QList<QString> fichierCharge;
    QList<QString> fichierTrouve;
};

#endif // DIALOG_H
