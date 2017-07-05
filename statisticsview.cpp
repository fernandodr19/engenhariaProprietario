#include "statisticsview.h"
#include "database.h"
#include <QGridLayout>
#include <QLabel>

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
}

void StatisticsView::update()
{

}
