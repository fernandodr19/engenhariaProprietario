#include "statisticsview.h"
#include "database.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include <QSet>
#include <QLinearGradient>
#include <thirdparty/qcustomplot/qcustomplot.h>

extern Database *g_database;

StatisticsView::StatisticsView()
{
    m_gridLayout = new QGridLayout();
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumWidth(1000);
    setMinimumHeight(800);
    setLayout(m_gridLayout);

//    QLabel *topografia = new QLabel("02 - Topografia");
//    m_topografiaData = new QLabel();

    update();
}

void StatisticsView::update()
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

//    graphs[1].bars[5].print();
    plotGraph(graphs[0]);
    plotGraph(graphs[1]);
}

void StatisticsView::plotGraph(const GraphData& graph)
{
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
    released->setPen(QPen(QColor(51, 153, 255).lighter(130)));
    released->setBrush(QColor(51, 153, 255));
    approved->setName("Aprovado Cliente");
    approved->setPen(QPen(QColor(51, 204, 51).lighter(150)));
    approved->setBrush(QColor(51, 204, 51));
    approvedWithComments->setName("Aprovado Cliente c/ Ressalvas");
    approvedWithComments->setPen(QPen(QColor(0, 102, 0).lighter(150)));
    approvedWithComments->setBrush(QColor(0, 102, 0));
    reproved->setName("Reprovado Cliente");
    reproved->setPen(QPen(QColor(153, 51, 51).lighter(170)));
    reproved->setBrush(QColor(153, 51, 51));
    documents->setName("Lista de Documentos");
    documents->setPen(QPen(QColor(150, 150, 150).lighter(170)));
    documents->setBrush((QColor(150, 150, 150)));
    // stack bars on top of each other:
    approved->moveAbove(released);
    approvedWithComments->moveAbove(approved);
    reproved->moveAbove(approvedWithComments);
    documents->moveAbove(reproved);

    // prepare x axis with country labels:
    QVector<double> ticks;
    QVector<QString> labels;

    QVector<GraphBar> bars = graph.bars;
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
    customPlot->yAxis->setRange(0, maxHeight);
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

//    int row = 0;
    m_gridLayout->addWidget(new QLabel(graph.name), row++, 0, 1, 1, Qt::AlignCenter);
    m_gridLayout->addWidget(customPlot, row++, 0);
}

