#include "graphdata.h"

GraphData::GraphData(const QString& _name, const QString& _filter, const QStringList& paths) :
    name(_name), filter(_filter)
{
    for(const QString& path : paths) {
        GraphBar bar;
        bar.name = path;
        bars.push_back(bar);
    }
}

void GraphBar::update(const LogEntry &file)
{
    if(file.evento == "Liberado para Cliente")
        liberado++;
    else if(file.evento == "Aprovado Cliente")
        aprovado++;
    else if(file.evento == "Aprovado Cliente c/ Ressalvas")
        aprovadoRessalva++;
    else if(file.evento == "Reprovado Cliente")
        reprovado++;
    else if(file.evento == "Lista de Documentos")
        listado++;
    else if(file.evento == "Transferindo para Versão")
        listado++;
}

DayInfo::DayInfo(const QString &data) :
    data(data)
{
}

void DayInfo::update(const LogEntry &file)
{
    if(file.evento == "Liberado para Cliente")
        liberado++;
    else if(file.evento == "Aprovado Cliente")
        aprovado++;
    else if(file.evento == "Aprovado Cliente c/ Ressalvas")
        aprovadoRessalva++;
    else if(file.evento == "Reprovado Cliente")
        reprovado++;
    else if(file.evento == "Lista de Documentos")
        listado++;
    else if(file.evento == "Transferindo para Versão")
        listado++;
}
