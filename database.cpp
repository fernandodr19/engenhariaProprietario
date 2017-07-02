#include "database.h"
#include <QSettings>
#include <QApplication>
#include "logentry.h"

QSettings *g_settings = nullptr;

Database::Database()
{
    QCoreApplication::setOrganizationName("Fluxo Engenharia");
    QCoreApplication::setOrganizationDomain("fluxoengenharia.com.br");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setApplicationName("Controle EP");

    g_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());

    load();
}

Database::~Database()
{
    delete g_settings;
}

void Database::load()
{
    m_historicFilter = g_settings->value("historicFilter", false).toBool();
    m_releasedFilter = g_settings->value("releasedFilter", true).toBool();
    m_approvedFilter = g_settings->value("approvedFilter", false).toBool();
    m_approvedWithCommentsFilter = g_settings->value("approvedWithCommentsFilter", false).toBool();
    m_reprovedFilter = g_settings->value("reprovedFilter", false).toBool();

    QStringList *m_headersName = new QStringList({"Feito", "Obra", "Evento", "Tipo", "Arquivo", "Usuário", "Empresa", "Hora", "Caminho", "Arquivos", "Data"});//checkHere

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

    loadLogEntry();
}

void Database::save()
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

    saveLogEntry();
}

void Database::loadLogEntry()
{
    int size = g_settings->beginReadArray("logEntries");
    for (int i = 0; i < size; ++i) {
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
        logEntry.epochTime = g_settings->value("epochTime").toInt();
        m_logEntries.push_back(logEntry);
    }
    g_settings->endArray();
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
