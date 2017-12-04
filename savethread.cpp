#include "savethread.h"

SaveThread::SaveThread(const QString& databasePath, const QString& fileName) :
    m_databasePath(databasePath), m_fileName(fileName)
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

SaveThread::SaveThread(const QString& databasePath, const QString& fileName, const QString& person) :
    m_databasePath(databasePath), m_fileName(fileName), m_person(person)
{
    m_col = col_Forwarded;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

SaveThread::SaveThread(const QString& databasePath, const QStringList &employes) :
    m_databasePath(databasePath), m_employees(employes)
{
    m_updateEmployees = true;
}

void SaveThread::setCommented(bool commented)
{
    m_col = col_Commented;
    m_commented = commented;
}

void SaveThread::setDownloaded(bool downloaded)
{
    m_col = col_Downloaded;
    m_downloaded = downloaded;
}

void SaveThread::saveActiveFilesStatus()
{
    QSettings settings(m_databasePath, QSettings::IniFormat);
    if(m_updateEmployees) {
        settings.beginWriteArray("employees");
        for(int i = 0; i < m_employees.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("employee", m_employees[i]);
        }
        settings.endArray();
    } else {
        settings.beginGroup(m_fileName);
        if(m_col == col_Forwarded) {
            if(!m_person.isEmpty())
                settings.setValue("forwarded", m_person);
            else
                settings.remove("forwarded");
        } else if(m_col == col_Downloaded) {
            if(m_downloaded)
                settings.setValue("downloaded", true);
            else
                settings.remove("downloaded");
        } else if (m_col == col_Commented){
            if(m_commented)
                settings.setValue("commented", true);
            else
                settings.remove("commented");
        }
        settings.endGroup();
    }
}
