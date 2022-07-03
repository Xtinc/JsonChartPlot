#include "chart.h"

QColor getColorByIndex(int index)
{
    QColor color;
    switch (index)
    {
    case 0:
        color = Qt::black;
        break;
    case 1:
        color = Qt::cyan;
        break;
    case 2:
        color = Qt::darkCyan;
        break;
    case 3:
        color = Qt::red;
        break;
    case 4:
        color = Qt::darkRed;
        break;
    case 5:
        color = Qt::magenta;
        break;
    case 6:
        color = Qt::darkMagenta;
        break;
    case 7:
        color = Qt::green;
        break;
    case 8:
        color = Qt::darkGreen;
        break;
    case 9:
        color = Qt::yellow;
        break;
    case 10:
        color = Qt::darkYellow;
        break;
    case 11:
        color = Qt::blue;
        break;
    default:
        color = Qt::darkBlue;
        break;
    }
    return color;
}

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

UChart::UChart(QWidget *parent, const QString &str, const QString &xtitle, const QString &ytitle)
    : QCustomPlot(parent), GraphCnt(0), RefPeriod(80), CurveCnt(MAX_CURVE_COUNT), Pending(false), title(this, str)
{
    yAxis->setTickLabels(true);
    yAxis2->setVisible(true);

    axisRect()->axis(QCPAxis::atRight, 0)->setPadding(60);

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                    QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect);

    plotLayout()->insertRow(0);
    plotLayout()->addElement(0, 0, &title);

    legend->setVisible(true);
    legend->setSelectableParts(QCPLegend::spItems);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QCustomPlot::selectionChangedByUser, this, &UChart::selectionChanged);
    connect(this, &QCustomPlot::mousePress, this, &UChart::mousePress);
    connect(this, &QCustomPlot::mouseWheel, this, &UChart::mouseWheel);
    connect(xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), xAxis2, QOverload<const QCPRange &>::of(&QCPAxis::setRange));
    connect(yAxis2, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), yAxis, QOverload<const QCPRange &>::of(&QCPAxis::setRange));
    connect(this, &QCustomPlot::axisDoubleClick, this, &UChart::axisLabelDoubleClick);
    connect(this, &QCustomPlot::legendDoubleClick, this, &UChart::legendDoubleClick);
    connect(&title, &QCPTextElement::doubleClicked, this, &UChart::titleDoubleClick);
    connect(this, &UChart::plottableClick, this, &UChart::graphClicked);
    connect(this, &UChart::customContextMenuRequested, this, &UChart::contextMenuRequest);
    connect(&mTimer, &QTimer::timeout, this, &UChart::refreshPlotArea);

    xAxis->setLabel(xtitle);
    yAxis->setLabel(ytitle);

    mTimer.start(100);
}

void UChart::addData(double x, double y, int index)
{
    if (index < 0 || index > CurveCnt - 1)
    {
        return;
    }
    if (mQueue[index].size() > CurveCnt)
    {
        mQueue[index].removeFirst();
    }
    mQueue[index].push_back(QCPGraphData(x, y));
}

void UChart::setRefreshPeriod(int period)
{
    RefPeriod = period;
}

void UChart::titleDoubleClick(QMouseEvent *)
{
    if (QCPTextElement *title = qobject_cast<QCPTextElement *>(sender()))
    {
        // Set the plot title by double clicking on it
        bool ok;
        QString newTitle = QInputDialog::getText(this, "UChart", "New plot title:", QLineEdit::Normal, title->text(), &ok);
        if (ok)
        {
            title->setText(newTitle);
            replot();
        }
    }
}

void UChart::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "UChart", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            replot();
        }
    }
}

void UChart::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    Q_UNUSED(legend)
    if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem *>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "UChart", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            replot();
        }
    }
}

void UChart::selectionChanged()
{

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
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    {
        axisRect()->setRangeDrag(xAxis->orientation());
    }
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    {
        axisRect()->setRangeDrag(yAxis->orientation());
    }
    else
    {
        axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    }
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

void UChart::addGraph()
{
    if (GraphCnt < MAX_GRAPH_COUNT)
    {
        mGraph[GraphCnt] = QCustomPlot::addGraph(xAxis, axisRect()->axis(QCPAxis::atRight, 0));
        mGraph[GraphCnt]->setPen(QPen(getColorByIndex(GraphCnt)));
        mTag[GraphCnt] = new AxisTag(mGraph[GraphCnt]->valueAxis());
        mTag[GraphCnt]->setPen(mGraph[GraphCnt]->pen());
        replot();
        GraphCnt++;
    }
}

void UChart::hideSelectedGraph()
{
    if (selectedGraphs().size() > 0)
    {
        selectedGraphs().first()->setVisible(false);
        replot();
    }
}

void UChart::showAllGraphs()
{
    for (int i = 0; i < graphCount(); ++i)
    {
        graph(i)->setVisible(true);
    }
    replot();
}

void UChart::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (legend->selectTest(pos, false) >= 0)
    {
        constructLegendMenu(menu);
    }
    else
    {
        constructNormalMenu(menu);
    }

    menu->popup(mapToGlobal(pos));
}

