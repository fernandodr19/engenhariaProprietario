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
#include <QSettings>
#include <QApplication>

#include "logentry.h"
#include "database.h"

#include <QDebug>

Database *g_database = new Database();

MainWindow::MainWindow(QWidget *parent)
    : QScrollArea(parent)
{
    setMinimumWidth(1366);
    setMinimumHeight(768);
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);
    setWindowTitle("Consulta EP");
    setWindowIcon(QIcon("icons\\logo32.ico"));

    QWidget *proxyWidget = new QWidget;
    setWidget(proxyWidget);

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    proxyWidget->setLayout(verticalLayout);

    QGridLayout *gridLayout = new QGridLayout;
    verticalLayout->addLayout(gridLayout);

    m_path = "X:\\Linhas\\Em Andamento\\EQUATORIAL\\Controle EP\\dados";

    m_config = new QPushButton();
    m_config->setIcon(QIcon("icons\\settings.ico"));
    m_config->setStyleSheet("border: none");
    connect(m_config, &QPushButton::clicked, [this]() {
        openMenu();
    });

    m_clearFilters = new QPushButton("Limpar filtros");
    connect(m_clearFilters, &QPushButton::clicked, [this](bool) {
        clearFilters();
    });

    m_filterCategory = new QComboBox();
    m_filterCategory->addItem("Operações atuais");
    m_filterCategory->addItem("Histórico de operações");
    connect(m_filterCategory, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        if(index == 1)
            m_historicFilter = true;
        else
            m_historicFilter = false;
        reloadTableData();
        populateTable();
    });

    m_headersName = new QStringList({"Feito", "Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});

    m_table = new QTableWidget(0, 11);
    m_table->setHorizontalHeaderLabels({"Feito", "Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});
    m_table->horizontalHeader()->setSectionsMovable(true);
    connect(m_table->horizontalHeader(), &QHeaderView::sectionClicked, [this](int index) {
        orderTableByColumn(index);
        reloadTableData();
        populateTable();
    });
    connect(m_table->horizontalHeader(), &QHeaderView::sectionDoubleClicked, [this](int index) { applyFilter(index); });
    connect(m_table->horizontalHeader(), &QHeaderView::sectionPressed, [this]() {
        connect(m_table->horizontalHeader(), &QHeaderView::sectionMoved, [this](int, int from, int to) {
            m_headersOrder.move(from, to);
        });
    });

    int row = 0;
    int col = 0;
    gridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), row, col++);
    gridLayout->addWidget(m_config, row, col++);
    gridLayout->addWidget(m_clearFilters, row, col++);
    gridLayout->addWidget(new QLabel("Listar"), row, col++);
    gridLayout->addWidget(m_filterCategory, row++, col++);
    gridLayout->addWidget(m_table, row++, 0, 1, col);

    updateFromDatabase();
//    loadData();
    reloadTableData();
    initializeTable();
}

MainWindow::~MainWindow()
{
    updateToDatabase();
    g_database->save();
    delete g_database;
}

void MainWindow::updateFromDatabase()
{
    m_historicFilter = g_database->getHistoricFilter();
    m_releasedFilter = g_database->getReleasedFilter();
    m_approvedFilter = g_database->getApprovedFilter();
    m_approvedWithCommentsFilter = g_database->getApprovedWithCommentsFilter();
    m_reprovedFilter = g_database->getReprovedFilter();

    for(int i = 0; i < m_headersName->size(); i++) {
        m_orderByCrescent.push_back(true);
    }

    m_showColumns = g_database->getShowColumns();

    if(m_showColumns.size() != m_headersName->size()) {
        m_showColumns.clear();
        for(int i = 0; i < m_headersName->size(); i++)
            m_showColumns.push_back(true);
    }

    m_headersOrder = g_database->getHeadersOrder();

    if(m_headersOrder.size() != m_headersName->size()) {
        m_headersOrder.clear();
        for(int i = 0; i < m_headersName->size(); i++)
            m_headersOrder.push_back(m_headersName->at(i));
    }

    m_undesirablePaths = g_database->getUndesirablePaths();
    m_logEntries = g_database->getLogEntries();
}

void MainWindow::updateToDatabase()
{
    g_database->setHistoricFilter(m_historicFilter);
    g_database->setReleasedFilter(m_releasedFilter);
    g_database->setApprovedFitler(m_approvedFilter);
    g_database->setApprovedWithCommentsFilter(m_approvedWithCommentsFilter);
    g_database->setReprovedFilter(m_reprovedFilter);
    g_database->setShowColumns(m_showColumns);
    g_database->setHeadersOrder(m_headersOrder);
    g_database->setUndesirablePaths(m_undesirablePaths);
    g_database->setLogEntries(m_logEntries);
}


void MainWindow::loadData()//vai virar update database na classe database
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
            LogEntry logEntry;
            logEntry.obra = fields[0];
            logEntry.evento = fields[1];
            logEntry.tipo = fields[2];
            logEntry.nome = fields[3];
            logEntry.usuario = fields[4];
            logEntry.empresa = fields[5];
            logEntry.hora = fields[6];
            logEntry.caminho = fields[7];
            logEntry.arquivo = fields[8];
            logEntry.data = data;
            logEntry.epochTime = getEpochTime(data, logEntry.hora);
            if(logEntry.evento == "Aprovado Cliente" ||
                    logEntry.evento == "Liberado para Cliente" ||
                    logEntry.evento == "Reprovado Cliente" ||
                    logEntry.evento == "Aprovado Cliente c/ Ressalvas") {
                m_logEntries.push_back(logEntry);
            }
        }
        file.close();
    }
    orderTableByColumn(col_Data);
    reloadTableData();
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
    return dateTime.toMSecsSinceEpoch();
}

