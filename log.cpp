#include "log.h"

Log::Log() {
    nomFichier = "SeriesManager";
    Log("SeriesManager");
}

Log::Log(QString nom) {
    nomFichier = nom;
    QFile file("Log/" + nomFichier + ".log");
    if(!file.exists()){
        if(file.open(QIODevice::WriteOnly|QIODevice::Text)){
            QTextStream flux(&file);
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
}

void Log::ecrire(QString text) {
    QFile file("Log/" + nomFichier + ".log");
    if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){
        QTextStream flux(&file);
        QDateTime date = QDateTime::currentDateTime();
        flux << "[" << date.toString() << "] " << text << endl;
    }
}
