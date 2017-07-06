#include "statisticsview.h"
#include "database.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include <QSet>
#include <QLinearGradient>
#include <thirdparty/qcustomplot/qcustomplot.h>

extern Database *g_database;

StatisticsView::StatisticsView(statistic_graph graph)
{
    m_gridLayout = new QGridLayout();
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumWidth(1000);
    setMinimumHeight(800);
    setLayout(m_gridLayout);

    switch (graph) {
    case graph_BarChart:
        plotBarChart();
        break;
    case graph_TimeSeries:
        plotTimeSeries();
        break;
    }
}

QVector<GraphData> StatisticsView::generateBarChartData()
{
    QSet<QString> basePaths;
    const QMap<QString, LogEntry>& files = g_database->getActiveFiles();
    for(auto it = files.begin(); it != files.end(); ++it) {
        const LogEntry& file = it.value();
        QStringList paths = file.caminho.split("\\");
        if(paths.size() >= 2 && !paths.at(1).isEmpty())
            basePaths.insert(paths.at(1));
    }
    QStringList basePathsList = basePaths.toList();
    std::sort(basePathsList.begin(), basePathsList.end());

    QVector<GraphData> graphs;
    graphs.push_back(GraphData("Topografia", "02 - Topografia", basePathsList));
    graphs.push_back(GraphData("Civil", "06 - Projeto Executivo\\Civil", basePathsList));

    QMap<QString, LogEntry> realFiles;
    QRegExp rx("(-R[0-9|L][0-9|D|A-Z])");

    for(auto it = files.begin(); it != files.end(); ++it) {
        QString name = it.key();
        name.remove(rx);

        auto cit = realFiles.find(name);
        if(cit != realFiles.end()) {
            if(cit.value().epochTime < it.value().epochTime)
                realFiles[name] = it.value();
        }
        else
            realFiles[name] = it.value();
    }

    for(GraphData& graph : graphs) {
        for(GraphBar& bar : graph.bars) {
            for(auto it = realFiles.begin(); it != realFiles.end(); ++it) {
                const LogEntry& file = it.value();
                if(file.caminho.contains(bar.name) && file.caminho.contains(graph.filter))
                    bar.update(file);
            }
        }
    }
    return graphs;
}

