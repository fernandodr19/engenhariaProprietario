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
    QString arquivos;
    QString data;
    int epochTime;
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
    void orderIndexChanged(int index);

private:
    void load();
    QStringList getFiles();
    QString getDate(QString fileName);
    int getEpochTime(QString date, QString time);
    void addEngProp(const EngProp engProp);
    int indexOfFile(QString key);
    void populateTables();

    QString m_path;

    QComboBox *m_orderBy;
    QTableWidget *m_table;

    QVector<EngProp> m_tableData;
    QVector<EngProp> m_database;
};

#endif // MAINWINDOW_H
