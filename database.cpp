#include "database.h"
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include "logentry.h"
#include "mainwindow.h"
#include "savethread.h"

extern QSettings *g_settings;

Database::Database()
{
}

void Database::load()
{
    m_filesPath = g_settings->value("filesPath", "X:\\Linhas\\Em Andamento\\EQUATORIAL\\Controle EP\\dados").toString();
    m_releasedFilter = g_settings->value("releasedFilter", true).toBool();
    m_approvedFilter = g_settings->value("approvedFilter", false).toBool();
    m_approvedWithCommentsFilter = g_settings->value("approvedWithCommentsFilter", false).toBool();
    m_reprovedFilter = g_settings->value("reprovedFilter", false).toBool();
    m_movedFilter = g_settings->value("movedFilter", false).toBool();

    QStringList headersName = {"Encaminhado", "Download", "Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Data/Hora", "Caminho", "Arquivos"};

    int size = g_settings->beginReadArray("showColumns");
    for(int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        bool show = g_settings->value("showColumn", true).toBool();
        m_showColumns.push_back(show);
    }
    g_settings->endArray();

    if(m_showColumns.size() != headersName.size()) {
        m_showColumns.clear();
        for(int i = 0; i < headersName.size(); i++)
            m_showColumns.push_back(true);
    }

    size = g_settings->beginReadArray("headersOrder");
    for(int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        QString header = g_settings->value("headerOrder", i).toString();
        m_headersOrder.push_back(header);
    }
    g_settings->endArray();

    if(m_headersOrder.size() != headersName.size()) {
        m_headersOrder.clear();
        for(int i = 0; i < headersName.size(); i++)
            m_headersOrder.push_back(headersName[i]);
    }

    size = g_settings->beginReadArray("undesirablePaths");
    for(int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        QString path = g_settings->value("undesirablePath").toString();
        m_undesirablePaths.push_back(path);
    }
    g_settings->endArray();

    size = g_settings->beginReadArray("readDatesList");
    for(int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        QString path = g_settings->value("readDate").toString();
        m_readDatesList.push_back(path);
    }
    g_settings->endArray();

    loadLogEntriesFromFile();
    createFilesFromLogEntries();
    loadActiveFilesState();
}

void Database::save()
{
    g_settings->setValue("filesPath", m_filesPath);
    g_settings->setValue("releasedFilter", m_releasedFilter);
    g_settings->setValue("approvedFilter", m_approvedFilter);
    g_settings->setValue("approvedWithCommentsFilter", m_approvedWithCommentsFilter);
    g_settings->setValue("reprovedFilter", m_reprovedFilter);
    g_settings->setValue("movedFilter", m_movedFilter);

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

    g_settings->beginWriteArray("readDatesList");
    for(int i = 0; i < m_readDatesList.size(); ++i) {
        g_settings->setArrayIndex(i);
        g_settings->setValue("readDate", m_readDatesList[i]);
    }
    g_settings->endArray();
}

void Database::loadActiveFilesState()
{
    QSettings settings(m_filesPath + "\\database.ini", QSettings::IniFormat);
    for(const QString& fileName : settings.childGroups()) {
        settings.beginGroup(fileName);
        auto it = m_activeFiles.find(fileName);
        if(it != m_activeFiles.end()) {
            if(settings.value("forwarded").toString() == "true") //remover esse if depois
                it.value().forwarded = "Não informado";
            else
                it.value().forwarded = settings.value("forwarded").toString();
            it.value().downloaded = settings.value("downloaded", false).toBool();
        }
        settings.endGroup();
    }
}