void MainWindow::reloadTableData()
{
    m_tableData.clear();

    QStringList eventos = getEventos();
    for(LogEntry logEntry : m_logEntries) {
        QString key = logEntry.nome;
        QString evento = logEntry.evento;

        int index = indexOfFile(key);
        if(!m_historicFilter) {
            if(index != -1)
                m_tableData.removeAt(index);
        }

        if(eventos.contains(evento))
            m_tableData.push_back(logEntry);
    }
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
        LogEntry logEntry = m_tableData[i];
        if(logEntry.nome == key)
            return i;
    }
    return -1;
}

void MainWindow::initializeTable()
{
    for(int i = 0; i < m_showColumns.size(); i++)
        m_table->setColumnHidden(i, !m_showColumns[i]);

    updateHeadersOrder();

    populateTable();
}

void MainWindow::populateTable()
{
    m_table->clearContents();
    m_table->setRowCount(0);

    int row = 0;
    for(LogEntry logEntry : m_tableData) {
        if(containsUndesirablePath(logEntry.caminho))
            continue;

        m_table->insertRow(row);
        int col = -1;
        if(m_showColumns[++col]) {
            QTableWidgetItem *item = new QTableWidgetItem(1);
            item->data(Qt::CheckStateRole);
            if(!logEntry.feito)
                item->setCheckState(Qt::Unchecked);
            else
                item->setCheckState(Qt::Checked);
            m_table->setItem(row, col, item);
        }
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.obra));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.evento));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.tipo));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.nome));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.usuario));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.empresa));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.hora));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.caminho));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.arquivo));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(logEntry.data));
        paintRow(logEntry.epochTime, row);
        row++;

    }

    connect(m_table, &QTableWidget::cellClicked, [this](int row, int column) {
        updateFromTable(row, column);
    });
    m_table->resizeColumnsToContents();
}

