#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScrollArea>
#include <QMap>

struct EngProp {
    QString obra;
    QString evento;
    QString tipo;
    QString nome;
    QString usuario;
    QString empresa;
    QString hora;
    QString caminho;
    QString arquivo;
    QString data;
    int epochTime;
};

class QComboBox;
class QTableWidget;
class QTabWidget;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QScrollArea
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void applyFilter(int index);

private:
    void load();
    void save();
    void loadData();
    QStringList getFiles();
    QString getDate(QString fileName);
    int getEpochTime(QString date, QString time);
    void reloadTableData();
    int indexOfFile(QString key);
    void populateTable();
    QStringList getEventos();
    void invertData();
    QTreeWidget* getTree();
    void openMenu();
    void paintRow(int epochTime, int row);
    void visitTree(QVector<QTreeWidgetItem*> &list, QTreeWidgetItem *item);
    void visitTree(QTreeWidget *tree);
    void updateUndesirabelPaths(QVector<QTreeWidgetItem*> items);
    bool isUndesirablePath(QString path);
    QString getPath(QTreeWidgetItem *item);

    QString m_path;

    QPushButton *m_settings;
    QComboBox *m_orderBy;
    QComboBox *m_filterCategory;
    QTableWidget *m_table;

    //FILTERS
    bool m_crescent;//crescente ou decrescente
    bool m_historicFilter; //historico/atual
    bool m_releasedFilter;
    bool m_approvedFilter;
    bool m_approvedWithCommentsFilter;
    bool m_reprovedFilter;
//    QStringList m_desirableWords;
    QStringList m_undesirablePaths;
    QMap<QString, QStringList> m_pathMap;
    QStringList *m_headersName;
    QVector<bool> m_showColumns;

    QVector<EngProp> m_tableData;
    QVector<EngProp> m_database;
};

#endif // MAINWINDOW_H
