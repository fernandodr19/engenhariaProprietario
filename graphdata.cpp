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
    if(file.event == "Liberado para Cliente")
        liberado++;
    else if(file.event == "Aprovado Cliente")
        aprovado++;
    else if(file.event == "Aprovado Cliente c/ Ressalvas" || file.event == "Aprovado Cliente c/ Ressalva")
        aprovadoRessalva++;
    else if(file.event == "Reprovado Cliente")
        reprovado++;
    else if(file.event == "Lista de Documentos")
        listado++;
    else if(file.event == "Transferindo para Versão")
        listado++;
}

DayInfo::DayInfo(const QString &data) :
    data(data)
{
}

void DayInfo::update(const LogEntry &file)
{
    if(file.event == "Liberado para Cliente")
        liberado++;
    else if(file.event == "Aprovado Cliente")
        aprovado++;
    else if(file.event == "Aprovado Cliente c/ Ressalvas" || file.event == "Aprovado Cliente c/ Ressalva")
        aprovadoRessalva++;
    else if(file.event == "Reprovado Cliente")
        reprovado++;
    else if(file.event == "Lista de Documentos")
        listado++;
    else if(file.event == "Transferindo para Versão")
        listado++;
}
