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
#include <QTextEdit>
#include <QTreeWidget>
#include <QMapIterator>
#include <QPushButton>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QScrollArea(parent)
{
    setMinimumWidth(1366);
    setMinimumHeight(768);
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setWindowTitle("Consulta EP");
    setWindowIcon(QIcon("C:\\fdr\\ep\\EngenhariaProprietario\\icons\\logo32.ico"));

    QWidget *proxyWidget = new QWidget;
    setWidget(proxyWidget);

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    proxyWidget->setLayout(verticalLayout);

    QGridLayout *gridLayout = new QGridLayout;
    verticalLayout->addLayout(gridLayout);

    m_path = "X:\\Linhas\\Em Andamento\\EQUATORIAL\\Controle EP\\dados";

    m_settings = new QPushButton();
    m_settings->setIcon(QIcon("C:\\fdr\\ep\\EngenhariaProprietario\\icons\\settings.ico"));
    m_settings->setStyleSheet("border: none");
    connect(m_settings, &QPushButton::clicked, [this]() {
        openMenu();
    });

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
    m_table->horizontalHeader()->setSectionsMovable(true);
    connect(m_table->horizontalHeader(), &QHeaderView::sectionDoubleClicked, [this](int index) { applyFilter(index); });

    int row = 0;
    int col = 0;
    gridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), row, col++);
    gridLayout->addWidget(m_settings, row, col++);
    gridLayout->addWidget(new QLabel("Listar"), row, col++);
    gridLayout->addWidget(m_filterCategory, row, col++);
    gridLayout->addWidget(new QLabel("Ordenar por :"), row, col++);
    gridLayout->addWidget(m_orderBy, row++, col++);
    gridLayout->addWidget(m_table, row++, 0, 1, col);

    load();
    loadData();
    //    populateTable();
    getPathMap();
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
    m_headersName = new QStringList({"Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});
//    m_showColumns = new QVector<bool>();
    for(int i = 0; i < m_headersName->size(); i++)
        m_showColumns.push_back(true);

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
        int col = -1;
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.obra));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.evento));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.tipo));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.nome));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.usuario));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.empresa));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.hora));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.caminho));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.arquivo));
        if(m_showColumns[++col])
            m_table->setItem(i, col, new QTableWidgetItem(engProp.data));
    }
    m_table->resizeColumnsToContents();
}

void MainWindow::applyFilter(int index)
{
    m_table->clearFocus();
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

    if(index == 7) {
        QDialog dialog;
        dialog.setWindowTitle("Filtros");
        dialog.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        dialog.setMinimumWidth(800);

        QFormLayout* formLayout = new QFormLayout();
        dialog.setLayout(formLayout);

        //        QLabel *desirableWordsLabel = new QLabel("Palavras desejáveis:");
        //        QTextEdit *desirableWords = new QTextEdit();
        //        formLayout->addRow(new QLabel("Palavras desejáveis:"), desirableWords);

        //        QTextEdit *undesirableWords = new QTextEdit();
        //        formLayout->addRow(new QLabel("Palavras indesejáveis:"), undesirableWords);

        getPathMap();

        QTreeWidget *treeWidget = new QTreeWidget();
        treeWidget->setColumnCount(1);
        QList<QTreeWidgetItem *> items;
        for (int i = 0; i < 10; ++i) {
            QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i)));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
            item->setCheckState(0, Qt::Checked);
            items.append(item);
        }
        treeWidget->insertTopLevelItems(0, items);
        treeWidget->itemAt(0, 0)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(90))));

        formLayout->addRow(treeWidget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
        formLayout->addRow(buttonBox);
        connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        if(dialog.exec() != QDialog::Accepted)
            return;
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

void MainWindow::getPathMap()
{
    QMap<QString, Path> root;
    for(EngProp engProp : m_database) {

        QString caminho = engProp.caminho;//"\\Lote 08\\LT\\Trecho 1 - LT Rio das Éguas - Barreiras II C2\\02 - Topografia\\Planta Cadastral de Propriedades";
        qDebug() << caminho;
        QStringList fields = caminho.split("\\");

        fields.removeOne("");
        for(int i = 0; i < fields.size() - 1; i++) {
            QString actual = fields[i];
            QString next = fields[i+1];
            root[actual].m_children[next];//.name = next;
        }
    }
    //    root["Lote 8"].m_children["LT"];
    //    root["LT"].m_children["blabla"];
    //    root["blablabla"].m_children["kitkat"];

    //    root["Lote 9"].m_children["LT"];

    QMapIterator<QString, Path> i(root);
    while(i.hasNext()) {
        i.next();
        qDebug() << i.key();// << ": " << i.value().name;
    }




}

void MainWindow::openMenu()
{
    QDialog dialog;
    dialog.setWindowTitle("Menu");
    dialog.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QFormLayout* formLayout = new QFormLayout();
    dialog.setLayout(formLayout);

    formLayout->addRow(new QLabel("Exibir colunas:"));

    QVBoxLayout *vBox = new QVBoxLayout;

    QVector<QCheckBox*> checkBoxes;
    for(int i = 0; i < m_headersName->size(); i++) {
        QCheckBox  *checkBox = new QCheckBox (m_headersName->at(i));
        checkBox->setChecked(m_showColumns[i]);
        vBox->addWidget(checkBox);
        checkBoxes.push_back(checkBox);
    }

    QGroupBox *groupBox = new QGroupBox();
    groupBox->setLayout(vBox);

    formLayout->addRow(groupBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    formLayout->addRow(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if(dialog.exec() != QDialog::Accepted)
        return;

    for(int i = 0; i < m_showColumns.size(); i++) {
        bool show = checkBoxes[i]->isChecked();
        m_showColumns[i] = show;
        m_table->setColumnHidden(i, !show);
    }
    populateTable();
}
