#ifndef CHART_H
#define CHART_H

#include "qcustomplot.h"
#include <QObject>

constexpr int MAX_GRAPH_COUNT = 12;
constexpr int MAX_CURVE_COUNT = 100;

class AxisTag : public QObject
{
    Q_OBJECT
public:
    explicit AxisTag(QCPAxis *parentAxis);
    virtual ~AxisTag();

    // setters:
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setText(const QString &text);

    // getters:
    QPen pen() const { return mLabel->pen(); }
    QBrush brush() const { return mLabel->brush(); }
    QString text() const { return mLabel->text(); }

    // other methods:
    void updatePosition(double value);

protected:
    QCPAxis *mAxis;
    QPointer<QCPItemTracer> mDummyTracer;
    QPointer<QCPItemLine> mArrow;
    QPointer<QCPItemText> mLabel;
};

class UChart : public QCustomPlot
{
    Q_OBJECT
public:
    UChart(QWidget *parent = nullptr,const QString &title = "UChart", const QString &xtitle = "xAxis", const QString &ytitle = "yAxis");
    void addData(double x, double y, int index);
    void addGraph();
    void setMaxCurveCount(int counts);
    void setRefreshPeriod(int period);
    void setTitle(const QString &str)
    {
        title.setText(str);
    }
    void setXAxisTitle(const QString &title)
    {
        xAxis->setLabel(title);
    }
    void setYAxisTitle(const QString &title)
    {
        yAxis->setLabel(title);
    }

signals:
    void graphClickedMsg(const QString &msg);

private:
    int GraphCnt;
    int RefPeriod;
    int CurveCnt;
    bool Pending;
    QTimer mTimer;
    QCPTextElement title;
    QVector<QCPGraphData> mQueue[MAX_GRAPH_COUNT];
    QPointer<QCPGraph> mGraph[MAX_GRAPH_COUNT];
    QPointer<AxisTag> mTag[MAX_GRAPH_COUNT];

private:
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void contextMenuRequest(QPoint pos);
    void constructLegendMenu(QMenu *menu);
    void constructNormalMenu(QMenu *menu);
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
    void hideSelectedGraph();
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void mousePress();
    void mouseWheel();
    void moveLegend(int flag);
    void refreshPlotArea();
    void saveCurveData(const QList<QCPGraph *> &indexSeq);
    void selectionChanged();
    void showAllGraphs();
    void titleDoubleClick(QMouseEvent *event);

    AxisTag *axisTag(int index) const
    {
        return index < GraphCnt ? mTag[index] : nullptr;
    };
};

#endif