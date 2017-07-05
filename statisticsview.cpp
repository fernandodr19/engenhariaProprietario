#include "statisticsview.h"
#include "database.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include <QSet>

extern Database *g_database;

StatisticsView::StatisticsView()
{
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);

    int row = 0;

    QLabel *topografia = new QLabel("02 - Topografia");
    m_topografiaData = new QLabel();
    layout->addWidget(topografia, row, 0);
    layout->addWidget(m_topografiaData, row++, 1);

    update();
}

void StatisticsView::update()
{
    struct GraphBar {
        QString name;
        int liberado = 0;
        int aprovado = 0;
        int aprovadoRessalva = 0;
        int reprovado = 0;
        int listado = 0;

        void update(const LogEntry& file) {
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

        void print() {
            qDebug() << liberado << aprovado << aprovadoRessalva << reprovado << listado;
        }
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

    graphs[1].bars[5].print();
}