void MainWindow::paintRow(int epochTime, int row)
{
    int day = 86400000;//msecs
    int actualTime = QDateTime::currentMSecsSinceEpoch();
    int diff = actualTime - epochTime;

    if(diff > day*7) {
        for(int col = 1; col < m_headersName->size(); col++) {
            if(m_showColumns[col])
                m_table->item(row, col)->setBackgroundColor(QColor(255, 50, 70));
        }
    } else if(diff > day*5) {
        for(int col = 1; col < m_headersName->size(); col++) {
            if(m_showColumns[col])
                m_table->item(row, col)->setBackgroundColor(QColor(255,165,0));
        }
    } else if(diff > day*3) {
        for(int col = 1; col < m_headersName->size(); col++) {
            if(m_showColumns[col])
                m_table->item(row, col)->setBackgroundColor(QColor(Qt::yellow));
        }
    }
}

void MainWindow::applyFilter(int index)
{
    if(index == 2) {
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

        reloadTableData();
    }

    if(index == 8) {
        QDialog dialog;
        dialog.setWindowTitle("Filtros");
        dialog.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        dialog.setMinimumWidth(800);

        QFormLayout* formLayout = new QFormLayout();
        dialog.setLayout(formLayout);

        QTreeWidget* treeWidget = getTree();
        formLayout->addRow(treeWidget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
        formLayout->addRow(buttonBox);
        connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        if(dialog.exec() != QDialog::Accepted)
            return;

        visitTree(treeWidget);
    }
}

void MainWindow::orderTableByColumn(int index)
{
    if(index > m_orderByCrescent.size() - 1)
        return;

    bool crescent = m_orderByCrescent[index];

    std::sort(m_logEntries.begin(), m_logEntries.end(), [index, crescent](const LogEntry& a, const LogEntry& b) {
        switch(index) {
        case col_Obra:
            if(crescent)
                return a.obra < b.obra;
            else
                return a.obra > b.obra;
        case col_Evento:
            if(crescent)
                return a.evento < b.evento;
            else
                return a.evento > b.evento;
        case col_Tipo:
            if(crescent)
                return a.tipo < b.tipo;
            else
                return a.tipo > b.tipo;
        case col_Nome:
            if(crescent)
                return a.nome < b.nome;
            else
                return a.nome > b.nome;
        case col_Usuario:
            if(crescent)
                return a.usuario < b.usuario;
            else
                return a.usuario > b.usuario;
        case col_Empresa:
            if(crescent)
                return a.empresa < b.empresa;
            else
                return a.empresa > b.empresa;
        case col_Hora:
            if(crescent)
                return a.epochTime < b.epochTime;
            else
                return a.epochTime > b.epochTime;
        case col_Caminho:
            if(crescent)
                return a.caminho < b.caminho;
            else
                return a.caminho > b.caminho;
        case col_Arquivo:
            if(crescent)
                return a.arquivo < b.arquivo;
            else
                return a.arquivo > b.arquivo;
        case col_Data:
            if(crescent)
                return a.epochTime < b.epochTime;
            else
                return a.epochTime > b.epochTime;
        }
    });
    m_orderByCrescent[index] = !m_orderByCrescent[index];
}

void MainWindow::updateHeadersOrder()
{
    QStringList names;
    for(int i = 0; i < m_headersName->size(); i++)
        names.push_back(m_headersName->at(i));

    for(int i = 0; i < names.size(); i++) {
        int index = names.indexOf(m_headersOrder[i]);
        if(index != i) {
            m_table->horizontalHeader()->moveSection(index, i);
            names.move(index, i);
        }
    }
}

void MainWindow::resetHeadersOrder()//checkHere
{
    //    for(int i = 0; i < m_headersName->size(); i++) {
    //        int index = m_headersOrder.indexOf(m_headersName->at(i));
    //        if(index != i) {
    //            m_table->horizontalHeader()->moveSection(index, i);
    //        }
    //    }
}

QTreeWidget* MainWindow::getTree()
{
    QTreeWidget *treeWidget = new QTreeWidget();
    connect(treeWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem *item){
        for (int i = 0; i < item->childCount(); ++i) {
            setEnabled(item->child(i));
        }

    });
    QTreeWidgetItem *topLevelItem;

    QStringList singlePaths;
    for(LogEntry logEntry : m_logEntries) {
        QString path = logEntry.caminho;

        if((path.length() - path.lastIndexOf(".")) == 4)
            path = path.mid(0, path.lastIndexOf("\\"));

        if(!singlePaths.contains(path))
            singlePaths.push_back(path);
    }

    for(QString path : singlePaths) {
        QStringList tokens = path.split("\\");
        tokens.removeOne("");

        // add root folder as top level item if treeWidget doesn't already have it
        if (treeWidget->findItems(tokens[0], Qt::MatchFixedString).isEmpty()) {
            topLevelItem = new QTreeWidgetItem;
            topLevelItem->setFlags(topLevelItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
            topLevelItem->setText(0, tokens[0]);
            topLevelItem->setCheckState(0, getCheckState(topLevelItem));
            //            setEnabled(topLevelItem);
            topLevelItem->setIcon(0, QIcon("icons\\file.png"));
            treeWidget->addTopLevelItem(topLevelItem);
        }

        QTreeWidgetItem *parentItem = topLevelItem;

        // iterate through non-root directories (file name comes after)
        for (int i = 1; i < tokens.size() - 1; ++i) {
            // iterate through children of parentItem to see if this directory exists
            bool thisDirectoryExists = false;
            for (int j = 0; j < parentItem->childCount(); ++j) {
                if (tokens[i] == parentItem->child(j)->text(0)) {
                    thisDirectoryExists = true;
                    parentItem = parentItem->child(j);
                    break;
                }
            }

            if (!thisDirectoryExists) {
                parentItem = new QTreeWidgetItem(parentItem);
                parentItem->setFlags(parentItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
                parentItem->setText(0, tokens[i]);
                parentItem->setCheckState(0, getCheckState(parentItem));
                setEnabled(parentItem);
                parentItem->setIcon(0, QIcon("icons\\file.png"));
            }
        }

        QTreeWidgetItem *childItem = new QTreeWidgetItem(parentItem);
        childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        childItem->setText(0, tokens.last());
        childItem->setCheckState(0, getCheckState(childItem));

    }
    return treeWidget;
}

void MainWindow::openMenu()
{
    QDialog dialog;
    dialog.setWindowTitle("Menu");
    dialog.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QGridLayout* gridLayout = new QGridLayout();
    dialog.setLayout(gridLayout);

    QVBoxLayout *vBoxCol = new QVBoxLayout;

    QVector<QCheckBox*> checkBoxesCol;
    for(int i = 0; i < m_headersName->size(); i++) {
        QCheckBox  *checkBox = new QCheckBox (m_headersName->at(i));
        checkBox->setChecked(m_showColumns[i]);
        vBoxCol->addWidget(checkBox);
        checkBoxesCol.push_back(checkBox);
    }

    QGroupBox *groupBoxCol = new QGroupBox();
    groupBoxCol->setLayout(vBoxCol);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    int row = 0;
    gridLayout->addWidget(new QLabel("Exibir colunas:"), row++, 0);
    gridLayout->addWidget(groupBoxCol, row++, 0);
    gridLayout->addWidget(buttonBox, row++, 0);


    if(dialog.exec() != QDialog::Accepted)
        return;

    for(int i = 0; i < m_showColumns.size(); i++) {
        bool show = checkBoxesCol[i]->isChecked();
        m_showColumns[i] = show;
        m_table->setColumnHidden(i, !show);
    }
    populateTable();
}

void MainWindow::clearFilters()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Limpar filtros", "Tem certeza de que deseja limpar todos os filtros?", QMessageBox::Yes|QMessageBox::No);

    if(reply != QMessageBox::Yes)
        return;

    m_historicFilter = false;
    m_filterCategory->setCurrentIndex(0);

    resetHeadersOrder();
    m_headersOrder.clear();
    for(int i = 0; i < m_headersName->size(); i++)
        m_headersOrder.push_back(m_headersName->at(i));

    m_releasedFilter = true;
    m_approvedFilter = false;
    m_approvedWithCommentsFilter = false;
    m_reprovedFilter = false;

    for(int i = 0; i < m_showColumns.size(); i++) {
        m_showColumns[i] = true;
        m_table->setColumnHidden(i, false);
    }

    m_undesirablePaths.clear();


    m_orderByCrescent.clear();
    for(int i = 0; i < m_headersName->size(); i++) {
        m_orderByCrescent.push_back(true);
    }

    orderTableByColumn(col_Data);

    reloadTableData();
    populateTable();
}

void MainWindow::visitTree(QVector<QTreeWidgetItem*> &list, QTreeWidgetItem *item){
    if(item->checkState(0) == Qt::Unchecked)
        list.push_back(item);
    for(int i=0;i<item->childCount(); ++i) {
        visitTree(list, item->child(i));
    }
}

void MainWindow::visitTree(QTreeWidget *tree) {
    QVector<QTreeWidgetItem*> uncheckdItems;
    for(int i=0;i<tree->topLevelItemCount();++i)
        visitTree(uncheckdItems, tree->topLevelItem(i));
    updateUndesirabelPaths(uncheckdItems);
}

void MainWindow::updateUndesirabelPaths(QVector<QTreeWidgetItem *> items)
{
    QString path;
    m_undesirablePaths.clear();
    for(QTreeWidgetItem* item : items) {
        path = getPath(item);
        m_undesirablePaths.push_back(path);
    }

    populateTable();
}

bool MainWindow::containsUndesirablePath(QString path)
{
    for(QString undesirablePath : m_undesirablePaths)
        if(path.contains(undesirablePath))
            return true;
    return false;
}

bool MainWindow::isUndesirablePath(QString path)
{
    for(QString undesirablePath : m_undesirablePaths)
        if(path == undesirablePath)
            return true;
    return false;
}

QString MainWindow::getPath(QTreeWidgetItem *item)
{
    QString path;
    while(item != nullptr) {
        path = "\\" + item->text(0) + path;
        item = item->parent();
    }
    return path;
}

Qt::CheckState MainWindow::getCheckState(QTreeWidgetItem *item)
{
    if(isUndesirablePath(getPath(item)))
        return Qt::Unchecked;
    return Qt::Checked;
}

void MainWindow::setEnabled(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();
    if(parent == nullptr)
        return;

    if(parent->checkState(0) == Qt::Unchecked)
        item->setFlags(item->flags() &= ~Qt::ItemIsEnabled);
    else
        item->setFlags(item->flags() | Qt::ItemIsEnabled);
}

void MainWindow::updateFromTable(int row, int col)
{
    if(col != 0)
        return;

    QTableWidgetItem *item = m_table->item(row, col);
    int indexTableData = -1;
    int indexDatabase = -1;
    for(int i = 0; i < m_tableData.size(); i++) {
        QString nome = m_table->item(row, col_Nome)->text();
        QString hora = m_table->item(row, col_Hora)->text();
        if(m_tableData[i].nome == nome && m_tableData[i].hora == hora)
            indexTableData = i;

        if(m_logEntries[i].nome == nome && m_logEntries[i].hora == hora)
            indexDatabase = i;
    }

    if(indexTableData == -1)
        return;

    if(item->checkState() == Qt::Checked)
        m_tableData[indexTableData].feito = true;
    else
        m_tableData[indexTableData].feito = false;

    if(indexDatabase == -1)
        return;

    if(item->checkState() == Qt::Checked)
        m_logEntries[indexDatabase].feito = true;
    else
        m_logEntries[indexDatabase].feito = false;

}

