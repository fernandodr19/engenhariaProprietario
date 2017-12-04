#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>
#include <QMap>
#include "logentry.h"

class Database
{
public:
    Database();

    void load();
    void save();

    QStringList getReadDates() { return m_readDatesList; }
    QString getFilesPath() { return m_filesPath; }
    bool getHistoricFilter() { return m_historicFilter; }
    bool getReleasedFilter() { return m_releasedFilter; }
    bool getApprovedFilter() { return m_approvedFilter; }
    bool getApprovedWithCommentsFilter() { return m_approvedWithCommentsFilter; }
    bool getReprovedFilter() { return m_reprovedFilter; }
    bool getMovedFilter() { return m_movedFilter; }
    QStringList getUndesirablePaths() { return m_undesirablePaths; }
    QStringList getHeadersOrder() { return m_headersOrder; }
    QVector<bool> getShowColumns() { return m_showColumns; }
    const QMap<QString, LogEntry>& getActiveFiles() { return m_activeFiles; }
    const QVector<LogEntry>& getHistoricFiles() { return m_historicFiles; }
    QStringList getEmployees();
    bool getPdfFilterIsEnabled() { return m_filterTypePdf; }

    void setFilesPath(const QString& path) { m_filesPath = path; }
    void setHistoricFilter(bool historicFilter) { m_historicFilter = historicFilter; }
    void setReleasedFilter(bool releasedFilter) { m_releasedFilter = releasedFilter; }
    void setApprovedFilter(bool approvedFitler) { m_approvedFilter = approvedFitler; }
    void setApprovedWithCommentsFilter(bool approvedWithComments) { m_approvedWithCommentsFilter = approvedWithComments; }
    void setReprovedFilter(bool reprovedFilter) { m_reprovedFilter = reprovedFilter; }
    void setMovedFilter(bool movedFilter) { m_movedFilter = movedFilter; }
    void setHeadersOrder(const QStringList& headersOrder) { m_headersOrder = headersOrder; }
    void setShowColumns(const QVector<bool>& showColumns) { m_showColumns = showColumns; }
    void setPdfFilterIsEnabled(bool filterTypePdf) { m_filterTypePdf = filterTypePdf; }

    void updateUndesirablePaths(const QString &path, bool desirable);
    void clearUndesirablePaths() { m_undesirablePaths.clear(); }

    void reloadLogEntries();
    void loadLogEntriesFromFile();
    void updateDownloaded(const QString& file, bool downloaded);
    void updateCommented(const QString& file, bool commented);
    void updateForwarded(const QString& file, const QString &person);
    void updateEmployees(const QStringList& employees);
private:
    void loadActiveFilesState();
    void saveActiveFilesCheckStatus();
    void createFilesFromLogEntries();
    void updateFiles();
    QStringList getLogFiles();
    QString getDate(QString fileName);

    //FILTERS
    bool m_historicFilter; //historico/atual
    bool m_releasedFilter;
    bool m_approvedFilter;
    bool m_approvedWithCommentsFilter;
    bool m_reprovedFilter;
    bool m_movedFilter;
    bool m_filterTypePdf;
    QStringList m_undesirablePaths;
    QStringList m_headersOrder;
    QVector<bool> m_showColumns;

    QList<LogEntry> m_logEntries;
    QStringList m_readDatesList;
    QString m_filesPath;
    QMap<QString, LogEntry> m_activeFiles;
    QVector<LogEntry> m_historicFiles;
    QStringList m_employees;

    QSettings m_settings;
};

#endif // DATABASE_H
