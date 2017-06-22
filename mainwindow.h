#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScrollArea>

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

enum Filter {
    Filter_Actual,
    Filter_Historic,
};

class QComboBox;
class QTableWidget;
class QTabWidget;

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
    void reloadTable();
    int indexOfFile(QString key);
    void populateTable();
    QStringList getEventos();
    void invertData();

    QString m_path;

    QComboBox *m_orderBy;
    QComboBox *m_filterCategory;
    QTableWidget *m_table;

    bool m_crescent;//crescente ou decrescente
    bool m_historicFilter; //historico/atual
    bool m_releasedFilter;
    bool m_approvedFilter;
    bool m_approvedWithCommentsFilter;
    bool m_reprovedFilter;

    QVector<EngProp> m_tableData;
    QVector<EngProp> m_database;
};

#endif // MAINWINDOW_H
