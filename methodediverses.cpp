#include "methodediverses.h"
MethodeDiverses::MethodeDiverses()
{
}

QString MethodeDiverses::dayToString(const int date) {
    switch (date) {
    case 1 : return "Lundi";
    case 2 : return "Mardi";
    case 3 : return "Mercredi";
    case 4 : return "Jeudi";
    case 5 : return "Vendredi";
    case 6 : return "Samedi";
    case 7 : return "Dimanche";
    default : return NULL;
    }
}

int MethodeDiverses::dayToInt(const QString date) {
    if(date == "Lundi") {
        return 1;
    } else if (date == "Mardi") {
        return 2;
    } else if (date == "Mercredi") {
        return 3;
    } else if (date == "Jeudi") {
        return 4;
    } else if (date == "Vendredi") {
        return 5;
    } else if (date == "Samedi") {
        return 6;
    } else {
        return 7;
    }
}

int MethodeDiverses::stringToInt(const QString date) {
    if(date == "1") {
        return 0;
    } else if(date == "2") {
        return 1;
    } else if(date == "3") {
        return 2;
    } else if(date == "4") {
        return 3;
    } else if(date == "5") {
        return 4;
    } else if(date == "6") {
        return 5;
    } else if(date == "7") {
        return 6;
    }
}

bool MethodeDiverses::msgBoxQuestion(const QString text) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(text);
    msgBox.addButton("Oui", QMessageBox::YesRole);
    msgBox.addButton("Non", QMessageBox::NoRole);
    return (msgBox.exec());
}

QTableWidgetItem* MethodeDiverses::itemForTableWidget(const QString text, bool center) {
    QTableWidgetItem *item = new QTableWidgetItem(text);
    if(center) {
        item->setTextAlignment(Qt::AlignCenter);
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

QString MethodeDiverses::dateToString(QDate date) {
    QString jour;
    QString mois;
    if(date.day() >= 1 && date.day() <=9) {
        jour = "0" + QString::number(date.day());
    } else {
        jour = QString::number(date.day());
    }
    if(date.month() >= 1 && date.month() <= 9) {
        mois = "0" + QString::number(date.month());
    } else {
        mois = QString::number(date.month());
    }
    return QString::number(date.year()) + "-" + mois + "-" +  jour;
}

QDate MethodeDiverses::stringToDate(QString text) {
    if(text == "0" || text == "") {
        return QDate();
    } else {
        QStringList liste = text.split("-");
        return QDate(liste.at(0).toInt(), liste.at(1).toInt(), liste.at(2).toInt());
    }
}

QDate MethodeDiverses::jmaToDate(QString text) {
    QList<QString> liste = text.split("/");
    return QDate(liste.at(2).toInt(), liste.at(1).toInt(), liste.at(0).toInt());
}

QString MethodeDiverses::formalismeEntier(int entier) {
    if(entier >= 0 && entier < 10) {
        return ("0" + QString::number(entier));
    } else {
        return (QString::number(entier));
    }
}

QString MethodeDiverses::formatDate(QDate date) {
    return dayToString(date.dayOfWeek()) + " " + QString::number(date.day()) + " " + monthToString(date.month());
}

QString MethodeDiverses::formatDateJJMMAA(QDate date) {
    return QString::number(date.day()) + "/" + QString::number(date.month()) + "/" + QString::number(date.year());
}

QString MethodeDiverses::monthToString(int mois) {
    switch(mois) {
    case 1 : return "Janvier";
    case 2 : return "Fevrier";
    case 3 : return "Mars";
    case 4 : return "Avril";
    case 5 : return "Mai";
    case 6 : return "Juin";
    case 7 : return "Juillet";
    case 8 : return "Aout";
    case 9 : return "Septembre";
    case 10 : return "Octobre";
    case 11 : return "Novembre";
    case 12 : return "Decembre";
    }
}

void MethodeDiverses::ecrireLog(QString message) {
#ifdef QT_DEBUG
    QFile log(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs/" + "SeriesManager_" + QDate::currentDate().toString("yyyy_MM_dd") + ".log");
#else
    QFile log(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs/" + "SeriesManager_" + QDate::currentDate().toString("yyyy_MM_dd") + ".log");
#endif
    bool nouveau = false;
    if(!log.exists()) {
        nouveau = true;
    }
    if(log.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)) {
        QTextStream flux(&log);
        flux.setCodec("UTF-8");
        if(nouveau) {
            flux << "---------------------------------------------" << endl;
            flux << "-                                           -" << endl;
            flux << "-  #### ##### ####   ###  #####  ####       -" << endl;
            flux << "- #     #     #   #   #   #     #           -" << endl;
            flux << "- #     #     #   #   #   #     #           -" << endl;
            flux << "-  ###  ####  ####    #   ####   ###        -" << endl;
            flux << "-     # #     # #     #   #         #       -" << endl;
            flux << "-     # #     #  #    #   #         #       -" << endl;
            flux << "- ####  ##### #   #  ###  ##### ####        -" << endl;
            flux << "-                                           -" << endl;
            flux << "- #   #  ###  #   #  ###   ###  ##### ####  -" << endl;
            flux << "- ## ## #   # #   # #   # #   # #     #   # -" << endl;
            flux << "- # # # #   # ##  # #   # #     #     #   # -" << endl;
            flux << "- # # # ##### # # # ##### #     ####  ####  -" << endl;
            flux << "- #   # #   # #  ## #   # #  ## #     # #   -" << endl;
            flux << "- #   # #   # #   # #   # #   # #     #  #  -" << endl;
            flux << "- #   # #   # #   # #   #  ###  ##### #   # -" << endl;
            flux << "-                                           -" << endl;
            flux << "---------------------------------------------" << endl << "" << endl;
            supprimerAncienFichier();
        }
        flux << "[" << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss:zzz") << "] " << message << endl;
    }
}

void MethodeDiverses::supprimerAncienFichier() {
    QStringList liste;
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    int max = settings.value("Configuration/PurgeLog", 7).toInt();
    for(int i = 0; i <= max; i++) {
        liste.append("SeriesManager_" + QDate::currentDate().addDays(-i).toString("yyyy_MM_dd") + ".log");
    }

    foreach (QFileInfo info, QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs/").entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries)) {
        if(info.fileName().contains("SeriesManager_") && !liste.contains(info.fileName())) {
            QFile(info.absoluteFilePath()).remove();
        }
    }
}

QString MethodeDiverses::getConfig(QString config) {
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    return settings.value(config).toString();
}

int MethodeDiverses::getConfig(QString config, int valeur) {
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    return settings.value(config, valeur).toInt();
}

void MethodeDiverses::setConfig(QString config, QString valeur) {
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    settings.setValue(config, valeur);
}

