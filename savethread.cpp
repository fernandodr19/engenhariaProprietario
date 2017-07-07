#include "savethread.h"
#include <QDebug>

SaveThread::SaveThread(const QString& databasePath, const QMap<QString, LogEntry>& activeFiles)
{
    m_databasePath = databasePath;
    m_activeFiles = activeFiles;
}

void SaveThread::saveActiveFilesCheckStatus()
{
    QSettings settings(m_databasePath, QSettings::IniFormat);
    settings.clear();
    settings.beginWriteArray("activeFiles");
    int i = 0;
    for(auto logEntry = m_activeFiles.begin(); logEntry != m_activeFiles.end(); logEntry++) {
        bool done = logEntry.value().feito;
        bool downloaded = logEntry.value().downloaded;
        if(done || downloaded) {
            settings.setArrayIndex(i++);
            settings.setValue("fileName", logEntry.value().nome);
            if(done)
                settings.setValue("done", true);
            if(downloaded)
                settings.setValue("downloaded", true);
        }
    }
    settings.endArray();
}
