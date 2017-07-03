#include "database.h"
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include "logentry.h"

QSettings *g_settings = nullptr;

Database::Database()
{
//    m_path = "X:\\Linhas\\Em Andamento\\EQUATORIAL\\Controle EP\\dados";//vai ser uma variavel

    QCoreApplication::setOrganizationName("Fluxo Engenharia");
    QCoreApplication::setOrganizationDomain("fluxoengenharia.com.br");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setApplicationName("Controle EP");

    g_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
}

Database::~Database()
{
    delete g_settings;
}

void Database::load()
{
    m_filesPath = g_settings->value("filesPath").toString();

    m_historicFilter = g_settings->value("historicFilter", false).toBool();
    m_releasedFilter = g_settings->value("releasedFilter", true).toBool();
    m_approvedFilter = g_settings->value("approvedFilter", false).toBool();
    m_approvedWithCommentsFilter = g_settings->value("approvedWithCommentsFilter", false).toBool();
    m_reprovedFilter = g_settings->value("reprovedFilter", false).toBool();

    QStringList headersName = {"Feito", "Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Data/Hora", "Caminho", "Arquivos"};

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

    loadLogEntry();
}

void Database::save()
{
    g_settings->setValue("filesPath", m_filesPath);

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

    g_settings->beginWriteArray("readDatesList");
    for(int i = 0; i < m_readDatesList.size(); ++i) {
        g_settings->setArrayIndex(i);
        g_settings->setValue("readDate", m_readDatesList[i]);
    }
    g_settings->endArray();

    saveLogEntry();
}

void Database::loadLogEntry()
{
    int size = g_settings->beginReadArray("logEntries");
    for(int i = 0; i < size; ++i) {
        g_settings->setArrayIndex(i);
        LogEntry logEntry;
        logEntry.feito = g_settings->value("feito", false).toBool();
        logEntry.obra = g_settings->value("obra").toString();
        logEntry.evento = g_settings->value("evento").toString();
        logEntry.tipo = g_settings->value("tipo").toString();
        logEntry.nome = g_settings->value("nome").toString();
        logEntry.usuario = g_settings->value("usuario").toString();
        logEntry.empresa = g_settings->value("empresa").toString();
        logEntry.hora = g_settings->value("hora").toString();
        logEntry.caminho = g_settings->value("caminho").toString();
        logEntry.data = g_settings->value("data").toString();
        logEntry.epochTime = g_settings->value("epochTime").toULongLong();
        m_logEntries.push_back(logEntry);
    }
    g_settings->endArray();

    loadNewLogEntry();
}

void Database::saveLogEntry()
{
    g_settings->beginWriteArray("logEntries");
    for(int i = 0; i < m_logEntries.size(); ++i) {
        g_settings->setArrayIndex(i);
        LogEntry logEntry = m_logEntries[i];
        g_settings->setValue("feito", logEntry.feito);
        g_settings->setValue("obra", logEntry.obra);
        g_settings->setValue("evento", logEntry.evento);
        g_settings->setValue("tipo", logEntry.tipo);
        g_settings->setValue("nome", logEntry.nome);
        g_settings->setValue("usuario", logEntry.usuario);
        g_settings->setValue("empresa", logEntry.empresa);
        g_settings->setValue("hora", logEntry.hora);
        g_settings->setValue("caminho", logEntry.caminho);
        g_settings->setValue("arquivo", logEntry.arquivo);
        g_settings->setValue("data", logEntry.data);
        g_settings->setValue("epochTime", logEntry.epochTime);
    }
    g_settings->endArray();
}

void Database::loadNewLogEntry()
{
    for(QString fileName : getFiles()) {
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
            logEntry.feito = false;
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
}

QStringList Database::getFiles()
{
    QDir dir(m_filesPath);
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

qint64 Database::getEpochTime(QString date, QString time)
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

QString Database::getDate(QString fileName)
{
    fileName = fileName.mid(fileName.lastIndexOf("_") + 1, 8);

    QString year = fileName.mid(0, 4);
    QString month = fileName.mid(4, 2);
    QString day = fileName.mid(6, 2);
    return day + "/" + month + "/" + year;
}
