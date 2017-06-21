#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QDateTime>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    QWidget *proxyWidget = new QWidget;
    setWidget(proxyWidget);

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    proxyWidget->setLayout(verticalLayout);

    QGridLayout *gridLayout = new QGridLayout;
    verticalLayout->addLayout(gridLayout);

    m_path = "X:\\Linhas\\Em Andamento\\EQUATORIAL\\Controle EP\\dados";

    m_orderBy = new QComboBox();
    m_orderBy->addItems({"Data crescente", "Data decrescente"});
    //    connect(m_orderBy, &QComboBox::currentIndexChanged, [this](int index) { orderIndexChanged(index); });


    m_table = new QTableWidget(0, 10);
    m_table->setHorizontalHeaderLabels({"Obra", "Evento", "Tipo", "Pasta", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});

    int row = 0;
    gridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), row, 0);
    gridLayout->addWidget(new QLabel("Ordenar por :"), row, 1);
    gridLayout->addWidget(m_orderBy, row++, 2);
    gridLayout->addWidget(m_table, row++, 0, 1, 3);

    load();
    populateTables();
}

MainWindow::~MainWindow()
{

}

void MainWindow::load()
{
    for(QString fileName : getFiles()) {
        QFile file(m_path + "\\" + fileName);
        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(0, "error", file.errorString());
        }

        QTextStream in(&file);

        if(!in.atEnd())
            in.readLine();

        QString data = getDate(fileName);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split("\t");
            if(fields.size() != 9)
                qDebug() << "erro";
            EngProp engProp;
            engProp.obra = fields[0];
            engProp.evento = fields[1];
            engProp.tipo = fields[2];
            engProp.nome = fields[3];
            engProp.usuario = fields[4];
            engProp.empresa = fields[5];
            engProp.hora = fields[6];
            engProp.caminho = fields[7];
            engProp.arquivos = fields[8];
            engProp.data = data;
            engProp.epochTime = getEpochTime(data, engProp.hora);
            if(engProp.evento == "Aprovado Cliente" ||
                    engProp.evento == "Liberado para Cliente" ||
                    engProp.evento == "Reprovado Cliente" ||
                    engProp.evento == "Aprovado Cliente c/ Ressalvas") {
                m_database.push_back(engProp);
            }
        }
        file.close();
    }

    std::sort(m_database.begin(), m_database.end(), [](const EngProp& a, const EngProp& b) {
        return a.epochTime < b.epochTime;
    });

    for(EngProp engProp : m_database)
        addEngProp(engProp);
}

QStringList MainWindow::getFiles()
{
    QDir dir(m_path);
    QStringList entryList = dir.entryList();

    QStringList filesList;
    bool contains = false;
    for(QString candidate : entryList) {
        contains = false;
        if(candidate.endsWith(".txt")) {
            QString dateCandidate = candidate.mid(candidate.lastIndexOf("_") + 1, 8);
            for(QString file : filesList) {
                QString dateFile = file.mid(file.lastIndexOf("_") + 1, 8);
                if(dateFile == dateCandidate)
                    contains = true;
            }
            if(!contains)
                filesList.push_back(candidate);
        }
    }
    return filesList;
}

QString MainWindow::getDate(QString fileName)
{
    fileName = fileName.mid(fileName.lastIndexOf("_") + 1, 8);

    QString year = fileName.mid(0, 4);
    QString month = fileName.mid(4, 2);
    QString day = fileName.mid(6, 2);
    return day + "/" + month + "/" + year;
}

int MainWindow::getEpochTime(QString date, QString time)
{
    if(date.length() < 10 || time.length() < 8)
        return -1;

    QStringList splitedDate = date.split("/");
    int year = splitedDate[2].toInt();
    int month = splitedDate[1].toInt();
    int day = splitedDate[0].toInt();

    QStringList splitedTime = time.split(":");
    int hour = splitedTime[0].toInt();
    int minute = splitedTime[1].toInt();
    int second = splitedTime[2].toInt();

    QDateTime dateTime(QDate(year, month, day), QTime(hour, minute, second));
    return dateTime.toSecsSinceEpoch();
}

void MainWindow::addEngProp(const EngProp engProp)
{
    QString key = engProp.nome;
    QString evento = engProp.evento;

    int index = indexOfFile(key);
    if(index != -1)
        m_tableData.removeAt(index);

    if(evento == "Liberado para Cliente")
        m_tableData.push_back(engProp);
}

int MainWindow::indexOfFile(QString key)
{
    for(int i = 0; i < m_tableData.size(); i++) {
        EngProp engProp = m_tableData[i];
        if(engProp.nome == key)
            return i;
    }
    return -1;
}

void MainWindow::populateTables()
{
    m_table->clearContents();
    m_table->setRowCount(0);
    for(int i = 0; i < m_tableData.size(); i++) {
        EngProp engProp = m_tableData[i];
        m_table->insertRow(i);
        m_table->setItem(i, 0, new QTableWidgetItem(engProp.obra));
        m_table->setItem(i, 1, new QTableWidgetItem(engProp.evento));
        m_table->setItem(i, 2, new QTableWidgetItem(engProp.tipo));
        m_table->setItem(i, 3, new QTableWidgetItem(engProp.nome));
        m_table->setItem(i, 4, new QTableWidgetItem(engProp.usuario));
        m_table->setItem(i, 5, new QTableWidgetItem(engProp.empresa));
        m_table->setItem(i, 6, new QTableWidgetItem(engProp.hora));
        m_table->setItem(i, 7, new QTableWidgetItem(engProp.caminho));
        m_table->setItem(i, 8, new QTableWidgetItem(engProp.arquivos));
        m_table->setItem(i, 9, new QTableWidgetItem(engProp.data));
    }
    m_table->resizeColumnsToContents();
}

void MainWindow::orderIndexChanged(int index)
{

}
