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

#include <QDebug>

QSettings *g_settings = nullptr;

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
        reloadTableData();
        populateTable();
    });

    m_table = new QTableWidget(0, 11);
    m_table->setHorizontalHeaderLabels({"Feito", "Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});
    m_table->horizontalHeader()->setSectionsMovable(true);
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
    gridLayout->addWidget(m_filterCategory, row, col++);
    gridLayout->addWidget(new QLabel("Ordenar por :"), row, col++);
    gridLayout->addWidget(m_orderBy, row++, col++);
    gridLayout->addWidget(m_table, row++, 0, 1, col);


    QCoreApplication::setOrganizationName("Fluxo Engenharia");
    QCoreApplication::setOrganizationDomain("fluxoengenharia.com.br");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setApplicationName("Controle EP");

    g_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());

    loadFilters();
    loadData();
    initializeTable();
}

MainWindow::~MainWindow()
{
    save();
    delete g_settings;
}

void MainWindow::loadFilters()
{
    m_historicFilter = g_settings->value("historicFilter", false).toBool();
    m_releasedFilter = g_settings->value("releasedFilter", true).toBool();
    m_approvedFilter = g_settings->value("approvedFilter", false).toBool();
    m_approvedWithCommentsFilter = g_settings->value("approvedWithCommentsFilter", false).toBool();
    m_reprovedFilter = g_settings->value("reprovedFilter", false).toBool();

    m_headersName = new QStringList({"Feito", "Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});

    int size = g_settings->beginReadArray("showColumns");
    for (int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        bool show = g_settings->value("showColumn", true).toBool();
        m_showColumns.push_back(show);
    }
    g_settings->endArray();

    if(m_showColumns.size() != m_headersName->size()) {
        m_showColumns.clear();
        for(int i = 0; i < m_headersName->size(); i++)
            m_showColumns.push_back(true);
    }

    size = g_settings->beginReadArray("headersOrder");
    for (int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        QString header = g_settings->value("headerOrder", i).toString();
        m_headersOrder.push_back(header);
    }
    g_settings->endArray();

    if(m_headersOrder.size() != m_headersName->size()) {
        m_headersOrder.clear();
        for(int i = 0; i < m_headersName->size(); i++)
            m_headersOrder.push_back(m_headersName->at(i));
    }

    size = g_settings->beginReadArray("undesirablePaths");
    for (int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        QString path = g_settings->value("undesirablePath").toString();
        m_undesirablePaths.push_back(path);
    }
    g_settings->endArray();
}

void MainWindow::save()
{
    g_settings->setValue("historicFilter", m_historicFilter);
    g_settings->setValue("releasedFilter", m_releasedFilter);
    g_settings->setValue("approvedFilter", m_approvedFilter);
    g_settings->setValue("approvedWithCommentsFilter", m_approvedWithCommentsFilter);
    g_settings->setValue("reprovedFilter", m_reprovedFilter);

    g_settings->beginWriteArray("showColumns");
    for(int i = 0; i < m_showColumns.size(); ++i) {
        g_settings->setArrayIndex(i);
        g_settings->setValue("showColumn", m_showColumns[i]);
    }
    g_settings->endArray();

    g_settings->beginWriteArray("headersOrder");
    for(int i = 0; i < m_headersOrder.size(); ++i) {
        g_settings->setArrayIndex(i);
        g_settings->setValue("headerOrder", m_headersOrder[i]);
    }
    g_settings->endArray();

    g_settings->beginWriteArray("undesirablePaths");
    for(int i = 0; i < m_undesirablePaths.size(); ++i) {
        g_settings->setArrayIndex(i);
        g_settings->setValue("undesirablePath", m_undesirablePaths[i]);
    }
    g_settings->endArray();
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
//                qDebug() << engProp.evento;//checkHere
                m_database.push_back(engProp);
            }
        }
        file.close();
    }

    std::sort(m_database.begin(), m_database.end(), [](const EngProp& a, const EngProp& b) {
        return a.epochTime < b.epochTime;
    });

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
    for(EngProp engProp : m_tableData) {
        if(containsUndesirablePath(engProp.caminho))
            continue;

        m_table->insertRow(row);
        int col = -1;
        if(m_showColumns[++col]) {
            QTableWidgetItem *item = new QTableWidgetItem(1);
            item->data(Qt::CheckStateRole);
            item->setCheckState(Qt::Unchecked);
            m_table->setItem(row, col, item);
        }
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.obra));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.evento));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.tipo));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.nome));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.usuario));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.empresa));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.hora));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.caminho));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.arquivo));
        if(m_showColumns[++col])
            m_table->setItem(row, col, new QTableWidgetItem(engProp.data));
        paintRow(engProp.epochTime, row);
        row++;

    }
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
    for(EngProp engProp : m_database) {
        QString path = engProp.caminho;

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

void MainWindow::clearFilters()
{
    m_historicFilter = false;
    m_filterCategory->setCurrentIndex(0);

    m_orderBy->setCurrentIndex(0);

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
