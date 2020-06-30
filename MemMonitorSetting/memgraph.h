#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>

#include "ui_memgraph.h"

class MemGraph : public QWidget
{
    Q_OBJECT

public:
    MemGraph(QWidget *parent = Q_NULLPTR);
    ~MemGraph();

    void setPID(unsigned long pid);
public slots:
    void setData(const std::vector<double>& data);

    void appendData(unsigned long p, double d);
private:
    Ui::memgraph ui;

    unsigned long _pid;
    QtCharts::QChartView* _view;
    QtCharts::QSplineSeries* series;

    double _min_y = 10000;
    double _max_y = 0;
};
