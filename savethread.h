#ifndef SAVETHREAD_H
#define SAVETHREAD_H

#include <QThread>
#include <QDebug>
#include "logentry.h"
#include "mainwindow.h"


class SaveThread : public QThread
{

public:
    SaveThread(const QString& databasePath, const QString& fileName, bool checked, column col);

    void saveActiveFilesCheckStatus();

    QString m_databasePath;
    QString m_fileName;
    bool m_checked;
    column m_col;
protected:
    void run() { saveActiveFilesCheckStatus(); }
};

#endif // SAVETHREAD_H
