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
    UChart(QWidget *parent = nullptr);
    void addData(double x, double y, int index);
    void setRefreshPeriod(int period);

signals:
    void graphClickedMsg(const QString &msg);
public slots:
    void addGraph();
    void startRefresh();

private:
    int GraphCnt;
    int RefPeriod;
    bool Pending;
    QTimer mTimer;
    QVector<QCPGraphData> mQueue[MAX_GRAPH_COUNT];
    QPointer<QCPGraph> mGraph[MAX_GRAPH_COUNT];
    QPointer<AxisTag> mTag[MAX_GRAPH_COUNT];

private slots:
    void titleDoubleClick(QMouseEvent *event);
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void hideSelectedGraph();
    void showAllGraphs();
    void contextMenuRequest(QPoint pos);
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
    void refreshPlotArea();
    void saveCurveData(const QList<QCPGraph*>& indexSeq);
    AxisTag *axisTag(int index) const
    {
        return index < GraphCnt ? mTag[index] : nullptr;
    };
};

#endif