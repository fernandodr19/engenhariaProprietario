#ifndef STATISTICSVIEW_H
#define STATISTICSVIEW_H

#include <QWidget>
#include <QDebug>
#include "logentry.h"

class QLabel;
class QCustomPlot;
class QGridLayout;

struct GraphBar {
    QString name;
    int liberado = 0;
    int aprovado = 0;
    int aprovadoRessalva = 0;
    int reprovado = 0;
    int listado = 0;

    void update(const LogEntry& file);
};

struct GraphData {
    GraphData() {}
    GraphData(const QString& _name, const QString& _filter, const QStringList& paths) :
        name(_name), filter(_filter) {
        for(const QString& path : paths) {
            GraphBar bar;
            bar.name = path;
            bars.push_back(bar);
        }
    }

    QString name;
    QString filter;
    QVector<GraphBar> bars;
};

class StatisticsView : public QWidget
{
public:
    StatisticsView();

    void update();

private:
    QGridLayout *m_gridLayout;
    QLabel *m_topografiaData;
    void plotGraph(const GraphData& graph, int row);
};

#endif // STATISTICSVIEW_H