void StatisticsView::plotBarChart()
{
    int row = 0;
    for(const GraphData& graph : generateBarChartData()) {
        QCustomPlot *customPlot = new QCustomPlot();
        customPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // set dark background gradient:
        QLinearGradient gradient(0, 0, 0, 400);
        gradient.setColorAt(0, QColor(90, 90, 90));
        gradient.setColorAt(0.38, QColor(105, 105, 105));
        gradient.setColorAt(1, QColor(70, 70, 70));
        customPlot->setBackground(QBrush(gradient));

        // create empty bar chart objects:
        QCPBars *released = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        QCPBars *approved = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        QCPBars *approvedWithComments = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        QCPBars *reproved = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        QCPBars *documents = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        released->setAntialiased(false); // gives more crisp, pixel aligned bar borders
        approved->setAntialiased(false);
        approvedWithComments->setAntialiased(false);
        reproved->setAntialiased(false);
        documents->setAntialiased(false);
        released->setStackingGap(1);
        approved->setStackingGap(1);
        approvedWithComments->setStackingGap(1);
        reproved->setStackingGap(1);
        documents->setStackingGap(1);
        // set names and colors:
        released->setName("Liberado para Cliente");
        released->setPen(QPen(releasedColor.lighter(130)));
        released->setBrush(releasedColor);
        approved->setName("Aprovado Cliente");
        approved->setPen(QPen(approvedColor.lighter(150)));
        approved->setBrush(approvedColor);
        approvedWithComments->setName("Aprovado Cliente c/ Ressalvas");
        approvedWithComments->setPen(QPen(approvedWithCommentsColor.lighter(150)));
        approvedWithComments->setBrush(approvedWithCommentsColor);
        reproved->setName("Reprovado Cliente");
        reproved->setPen(QPen(reprovedColor.lighter(170)));
        reproved->setBrush(reprovedColor);
        documents->setName("Lista de Documentos");
        documents->setPen(QPen(documentsColor.lighter(170)));
        documents->setBrush((documentsColor));
        // stack bars on top of each other:
        approved->moveAbove(released);
        approvedWithComments->moveAbove(approved);
        reproved->moveAbove(approvedWithComments);
        documents->moveAbove(reproved);

        // prepare x axis with country labels:
        QVector<double> ticks;
        QVector<QString> labels;

        const QVector<GraphBar>& bars = graph.bars;
        for(int i = 0; i < bars.size(); i++) {
            ticks.push_back(i + 1);
            labels.push_back(bars[i].name);
        }

        // Add data:
        QVector<double> releasedData, approvedData, approvedWithCommentsData, reprovedData, documentsData;

        double maxHeight = 0;
        for(const GraphBar& bar : bars) {
            releasedData.push_back(bar.liberado);
            approvedData.push_back(bar.aprovado);
            approvedWithCommentsData.push_back(bar.aprovadoRessalva);
            reprovedData.push_back(bar.reprovado);
            documentsData.push_back(bar.listado);
            double height = bar.liberado + bar.aprovado + bar.aprovadoRessalva + bar.reprovado + bar.listado;
            if(height > maxHeight)
                maxHeight = height;
        }

        released->setData(ticks, releasedData);
        approved->setData(ticks, approvedData);
        approved->setData(ticks, approvedData);
        reproved->setData(ticks, reprovedData);
        documents->setData(ticks, documentsData);

        QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
        textTicker->addTicks(ticks, labels);
        customPlot->xAxis->setTicker(textTicker);
        customPlot->xAxis->setTickLabelRotation(60);
        customPlot->xAxis->setSubTicks(false);
        customPlot->xAxis->setTickLength(0, 4);
        customPlot->xAxis->setRange(0, ticks.size() + 1);
        customPlot->xAxis->setBasePen(QPen(Qt::white));
        customPlot->xAxis->setTickPen(QPen(Qt::white));
        customPlot->xAxis->grid()->setVisible(true);
        customPlot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
        customPlot->xAxis->setTickLabelColor(Qt::white);
        customPlot->xAxis->setLabelColor(Qt::white);

        // prepare y axis:
        customPlot->yAxis->setRange(0, std::max(12.1, maxHeight));
        customPlot->yAxis->setPadding(5); // a bit more space to the left border
        customPlot->yAxis->setLabel("Número de registros???");
        customPlot->yAxis->setBasePen(QPen(Qt::white));
        customPlot->yAxis->setTickPen(QPen(Qt::white));
        customPlot->yAxis->setSubTickPen(QPen(Qt::white));
        customPlot->yAxis->grid()->setSubGridVisible(true);
        customPlot->yAxis->setTickLabelColor(Qt::white);
        customPlot->yAxis->setLabelColor(Qt::white);
        customPlot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
        customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));


        // setup legend:
        customPlot->legend->setVisible(true);
        customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
        customPlot->legend->setBrush(QColor(255, 255, 255, 100));
        customPlot->legend->setBorderPen(Qt::NoPen);
        QFont legendFont = font();
        legendFont.setPointSize(10);
        customPlot->legend->setFont(legendFont);
        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

        //title
        customPlot->plotLayout()->insertRow(0);
        QFont font("sans", 17, QFont::Bold);
        QCPTextElement *title = new QCPTextElement(customPlot, graph.name, font);
        title->setTextColor(Qt::white);
        customPlot->plotLayout()->addElement(0,0, title);

        m_gridLayout->addWidget(customPlot, row++, 0);
    }
}

QVector<DayInfo> StatisticsView::generateTimeSeriesData()
{
    QVector<DayInfo> timeSeriesData;
    for(const QString& date : g_database->getReadDates())
        timeSeriesData.push_back(DayInfo(date));

    std::sort(timeSeriesData.begin(), timeSeriesData.end(), [this](const DayInfo& a, const DayInfo& b) {
        return a.data < b.data;
    });

    for(int i = 0; i < timeSeriesData.size(); i++)
        timeSeriesData[i].data = getDate(timeSeriesData[i].data);

    for(const LogEntry& logEntry : g_database->getHistoricFiles()) {
        for(DayInfo& dayInfo : timeSeriesData) {
            if(dayInfo.data == logEntry.data) {
                dayInfo.update(logEntry);
                break;
            }
        }
    }
    return timeSeriesData;
}

