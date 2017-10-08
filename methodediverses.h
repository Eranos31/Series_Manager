#ifndef METHODEDIVERSES_H
#define METHODEDIVERSES_H
#include <QString>
#include <QDate>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QHBoxLayout>

class MethodeDiverses
{
public:
    MethodeDiverses();
    QString dayToString(const int date);
    int dayToInt(const QString date);
    int stringToInt(const QString date);
    bool msgBoxQuestion(const QString text);
    QTableWidgetItem* itemForTableWidget(const QString text, bool center);
    QString dateToString(QDate date);
    QDate stringToDate(QString text);
    QDate jmaToDate(QString text);
    QString formalismeEntier(int entier);
    QString formatDate(QDate date);
    QString formatDateJJMMAA(QDate date);
    QString monthToString(int mois);
};

#endif // METHODEDIVERSES_H
