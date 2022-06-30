#ifndef CHART_H
#define CHART_H

#include "qcustomplot.h"
#include <QObject>

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
    AxisTag *axisTag(int index) const
    {
        return index < GraphCnt ? mTag[index] : nullptr;
    };
public slots:
    void addRandomGraph();

private:
    QPointer<QCPGraph> mGraph[12];
    QPointer<AxisTag> mTag[12];
    int GraphCnt;

private slots:
    void titleDoubleClick(QMouseEvent *event);
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void removeSelectedGraph();
    void removeAllGraphs();
    void contextMenuRequest(QPoint pos);
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
};

#endif