void StatisticsView::plotTimeSeries()
{
    QCustomPlot *customPlot = new QCustomPlot();

    // set locale to english, so we get english month names:
    customPlot->setLocale(QLocale(QLocale::Portuguese, QLocale::Brazil));

    QVector<DayInfo> timeSeriesData = generateTimeSeriesData();
    if(timeSeriesData.isEmpty())
        return;

    QVector<QCPGraphData> releasedData(timeSeriesData.size());
    QVector<QCPGraphData> approvedData(timeSeriesData.size());
    QVector<QCPGraphData> approvedWithCommentsData(timeSeriesData.size());
    QVector<QCPGraphData> reprovedData(timeSeriesData.size());

    int i = 0;
    double maxValue = 0;
    for(const DayInfo& dayInfo : timeSeriesData) {
        releasedData[i].key = getEpochTime(dayInfo.data);
        releasedData[i].value = dayInfo.liberado;
        //        if(dayInfo.liberado > maxValue)
        //            maxValue = dayInfo.liberado;

        approvedData[i].key = getEpochTime(dayInfo.data);
        approvedData[i].value = dayInfo.aprovado;
        if(dayInfo.aprovado > maxValue)
            maxValue = dayInfo.aprovado;

        approvedWithCommentsData[i].key = getEpochTime(dayInfo.data);
        approvedWithCommentsData[i].value = dayInfo.aprovadoRessalva;
        if(dayInfo.aprovadoRessalva > maxValue)
            maxValue = dayInfo.aprovadoRessalva;

        approvedWithCommentsData[i].key = getEpochTime(dayInfo.data);
        approvedWithCommentsData[i].value = dayInfo.aprovadoRessalva;
        if(dayInfo.aprovadoRessalva > maxValue)
            maxValue = dayInfo.aprovadoRessalva;

        reprovedData[i].key = getEpochTime(dayInfo.data);
        reprovedData[i].value = dayInfo.reprovado;
        if(dayInfo.reprovado > maxValue)
            maxValue = dayInfo.reprovado;
        i++;
    }

    int index = 0;
    customPlot->addGraph();
    customPlot->graph(index)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(index)->setPen(QPen(releasedColor.lighter(200)));
    customPlot->graph(index)->setBrush(QBrush(releasedColor));
    customPlot->graph(index)->setName("Liberado para Cliente");
    customPlot->graph(index++)->data()->set(releasedData);
    customPlot->addGraph();
    customPlot->graph(index)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(index)->setPen(QPen(approvedColor.lighter(200)));
    customPlot->graph(index)->setBrush(QBrush(approvedColor));
    customPlot->graph(index)->setName("Aprovado Cliente");
    customPlot->graph(index++)->data()->set(approvedData);
    customPlot->addGraph();
    customPlot->graph(index)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(index)->setPen(QPen(approvedWithCommentsColor.lighter(200)));
    customPlot->graph(index)->setBrush(QBrush(approvedWithCommentsColor));
    customPlot->graph(index)->setName("Aprovado Cliente c/ Ressalvas");
    customPlot->graph(index++)->data()->set(approvedWithCommentsData);
    customPlot->addGraph();
    customPlot->graph(index)->setLineStyle(QCPGraph::lsLine);
    customPlot->graph(index)->setPen(QPen(reprovedColor.lighter(200)));
    customPlot->graph(index)->setBrush(QBrush(reprovedColor));
    customPlot->graph(index)->setName("Reprovado Cliente");
    customPlot->graph(index++)->data()->set(reprovedData);

    // configure bottom axis to show date instead of number:
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("d. MMMM\nyyyy");
    customPlot->xAxis->setTicker(dateTicker);
    // set axis labels:
    customPlot->xAxis->setLabel("Data");
    customPlot->yAxis->setLabel("Quantidade de registro");
    // make top and right axes visible but without ticks and labels:
    customPlot->xAxis2->setVisible(true);
    customPlot->yAxis2->setVisible(true);
    customPlot->xAxis2->setTicks(false);
    customPlot->yAxis2->setTicks(false);
    customPlot->xAxis2->setTickLabels(false);
    customPlot->yAxis2->setTickLabels(true);
    // set axis ranges to show all data:
    customPlot->xAxis->setRange(getEpochTime(timeSeriesData[0].data), getEpochTime(timeSeriesData[timeSeriesData.size() - 1].data));
    customPlot->yAxis->setRange(0, maxValue);
    // show legend with slightly transparent background brush:
    customPlot->legend->setVisible(true);
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    customPlot->legend->setBrush(QColor(255, 255, 255, 150));

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    //title
    customPlot->plotLayout()->insertRow(0);
    QFont font("sans", 17, QFont::Bold);
    QCPTextElement *title = new QCPTextElement(customPlot, "Fluxo de arquivos por dia", font);
    title->setTextColor(Qt::black);
    customPlot->plotLayout()->addElement(0,0, title);

    m_gridLayout->addWidget(customPlot, 0, 0);
}

QString StatisticsView::getDate(QString date)
{
    QString year = date.mid(0, 4);
    QString month = date.mid(4, 2);
    QString day = date.mid(6, 2);
    return day + "/" + month + "/" + year;
}

qint64 StatisticsView::getEpochTime(const QString& date)
{
    if(date.length() < 10)
        return -1;

    QStringList splitedDate = date.split("/");
    int year = splitedDate[2].toInt();
    int month = splitedDate[1].toInt();
    int day = splitedDate[0].toInt();

    QDateTime dateTime(QDate(year, month, day));
    return dateTime.toMSecsSinceEpoch()/1000;
}
