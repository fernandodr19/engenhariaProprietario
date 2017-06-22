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
#include <QHeaderView>
#include <QTabWidget>
#include <QTableWidget>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QSettings>
#include <QGroupBox>
#include <QIcon>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QScrollArea(parent)
{
    setMinimumWidth(1366);
    setMinimumHeight(768);
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setWindowTitle("Consulta EP");
    setWindowIcon(QIcon("icons/logo32.ico"));

    QWidget *proxyWidget = new QWidget;
    setWidget(proxyWidget);

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    proxyWidget->setLayout(verticalLayout);

    QGridLayout *gridLayout = new QGridLayout;
    verticalLayout->addLayout(gridLayout);

    m_path = "X:\\Linhas\\Em Andamento\\EQUATORIAL\\Controle EP\\dados";

    m_orderBy = new QComboBox();
    m_orderBy->addItem("Mais antigo");
    m_orderBy->addItem("Mais recente");
    connect(m_orderBy, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        if(index == 1)
            m_crescent = true;
        else
            m_crescent = false;
        invertData();
    });

    m_filterCategory = new QComboBox();
    m_filterCategory->addItem("Operações atuais");
    m_filterCategory->addItem("Histórico de operações");
    connect(m_filterCategory, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        if(index == 1)
            m_historicFilter = true;
        else
            m_historicFilter = false;
        reloadTable();
    });


    m_table = new QTableWidget(0, 10);
    m_table->setHorizontalHeaderLabels({"Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});
    connect(m_table->horizontalHeader(), &QHeaderView::sectionDoubleClicked, [this](int index) { applyFilter(index); });

    int row = 0;
    int col = 0;
    gridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), row, col++);
    gridLayout->addWidget(new QLabel("Listar"), row, col++);
    gridLayout->addWidget(m_filterCategory, row, col++);
    gridLayout->addWidget(new QLabel("Ordenar por :"), row, col++);
    gridLayout->addWidget(m_orderBy, row++, col++);
    gridLayout->addWidget(m_table, row++, 0, 1, col);

    load();
    loadData();
    populateTable();
}

MainWindow::~MainWindow()
{
    save();
}

void MainWindow::load()
{
    m_historicFilter = false;
    m_releasedFilter = true;
    m_approvedFilter = false;
    m_approvedWithCommentsFilter = false;
    m_reprovedFilter = false;
}

void MainWindow::save()
{
    qDebug() << "saved";
}

void MainWindow::loadData()
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
            engProp.arquivo = fields[8];
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

    reloadTable();
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

void MainWindow::reloadTable()
{
    m_tableData.clear();

    QStringList eventos = getEventos();
    for(EngProp engProp : m_database) {
        QString key = engProp.nome;
        QString evento = engProp.evento;

        int index = indexOfFile(key);
        if(!m_historicFilter) {
            if(index != -1)
                m_tableData.removeAt(index);
        }

        if(eventos.contains(evento))
            m_tableData.push_back(engProp);
    }

    populateTable();
}

QStringList MainWindow::getEventos()
{
    QStringList eventos;
    if(m_releasedFilter)
        eventos.push_back("Liberado para Cliente");
    if(m_approvedFilter)
        eventos.push_back("Aprovado Cliente");
    if(m_approvedWithCommentsFilter)
        eventos.push_back("Aprovado Cliente c/ Ressalvas");
    if(m_reprovedFilter)
        eventos.push_back("Reprovado Cliente");
    return eventos;

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

void MainWindow::populateTable()
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
        m_table->setItem(i, 8, new QTableWidgetItem(engProp.arquivo));
        m_table->setItem(i, 9, new QTableWidgetItem(engProp.data));
    }
    m_table->resizeColumnsToContents();
}

void MainWindow::applyFilter(int index)
{
    if(index == 1) {
        QDialog dialog;
        dialog.setWindowTitle("Filtros");
        dialog.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        QFormLayout* formLayout = new QFormLayout();
        dialog.setLayout(formLayout);

        formLayout->addRow(new QLabel("Eventos a serem filtrados:"));

        QVBoxLayout *vBox = new QVBoxLayout;

        QCheckBox  *releasedBox = new QCheckBox ("Liberado para Cliente");
        releasedBox->setChecked(m_releasedFilter);
        vBox->addWidget(releasedBox);
        QCheckBox  *approvedBox = new QCheckBox ("Aprovado Cliente");
        approvedBox->setChecked(m_approvedFilter);
        vBox->addWidget(approvedBox);
        QCheckBox  *approvedWithCommentsBox = new QCheckBox ("Aprovado Cliente c/ Ressalvas");
        approvedWithCommentsBox->setChecked(m_approvedWithCommentsFilter);
        vBox->addWidget(approvedWithCommentsBox);
        QCheckBox  *reprovedBox = new QCheckBox ("Reprovado Cliente");
        reprovedBox->setChecked(m_reprovedFilter);
        vBox->addWidget(reprovedBox);

        QGroupBox *groupBox = new QGroupBox();
        groupBox->setLayout(vBox);

        formLayout->addRow(groupBox);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
        formLayout->addRow(buttonBox);
        connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        if(dialog.exec() != QDialog::Accepted)
            return;

        m_releasedFilter = releasedBox->isChecked();
        m_approvedFilter = approvedBox->isChecked();
        m_approvedWithCommentsFilter = approvedWithCommentsBox->isChecked();
        m_reprovedFilter = reprovedBox->isChecked();

        reloadTable();
    }
}

void MainWindow::invertData()
{
    QVector<EngProp> clone;
    for(int i = m_tableData.size() - 1; i >= 0; i--)
        clone.push_back(m_tableData[i]);

    m_tableData.clear();
    for(EngProp engProp : clone)
        m_tableData.push_back(engProp);

    populateTable();
}
