#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScrollArea>
#include <QMap>



enum col {
    col_Feito = 0,
    col_Obra,
    col_Evento,
    col_Tipo,
    col_Nome,
    col_Usuario,
    col_Empresa,
    col_Hora,
    col_Caminho,
    col_Arquivo,
    col_Data,
};

class QComboBox;
class QTableWidget;
class QTabWidget;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QSettings;
class LogEntry;

class MainWindow : public QScrollArea
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void clearFilters();

private:
    void updateFromDatabase();
    void updateToDatabase();
    void initializeTable();
    void reloadTableData();
    int indexOfFile(QString key);
    void populateTable();
    QStringList getEventos();
    QTreeWidget* getTree();
    void openMenu();
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
    void orderTableByColumn(int index);
    void updateFromTable(int row, int col);

    QPushButton *m_clearFilters;
    QPushButton *m_config;
    QComboBox *m_filterCategory;
    QTableWidget *m_table;
    QString m_filesPath;
    QStringList m_readDates;

    //FILTERS
    bool m_historicFilter; //historico/atual
    bool m_releasedFilter;
    bool m_approvedFilter;
    bool m_approvedWithCommentsFilter;
    bool m_reprovedFilter;
    QStringList m_undesirablePaths;
    QVector<bool> m_showColumns;
    QStringList m_headersOrder;
    QVector<bool> m_orderByCrescent;

    QStringList m_headersName;
    QVector<LogEntry> m_tableData;
    QVector<LogEntry> m_logEntries;
};

#endif // MAINWINDOW_H
