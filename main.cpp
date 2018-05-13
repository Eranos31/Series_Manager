#include "fenetreprincipale.h"
#include <QApplication>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <winuser.h>

QMap<int, QString> mapProcess;

// Vérifie si le process existe déjà
bool processExiste(QString nomProcess) {
    unsigned long aProcesses[1024], cbNeeded, cProcesses;

    if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        printf("text01");

    cProcesses = cbNeeded / sizeof(unsigned long);
    int count = 0;

    for(unsigned int i = 0; i < cProcesses; i++) {
        if(aProcesses[i] == 0)
            continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, aProcesses[i]);

        wchar_t buffer[50];

        GetModuleBaseName(hProcess, 0, buffer, 50);

        CloseHandle(hProcess);

        mapProcess[aProcesses[i]] = QString::fromWCharArray(buffer);

        if(QString::fromWCharArray(buffer).contains(nomProcess)) {
            count++;
        }
    }

    return (count >= 2);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString nomProcess = QFileInfo(argv[0]).fileName();
    FenetrePrincipale w;
    if(!processExiste(nomProcess)) {
        w.show();
    } else {
        // Pour chaque PID des process du même nom
        foreach (int pid, mapProcess.keys(nomProcess)) {
            HWND handle = NULL;
            // Pour toutes les fenêtre ouverte
            do {
                handle = FindWindowEx(NULL, handle, NULL, NULL);
                DWORD dwPID = 0;
                GetWindowThreadProcessId(handle, &dwPID);
                // Si le PID correspond à un de ceux du même nom
                if(pid == dwPID)
                    // On le met au premier plan
                    SetForegroundWindow(handle);
            } while (handle != NULL);
        }
        return 1;
    }

    return a.exec();
}


