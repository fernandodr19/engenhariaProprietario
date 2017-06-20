#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScrollArea>

struct EngProp {
    QString obra;
    QString evento;
    QString tipo;
    QString pasta;
    QString usuario;
    QString empresa;
    QString hora;
    QString caminho;
    QString arquivos;
};

class QLineEdit;
class QPushButton;
class QTableWidget;

class MainWindow : public QScrollArea
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void load();
    void populateTable();

    QLineEdit *m_fileName;
    QPushButton *m_loadFileButton;
    QTableWidget *m_table;

    QVector<EngProp> m_data;
};

#endif // MAINWINDOW_H
