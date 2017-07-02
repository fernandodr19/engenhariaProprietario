#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>

class LogEntry;

class Database
{
public:
    Database();
    ~Database();

    void load();
    void save();

    bool getHistoricFilter() { return m_historicFilter; }
    bool getReleasedFilter() { return m_releasedFilter; }
    bool getApprovedFilter() { return m_approvedFilter; }
    bool getApprovedWithCommentsFilter() { return m_approvedWithCommentsFilter; }
    bool getReprovedFilter() { return m_reprovedFilter; }
    QStringList getUndesirablePaths() { return m_undesirablePaths; }
    QStringList getHeadersOrder() { return m_headersOrder; }
    QVector<bool> getShowColumns() { return m_showColumns; }
    QVector<LogEntry> getLogEntries() { return m_logEntries; }

    void setHistoricFilter(bool historicFilter) { m_historicFilter = historicFilter; }
    void setReleasedFilter(bool releasedFilter) { m_releasedFilter = releasedFilter; }
    void setApprovedFitler(bool approvedFitler) { m_approvedFilter = approvedFitler; }
    void setApprovedWithCommentsFilter(bool approvedWithComments) { m_approvedWithCommentsFilter = approvedWithComments; }
    void setReprovedFilter(bool reprovedFilter) { m_reprovedFilter = reprovedFilter; }
    void setUndesirablePaths(QStringList undesirablePaths) { m_undesirablePaths = undesirablePaths; }
    void setHeadersOrder(QStringList headersOrder) { m_headersOrder = headersOrder; }
    void setShowColumns(QVector<bool> showColumns) { m_showColumns = showColumns; }
    void setLogEntries(QVector<LogEntry> logEntries) { m_logEntries = logEntries; }

private:
    void loadLogEntry();
    void saveLogEntry();

    //FILTERS
    bool m_historicFilter; //historico/atual
    bool m_releasedFilter;
    bool m_approvedFilter;
    bool m_approvedWithCommentsFilter;
    bool m_reprovedFilter;
    QStringList m_undesirablePaths;
    QStringList m_headersOrder;
    QVector<bool> m_showColumns;

    QVector<LogEntry> m_logEntries;
};

#endif // DATABASE_H
