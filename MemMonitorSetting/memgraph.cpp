#include "memgraph.h"
#include <QtCharts/QChart>
#include <QGridLayout>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

MemGraph::MemGraph(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    _view = new QChartView(this);
    _view->setRenderHint(QPainter::Antialiasing);
    layout()->addWidget(_view);

    QChart* chart = new QChart();
    chart->legend()->hide();
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setContentsMargins(0, 0, 0, 0);
    _view->setChart(chart);

    chart->createDefaultAxes();
    QValueAxis* x = new QValueAxis();
    x->setRange(0, 400);
    x->setGridLineVisible(false);

    QValueAxis* y = new QValueAxis();
    //y->setTickCount(10);
 //   y->setRange(0, 100);
    y->setGridLineVisible(false);

    chart->addAxis(x, Qt::AlignBottom);
    chart->addAxis(y, Qt::AlignLeft);
    
    series = new QSplineSeries();
    chart->addSeries(series);
    series->attachAxis(x);
    series->attachAxis(y);
    series->useOpenGL();
    
    setAttribute(Qt::WA_DeleteOnClose);
}

MemGraph::~MemGraph()
{
}

void MemGraph::setPID(unsigned long pid)
{
    _pid = pid;
    _min_y = 100000;
    _max_y = 0;
}

void MemGraph::setData(const std::vector<double>& data)
{
    QVector<QPointF> newd;
    for (int i = 0; i < data.size(); i++) {
        if (data[i] > _max_y) {
            _max_y = data[i];
        }

        if (data[i] < _min_y)
            _min_y = data[i];
        newd.push_back(QPointF(i, data[i]));
    }
    _view->chart()->axisY()->setRange(_min_y - 1 > 0 ? _min_y - 1 : 0, _max_y + 1);
    series->clear();
    series->replace(newd);
}

void MemGraph::appendData(unsigned long pid, double d)
{
    if (pid != _pid)
        return;

    if (d > _max_y) {
        _max_y = d;
    }

    if (d < _min_y)
        _min_y = d;
    _view->chart()->axisY()->setRange(_min_y - 1 > 0 ? _min_y - 1 : 0, _max_y + 1);

    auto vec = series->points();
    std::vector<QPointF> copyvec;

    if (vec.size() > 400)
        for (int i = 0; i < 399; i++)
            copyvec.push_back(QPointF(i, vec[vec.size() - 400 + i].y()));
    else
        copyvec.assign(vec.begin(), vec.end());

    copyvec.push_back(QPointF(vec.size(), d));
    series->clear();
    series->replace(QVector<QPointF>::fromStdVector(copyvec));
    _view->update();
}