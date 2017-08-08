#include "log.h"

Log::Log() {
    nomFichier = "SeriesManager";
    Log("SeriesManager");
}

Log::Log(QString nom) {
    nomFichier = nom;
#ifdef QT_DEBUG
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs/" + nomFichier + "_" + QDate::currentDate().toString("yyyy_MM_dd") + ".log");
#else
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs/" + nomFichier + "_" + QDate::currentDate().toString("yyyy_MM_dd") + ".log");
#endif
    if(!file.exists()){
        if(file.open(QIODevice::WriteOnly|QIODevice::Text)){
            QTextStream flux(&file);
            flux.setCodec("UTF-8");
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
        }
    }
    supprimerAncienFichier();
}

void Log::ecrire(QString text) {
    bool nouveau = false;
#ifdef QT_DEBUG
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/logs/" + nomFichier + "_" + QDate::currentDate().toString("yyyy_MM_dd") + ".log");
#else
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs/" + nomFichier + "_" + QDate::currentDate().toString("yyyy_MM_dd") + ".log");
#endif

    if(!file.exists()) {
        nouveau = true;
    }

    if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){
        QTextStream flux(&file);
        flux.setCodec("UTF-8");
        QDateTime date = QDateTime::currentDateTime();
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
        flux << "[" << date.toString() << "] " << text << endl;
    }
}

void Log::supprimerAncienFichier() {
    QStringList liste;
#ifdef QT_DEBUG
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/debug/config.ini", QSettings::IniFormat);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.ini", QSettings::IniFormat);
#endif
    int max = settings.value("Configuration/PurgeLog", 7).toInt();
    for(int i = 0; i <= max; i++) {
        liste.append(nomFichier + "_" + QDate::currentDate().addDays(-i).toString("yyyy_MM_dd") + ".log");
    }

    foreach (QFileInfo info, QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs/").entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries)) {
        if(info.fileName().contains(nomFichier) && !liste.contains(info.fileName())) {
            QFile(info.absoluteFilePath()).remove();
        }
    }
}
