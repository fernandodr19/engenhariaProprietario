#ifndef SAVETHREAD_H
#define SAVETHREAD_H

#include <QThread>
#include <QDebug>
#include "logentry.h"


class SaveThread : public QThread
{

public:
    SaveThread(const QString& databasePath, const QMap<QString, LogEntry>& activeFiles);

    void saveActiveFilesCheckStatus();

    QString m_databasePath;
    QMap<QString, LogEntry> m_activeFiles;
protected:
    void run() {
        saveActiveFilesCheckStatus();
    }
};

#endif // SAVETHREAD_H
