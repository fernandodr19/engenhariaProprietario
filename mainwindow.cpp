#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QScrollArea(parent)
{
//    setFixedHeight(800);
//    setFixedWidth(1000);
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    QWidget *proxyWidget = new QWidget;
    setWidget(proxyWidget);

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    proxyWidget->setLayout(verticalLayout);

    QGridLayout *gridLayout = new QGridLayout;
    verticalLayout->addLayout(gridLayout);

//    verticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_fileName = new QLineEdit();
    m_loadFileButton = new QPushButton("Recarregar");
    m_table = new QTableWidget(0, 9);
    m_table->setHorizontalHeaderLabels({"Obra", "Evento", "Tipo", "Pasta", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos"});

    int row = 0;
    gridLayout->addWidget(new QLabel("Nome do arquivo:"), row, 0);
    gridLayout->addWidget(m_fileName, row, 1);
    gridLayout->addWidget(m_loadFileButton, row++, 2);
    gridLayout->addWidget(m_table, row++, 0, 20, 3);

    load();
    populateTable();
}

MainWindow::~MainWindow()
{

}

void MainWindow::load()
{
    QFile file("C:\\fdr\\Construmanager.txt");
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);

    if(!in.atEnd())
        in.readLine();

    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split("\t");
        if(fields.size() != 9)
            qDebug() << "erro";
        EngProp engProp;
        engProp.obra = fields[0];
        engProp.evento = fields[1];
        engProp.tipo = fields[2];
        engProp.pasta = fields[3];
        engProp.usuario = fields[4];
        engProp.empresa = fields[5];
        engProp.hora = fields[6];
        engProp.caminho = fields[7];
        engProp.arquivos = fields[8];
        m_data.push_back(engProp);
    }

    file.close();
}

void MainWindow::populateTable()
{
    m_table->clearContents();
    m_table->setRowCount(0);
    for(int i = 0; i < m_data.size(); i++) {
        EngProp engProp = m_data[i];
        m_table->insertRow(i);
        m_table->setItem(i, 0, new QTableWidgetItem(engProp.obra));
        m_table->setItem(i, 1, new QTableWidgetItem(engProp.evento));
        m_table->setItem(i, 2, new QTableWidgetItem(engProp.tipo));
        m_table->setItem(i, 3, new QTableWidgetItem(engProp.pasta));
        m_table->setItem(i, 4, new QTableWidgetItem(engProp.usuario));
        m_table->setItem(i, 5, new QTableWidgetItem(engProp.empresa));
        m_table->setItem(i, 6, new QTableWidgetItem(engProp.hora));
        m_table->setItem(i, 7, new QTableWidgetItem(engProp.caminho));
        m_table->setItem(i, 8, new QTableWidgetItem(engProp.arquivos));
    }
    m_table->resizeColumnsToContents();
}
