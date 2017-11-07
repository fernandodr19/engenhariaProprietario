#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScrollArea>
#include <QMap>
#include <statisticsview.h>

enum column {
    col_Forwarded = 0,
    col_Downloaded,
    col_Work,
    col_Event,
    col_Type,
    col_Name,
    col_User,
    col_Company,
    col_DateHour,
    col_Path,
    col_File,
};

class QComboBox;
class QTableWidget;
class QTabWidget;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QSettings;
class QStatusBar;
class QCheckBox;

class MainWindow : public QScrollArea
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);

    void closeEvent(QCloseEvent *event);

private slots:
    void clearFilters();
    void showRegistredDates();
    void reloadDatabase();

private:
    void updateFromDatabase();
    void updateToDatabase();
    void initializeTable();
    void reloadTableData();
    void populateTable();
    void insertRow(const LogEntry& logEntry, int row);
    QStringList getEventos();
    QTreeWidget* getTree();
    void openMenu();
    void openStatisticsDialog();
    void openStatisticsView(statistic_graph g);
    void paintRow(qint64, int row);
    bool containsUndesirablePath(const QString& path);
    bool isUndesirablePath(const QString& path);
    QString getPath(QTreeWidgetItem *item);
    Qt::CheckState getCheckState(QTreeWidgetItem *item);
    void setEnabled(QTreeWidgetItem *item);
    void updateHeadersOrder();
    void resetHeadersOrder();
    QString getDate(QString fileName);
    void customMenuRequested(QPoint p);
    void editEmployees();
    void showUndesirablePaths();
    QStringList getTableEmployees(QTableWidget *table);
    void exportExcel();
    void filterPdfFiles();

    QPushButton *m_showRegistredDates;
    QPushButton *m_reloadDatabase;
    QPushButton *m_statisticsButton;
    QPushButton *m_exportExcelButton;
    QPushButton *m_clearFilters;
    QPushButton *m_config;
    QComboBox *m_filterCategory;
    QTableWidget *m_table;
    QStatusBar *m_statusBar;
    QCheckBox *m_filterType;

    QStringList m_readDates;
    QLabel *m_selectedRowsCount;

    //FILTERS
    bool m_historicFilter; //historico/atual
    bool m_releasedFilter;
    bool m_approvedFilter;
    bool m_approvedWithCommentsFilter;
    bool m_reprovedFilter;
    bool m_movedFilter;
    QStringList m_headersOrder;

    QStringList m_headersName;
};

#endif // MAINWINDOW_H
