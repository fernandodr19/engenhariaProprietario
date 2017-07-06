#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include "logentry.h"
#include <QVector>

struct GraphBar
{
    QString name;
    int liberado = 0;
    int aprovado = 0;
    int aprovadoRessalva = 0;
    int reprovado = 0;
    int listado = 0;

    void update(const LogEntry& file);
};

struct DayInfo {
    DayInfo() {}
    DayInfo(const QString& data);
    QString data;
    int liberado = 0;
    int aprovado = 0;
    int aprovadoRessalva = 0;
    int reprovado = 0;
    int listado = 0;

    void update(const LogEntry& file);
};

class GraphData
{
public:
    GraphData() {}
    GraphData(const QString& _name, const QString& _filter, const QStringList& paths);

    QString name;
    QString filter;
    QVector<GraphBar> bars;
};

#endif // GRAPHDATA_H
