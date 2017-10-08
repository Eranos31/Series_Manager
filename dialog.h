#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QButtonGroup>
#include <QTimer>
#include <QFileIconProvider>
#include "fenetreprincipale.h"
#include "log.h"
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

private slots:
    QString tailleFichier(double taille, QString unite);
    void on_pushButtonDeplacerTout_clicked();
    void on_pushButtonSupprimerTout_clicked();
    void on_pushButtonNeRienFaire_clicked();
    void on_pushButtonDeplacerFichier_clicked();
    void on_pushButtonRetour_clicked();
    void deplacerFichier(QList<QString> liste);
    void supprimerFichier(QList<QString> liste);
    void refresh(QString dossier);
    void on_tableWidget_clicked(const QModelIndex &index);
    void verificationChangementDossierTelechargement();
    void closeEvent(QCloseEvent *event);
    void nombreFichier(QString chemin);
    bool contenuIdentique();
    void activationBoutonDeplacer();
    void supprimerDossierEtContenu(QString dossier);
    QList<QString> getListeSerie();

private:
    Ui::Dialog *ui;
    FenetrePrincipale *pere;
    Log *log;
    MethodeDiverses methodeDiverses;
    QFileInfoList listeInfoFichier;
    QTimer *timer;
    QList<QString> fichierCharge;
    QList<QString> fichierTrouve;
};

#endif // DIALOG_H
