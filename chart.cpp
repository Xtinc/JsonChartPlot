#include "chart.h"

AxisTag::AxisTag(QCPAxis *parentAxis) : QObject(parentAxis),
                                        mAxis(parentAxis)
{
    // The dummy tracer serves here as an invisible anchor which always sticks to the right side of
    // the axis rect
    mDummyTracer = new QCPItemTracer(mAxis->parentPlot());
    mDummyTracer->setVisible(false);
    mDummyTracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
    mDummyTracer->position->setTypeY(QCPItemPosition::ptPlotCoords);
    mDummyTracer->position->setAxisRect(mAxis->axisRect());
    mDummyTracer->position->setAxes(0, mAxis);
    mDummyTracer->position->setCoords(1, 0);

    mArrow = new QCPItemLine(mAxis->parentPlot());
    mArrow->setLayer("overlay");
    mArrow->setClipToAxisRect(false);
    mArrow->setHead(QCPLineEnding::esSpikeArrow);
    mArrow->end->setParentAnchor(mDummyTracer->position);
    mArrow->start->setParentAnchor(mArrow->end);
    mArrow->start->setCoords(15, 0);

    mLabel = new QCPItemText(mAxis->parentPlot());
    mLabel->setLayer("overlay");
    mLabel->setClipToAxisRect(false);
    mLabel->setPadding(QMargins(3, 0, 3, 0));
    mLabel->setBrush(QBrush(Qt::white));
    mLabel->setPen(QPen(Qt::blue));
    mLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mLabel->position->setParentAnchor(mArrow->start);
}

AxisTag::~AxisTag()
{
    if (mDummyTracer)
        mDummyTracer->parentPlot()->removeItem(mDummyTracer);
    if (mArrow)
        mArrow->parentPlot()->removeItem(mArrow);
    if (mLabel)
        mLabel->parentPlot()->removeItem(mLabel);
}

void AxisTag::setPen(const QPen &pen)
{
    mArrow->setPen(pen);
    mLabel->setPen(pen);
}

void AxisTag::setBrush(const QBrush &brush)
{
    mLabel->setBrush(brush);
}

void AxisTag::setText(const QString &text)
{
    mLabel->setText(text);
}

void AxisTag::updatePosition(double value)
{
    mDummyTracer->position->setCoords(1, value);
    mArrow->end->setCoords(mAxis->offset(), 0);
}

UChart::UChart(QWidget *parent) : QCustomPlot(parent), GraphCnt(0)
{
    yAxis->setTickLabels(true);
    yAxis2->setVisible(true);
    axisRect()->addAxis(QCPAxis::atRight);
    axisRect()->axis(QCPAxis::atRight, 0)->setPadding(30); // add some padding to have space for tags
    axisRect()->axis(QCPAxis::atRight, 1)->setPadding(30); // add some padding to have space for tags

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                    QCP::iSelectLegend | QCP::iSelectPlottables);
    // axisRect()->setupFullAxesBox();

    plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(this, "Interaction Example", QFont("sans", 17, QFont::Bold));
    plotLayout()->addElement(0, 0, title);

    xAxis->setLabel("x Axis");
    yAxis->setLabel("y Axis");
    legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    legend->setFont(legendFont);
    legend->setSelectedFont(legendFont);
    legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(this, SIGNAL(mousePress(QMouseEvent *)), this, SLOT(mousePress()));
    connect(this, SIGNAL(mouseWheel(QWheelEvent *)), this, SLOT(mouseWheel()));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));

    // connect some interaction slots:
    connect(this, SIGNAL(axisDoubleClick(QCPAxis *, QCPAxis::SelectablePart, QMouseEvent *)), this, SLOT(axisLabelDoubleClick(QCPAxis *, QCPAxis::SelectablePart)));
    connect(this, SIGNAL(legendDoubleClick(QCPLegend *, QCPAbstractLegendItem *, QMouseEvent *)), this, SLOT(legendDoubleClick(QCPLegend *, QCPAbstractLegendItem *)));
    connect(title, SIGNAL(doubleClicked(QMouseEvent *)), this, SLOT(titleDoubleClick(QMouseEvent *)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(this, SIGNAL(plottableClick(QCPAbstractPlottable *, int, QMouseEvent *)), this, SLOT(graphClicked(QCPAbstractPlottable *, int)));

    // setup policy and connect slot for context menu popup:
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

void UChart::titleDoubleClick(QMouseEvent *)
{
    if (QCPTextElement *title = qobject_cast<QCPTextElement *>(sender()))
    {
        // Set the plot title by double clicking on it
        bool ok;
        QString newTitle = QInputDialog::getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, title->text(), &ok);
        if (ok)
        {
            title->setText(newTitle);
            replot();
        }
    }
}

void UChart::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    // Set an axis label by double clicking on it
    if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "QCustomPlot example", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            replot();
        }
    }
}

void UChart::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    // Rename a graph by double clicking on its legend item
    Q_UNUSED(legend)
    if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem *>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            replot();
        }
    }
}

void UChart::selectionChanged()
{
    /*
     normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
     the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
     and the axis base line together. However, the axis label shall be selectable individually.

     The selection state of the left and right axes shall be synchronized as well as the state of the
     bottom and top axes.

     Further, we want to synchronize the selection of the graphs with the selection state of the respective
     legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
     or on its legend item.
    */

    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis) || xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }
    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (yAxis->selectedParts().testFlag(QCPAxis::spAxis) || yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i = 0; i < graphCount(); ++i)
    {
        QCPGraph *grh = graph(i);
        QCPPlottableLegendItem *item = legend->itemWithPlottable(grh);
        if (item->selected() || grh->selected())
        {
            item->setSelected(true);
            grh->setSelection(QCPDataSelection(grh->data()->dataRange()));
        }
    }
}

void UChart::mousePress()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeDrag(xAxis->orientation());
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeDrag(yAxis->orientation());
    else
        axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
}

void UChart::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeZoom(xAxis->orientation());
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeZoom(yAxis->orientation());
    else
        axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

void UChart::addRandomGraph()
{
    if (GraphCnt < 12)
    {
        mGraph[GraphCnt] = addGraph(xAxis, axisRect()->axis(QCPAxis::atRight, 0));
        mGraph[GraphCnt]->setPen(QPen(QColor( 50 * GraphCnt, 10 + 50 * GraphCnt, 50 * GraphCnt)));
        mTag[GraphCnt] = new AxisTag(mGraph[GraphCnt]->valueAxis());
        mTag[GraphCnt]->setPen(mGraph[GraphCnt]->pen());
        replot();
        GraphCnt++;
    }
}

void UChart::removeSelectedGraph()
{
    if (selectedGraphs().size() > 0)
    {
        removeGraph(selectedGraphs().first());
        replot();
    }
}

void UChart::removeAllGraphs()
{
    clearGraphs();
    replot();
}

void UChart::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
    }
    else // general context menu on graphs requested
    {
        menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
        if (selectedGraphs().size() > 0)
            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        if (graphCount() > 0)
            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
    }

    menu->popup(mapToGlobal(pos));
}

void UChart::moveLegend()
{
    if (QAction *contextAction = qobject_cast<QAction *>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            replot();
        }
    }
}

void UChart::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
    // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
}