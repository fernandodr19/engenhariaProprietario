#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScrollArea>
#include <QMap>
#include <statisticsview.h>

enum column {
    col_Done = 0,
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
    void visitTree(QVector<QTreeWidgetItem*> &list, QTreeWidgetItem *item);
    void visitTree(QTreeWidget *tree);
    void updateUndesirabelPaths(const QVector<QTreeWidgetItem *>& items);
    bool containsUndesirablePath(const QString& path);
    bool isUndesirablePath(const QString& path);
    QString getPath(QTreeWidgetItem *item);
    Qt::CheckState getCheckState(QTreeWidgetItem *item);
    void setEnabled(QTreeWidgetItem *item);
    void updateHeadersOrder();
    void resetHeadersOrder();
    QString getDate(QString fileName);

    QPushButton *m_showRegistredDates;
    QPushButton *m_refreshDatabase;
    QPushButton *m_clearFilters;
    QPushButton *m_config;
    QComboBox *m_filterCategory;
    QTableWidget *m_table;
    QStringList m_readDates;

    //FILTERS
    bool m_historicFilter; //historico/atual
    bool m_releasedFilter;
    bool m_approvedFilter;
    bool m_approvedWithCommentsFilter;
    bool m_reprovedFilter;
    QStringList m_headersOrder;

    QStringList m_headersName;
};

#endif // MAINWINDOW_H
