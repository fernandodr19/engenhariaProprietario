#ifndef SAVETHREAD_H
#define SAVETHREAD_H

#include <QThread>
#include <QDebug>
#include "logentry.h"
#include "mainwindow.h"


class SaveThread : public QThread
{

public:
    SaveThread(const QString& databasePath, const QString& fileName, bool checked);
    SaveThread(const QString& databasePath, const QString& fileName, const QString &person);
    SaveThread(const QString& databasePath, const QStringList& employes);

    void saveActiveFilesStatus();

    QString m_databasePath;
    QString m_fileName;
    bool m_downloaded;
    QString m_person;
    column m_col;
    QStringList m_employees;
    bool m_updateEmployees = false;
protected:
    void run() { saveActiveFilesStatus(); }
};

#endif // SAVETHREAD_H
