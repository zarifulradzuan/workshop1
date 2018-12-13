#include "linechart.h"
#include <QtCharts>
lineChart::lineChart(QWidget *parent) : QWidget(parent)
{
        QLineSeries *series = new QLineSeries();
        q.exec("select i.dateDone,sum(i.amountPaid) from invoice i group by i.dateDone order by i.dateDone");
        int i=0;
        QStringList dates;
        while(q.next()){
            series->append(i,q.value(1).toDouble());
            dates<<q.value(0).toString();
            i++;
        }
        QChart *chart = new QChart();
        chart->legend()->hide();
        chart->addSeries(series);
        chart->createDefaultAxes();
        chart->axisX()->setTitleText("Date");
        chart->axisY()->setTitleText("RM");
        chart->axisY()->setRange(0,5000);
        chart->setTitle("Simple line chart example");



        QChartView chartView(this);
        chartView.setRenderHint(QPainter::Antialiasing);
}
