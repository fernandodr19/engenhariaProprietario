#ifndef STATISTICSVIEW_H
#define STATISTICSVIEW_H

#include <QWidget>
#include <QDebug>
#include "logentry.h"
#include "graphdata.h"

class QLabel;
class QCustomPlot;
class QGridLayout;

enum statistic_graph {
    graph_BarChart = 0,
    graph_TimeSeries,
};

class StatisticsView : public QWidget
{
public:
    StatisticsView(statistic_graph graph);

private:
    QGridLayout *m_gridLayout;
    QLabel *m_topografiaData;
    QVector<GraphData> generateBarChartData();
    void plotBarChart();
    void plotNumbers(QCustomPlot *customPlot, QVector<int> numbers, int x, double maxHeight);
    QVector<DayInfo> generateTimeSeriesData();
    void plotTimeSeries();
    QString getDate(QString data);
    qint64 getEpochTime(const QString& date);

    QColor releasedColor = QColor(51, 153, 255, 200);
    QColor approvedColor = QColor(51, 204, 51, 200);
    QColor approvedWithCommentsColor = QColor(255, 102, 0, 200);
    QColor reprovedColor = QColor(153, 51, 51, 200);
    QColor documentsColor = QColor(150, 150, 150, 200);
};

#endif // STATISTICSVIEW_H
