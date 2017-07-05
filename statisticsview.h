#ifndef STATISTICSVIEW_H
#define STATISTICSVIEW_H

#include <QWidget>

class QLabel;

class StatisticsView : public QWidget
{
public:
    StatisticsView();

    void update();

private:
    QLabel *m_topografiaData;
};

#endif // STATISTICSVIEW_H
