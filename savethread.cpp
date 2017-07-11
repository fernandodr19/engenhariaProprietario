#include "savethread.h"

SaveThread::SaveThread(const QString& databasePath, const QString& fileName, bool checked, column col) :
    m_databasePath(databasePath), m_fileName(fileName), m_checked(checked), m_col(col)
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void SaveThread::saveActiveFilesCheckStatus()
{
    QSettings settings(m_databasePath, QSettings::IniFormat);
    settings.beginGroup(m_fileName);
    if(m_col == col_Forwarded) {
        if(m_checked)
            settings.setValue("forwarded", true);
        else
            settings.remove("forwarded");
    } else {
        if(m_checked)
            settings.setValue("downloaded", true);
        else
            settings.remove("downloaded");
    }
    settings.endGroup();
}