void UChart::moveLegend(int orientation)
{
    axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)orientation);
    replot();
}

void UChart::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
    // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
    emit graphClickedMsg(message);
}

void UChart::refreshPlotArea()
{
    if (!Pending)
    {
        for (int i = 0; i < graphCount(); ++i)
        {
            QCPGraph *grh = graph(i);
            AxisTag *tag = axisTag(i);
            grh->data()->set(mQueue[i], false);
            grh->rescaleValueAxis(false, true);
            if (grh->dataCount() > 0)
            {
                double graph1Value = grh->dataMainValue(grh->dataCount() - 1);
                tag->updatePosition(graph1Value);
                tag->setText(QString::number(graph1Value, 'f', 2));
            }
            if (i == 0)
            {
                xAxis->rescale();
                yAxis2->rescale();
            }
            else
            {
                yAxis2->rescale(true);
                xAxis->rescale(true);
            }
        }
        xAxis->setRange(xAxis->range().upper, 100, Qt::AlignRight);
        replot();
    }
    mTimer.start(RefPeriod);
}

void UChart::saveCurveData(const QList<QCPGraph *> &idxseq)
{
    auto filename = QFileDialog::getSaveFileName(this, "Save Plot data", "./", "Text file(*.txt)");
    if (filename.isEmpty())
    {
        return;
    }
    QFile qfile(filename);
    if (!qfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
    QTextStream qstream(&qfile);
    qstream.setFieldWidth(8);
    qstream.setNumberFlags(QTextStream::ForceSign);
    qstream.setRealNumberNotation(QTextStream::ScientificNotation);
    for (const auto &i : idxseq)
    {
        qstream << i->name() << "     ";
    }
    qstream << "\n";
    int maxCount = 0;
    for (const auto &i : idxseq)
    {
        maxCount = qMax(maxCount, i->data()->size());
    }
    for (auto i = 0; i < maxCount; ++i)
    {
        for (const auto &j : idxseq)
        {
            auto iter = j->data()->at(i);
            auto iend = j->data()->end();
            if (iter != iend)
            {
                qstream << iter->key << " " << iter->value << " ";
            }
        }
        qstream << "\n";
    }
    qfile.close();
}

void UChart::constructLegendMenu(QMenu *menu)
{
    menu->addAction("Move to top left", this, [&]()
                    { moveLegend((int)(Qt::AlignTop | Qt::AlignLeft)); });
    menu->addAction("Move to top center", this, [&]()
                    { moveLegend((int)(Qt::AlignTop | Qt::AlignHCenter)); });
    menu->addAction("Move to top right", this, [&]()
                    { moveLegend((int)(Qt::AlignTop | Qt::AlignRight)); });
    menu->addAction("Move to bottom right", this, [&]()
                    { moveLegend((int)(Qt::AlignBottom | Qt::AlignRight)); });
    menu->addAction("Move to bottom left", this, [&]()
                    { moveLegend((int)(Qt::AlignBottom | Qt::AlignLeft)); });
}

void UChart::constructNormalMenu(QMenu *menu)
{
    menu->addAction("Add graph", this, [&]()
                    { addGraph(); });
    menu->addAction("Show all graphs", this, [&]()
                    { showAllGraphs(); });
    if (selectedGraphs().size() > 0)
    {
        menu->addAction("Hide selected graph", this, [&]()
                        { hideSelectedGraph(); });
        menu->addAction("Save Selected Datas", this, [&]()
                        { saveCurveData(selectedGraphs()); });
    }
    if (graphCount() > 0)
    {
        menu->addAction("Pending", this, [&]()
                        { Pending = !Pending; });
        menu->addAction("Save graph", this, [&]()
                        {
            auto filename = QFileDialog::getSaveFileName(this, "Save graph", "./", "Graph file(*.png)");
            if(filename.isEmpty()){
                return;
            }
            savePng(filename); });
    }
}