QStringList Database::getEmployees()
{
    m_employees.clear();
    QSettings settings(m_filesPath + "\\database.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("employees");
    for(int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString employee = settings.value("employee").toString();
        m_employees.push_back(employee);
    }
    settings.endArray();

    std::sort(m_employees.begin(), m_employees.end(), [this](const QString& a, const QString& b) {
        return a < b;
    });

    return m_employees;
}

void Database::updateFiles()
{
    for(const LogEntry& logEntry : m_logEntries) {
        const QString& key = logEntry.name;
        const QString& evento = logEntry.event;
        if(evento == "Liberado para Cliente" ||
                evento == "Aprovado Cliente" ||
                evento == "Aprovado Cliente c/ Ressalva" ||
                evento == "Aprovado Cliente c/ Ressalvas" ||
                evento == "Para Informação" ||
                evento == "Mover / Copiar Arquivo" ||
                evento == "Reprovado Cliente") {
            m_historicFiles.push_back(logEntry);
        }

        if(evento == "Exclusão" /*|| evento == "Exclusão de versão"*/)
            m_activeFiles.remove(key);
        else
            m_activeFiles[key] = logEntry;
    }
}

void Database::loadLogEntriesFromFile()
{
    m_logEntries.clear();
    m_readDatesList.clear();
    for(const QString& fileName : getLogFiles()) {
        QFile file(m_filesPath + "\\" + fileName);
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
            LogEntry logEntry;
            logEntry.load(data, fields);

            QDateTime d = QDateTime::fromMSecsSinceEpoch(logEntry.epochTime);
            if(d < QDateTime(QDate(2017, 8, 2))) { // Nasty hack: old logs didnt have the prefix.
                const QString& path = logEntry.path;
                if(path.startsWith("\\Lote 08") || path.startsWith("\\Lote 09") || path.startsWith("\\Lote 12"))
                    logEntry.path = "\\LINHÃO BA-PI - LOTES 08, 09, 12 (TBAPI)" + path;
                else if(path.startsWith("\\Lote 14 A") || path.startsWith("\\Lote 14 B") || path.startsWith("\\Lote 15") || path.startsWith("\\Lote 16"))
                    logEntry.path = "\\LINHÃO MG-BA - LOTES 14, 15, 16 (TMGBA)" + path;
                else if(path.startsWith("\\Lote 23"))
                    logEntry.path = "\\LINHÃO PA - LOTE 23 (TPARA)" + path;
            }

            if(logEntry.event == "Mover / Copiar Arquivo") {
                if(logEntry.type == "Arquivo") {
                    QStringList l = logEntry.path.split('\\');
                    if(!l.empty())
                        logEntry.name = l.last();
                }
            }

            if(logEntry.event == "Aprovado Cliente" ||
                    logEntry.event == "Liberado para Cliente" ||
                    logEntry.event == "Reprovado Cliente" ||
                    logEntry.event == "Aprovado Cliente c/ Ressalva" ||
                    logEntry.event == "Aprovado Cliente c/ Ressalvas" ||
                    logEntry.event == "Transferindo para Versão" ||
                    logEntry.event == "Em aprovação" ||
                    logEntry.event == "Lista de Documentos" ||
                    logEntry.event == "Para Informação" ||
                    logEntry.event == "Mover / Copiar Arquivo" ||
                    //logEntry.event == "Exclusão de versão" ||
                    logEntry.event == "Exclusão") {
                m_logEntries.push_back(logEntry);
            }
        }
        file.close();
    }

    std::sort(m_logEntries.begin(), m_logEntries.end(), [this](const LogEntry& a, const LogEntry& b) {
        return a.epochTime < b.epochTime;
    });
}

QStringList Database::getLogFiles()
{
    QDir dir(m_filesPath);
    QStringList entryList = dir.entryList();

    QStringList filesList;
    for(const QString& candidate : entryList) {
        if(candidate.endsWith(".txt")) {
            bool contains = false;
            QString dateCandidate = candidate.mid(candidate.lastIndexOf("_") + 1, 8);
            for(const QString& file : filesList) {
                QString dateFile = file.mid(file.lastIndexOf("_") + 1, 8);
                if(dateFile == dateCandidate)
                    contains = true;
            }

            if(m_readDatesList.contains(dateCandidate))
                contains = true;

            if(!contains) {
                m_readDatesList.push_back(dateCandidate);
                filesList.push_back(candidate);
            }
        }
    }
    return filesList;
}

QString Database::getDate(QString fileName)
{
    fileName = fileName.mid(fileName.lastIndexOf("_") + 1, 8);
    QString year = fileName.mid(0, 4);
    QString month = fileName.mid(4, 2);
    QString day = fileName.mid(6, 2);
    return day + "/" + month + "/" + year;
}

void Database::reloadLogEntries()
{
    loadLogEntriesFromFile();
    createFilesFromLogEntries();
    loadActiveFilesState();
}

void Database::createFilesFromLogEntries()
{
    m_activeFiles.clear();
    m_historicFiles.clear();
    updateFiles();
}

void Database::updateUndesirablePaths(const QString& path, bool desirable)
{
    if(desirable)
        m_undesirablePaths.removeOne(path);
    else
        if(!m_undesirablePaths.contains(path))
            m_undesirablePaths.push_back(path);
}

void Database::updateDownloaded(const QString& file, bool checked)
{
    m_activeFiles[file].downloaded = checked;
    SaveThread *sThread = new SaveThread(m_filesPath + "\\database.ini", file, checked);
    sThread->start();
}

void Database::updateForwarded(const QString& file, const QString& person)
{
    m_activeFiles[file].forwarded = person;
    SaveThread *sThread = new SaveThread(m_filesPath + "\\database.ini", file, person);
    sThread->start();
}

void Database::updateEmployees(const QStringList& employees)
{
    m_employees = employees;
    SaveThread *sThread = new SaveThread(m_filesPath + "\\database.ini", employees);
    sThread->start();
}
