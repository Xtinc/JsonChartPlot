#include <QtWidgets>
#include "mainwindow.h"
#include "highlighter.h"
#include "chart.h"

MainWindow::MainWindow()
{
    constructUI();
    createActions();
    createStatusBar();
    readSettings();
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::constructUI()
{
    mainWidget = new QWidget;
    placeholder_widget = new UChart(this);
    msgConsole = new QPlainTextEdit;
    msgConsole->setReadOnly(true);
    Highlighter *highlighter = new Highlighter(msgConsole->document());
    QVBoxLayout *hlayout = new QVBoxLayout;
    hlayout->addWidget(placeholder_widget, 3);
    hlayout->addWidget(msgConsole, 1);
    mainWidget->setLayout(hlayout);
    setCentralWidget(mainWidget);

    placeholder_widget->addGraph();
    placeholder_widget->addGraph();
    placeholder_widget->addGraph();
    placeholder_widget->addGraph();
    placeholder_widget->addGraph();
    placeholder_widget->addGraph();
    placeholder_widget->startRefresh();

    connect(&mDataTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    mDataTimer.start(40);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        loadFile(fileName);
    }
}

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    return saveFile(dialog.selectedFiles().first());
}

void MainWindow::createActions()
{

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    // const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);

    // const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the plot data to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    }
    else
    {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    msgConsole->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        out << msgConsole->toPlainText();
        if (!file.commit())
        {
            errorMessage = tr("Cannot write file %1:\n%2.")
                               .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    }
    else
    {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::timerSlot()
{
    ++cnt;
    for (int i = 0; i < placeholder_widget->graphCount(); ++i)
    {
        QCPGraph *grh = placeholder_widget->graph(i);
        /*QCPGraph *grh = placeholder_widget->graph(i);
        AxisTag *tag = placeholder_widget->axisTag(i);
        grh->addData(grh->dataCount(), qSin(grh->dataCount() / 50.0) + qSin(grh->dataCount() / 50.0 / 0.3843) * 0.25 * i);
        grh->rescaleValueAxis(false, true);
        double graph1Value = grh->dataMainValue(grh->dataCount() - 1);
        tag->updatePosition(graph1Value);
        tag->setText(QString::number(graph1Value, 'f', 2));
        if (i == 0)
        {
            placeholder_widget->xAxis->rescale();
            placeholder_widget->yAxis2->rescale();
        }
        else
        {
            placeholder_widget->yAxis2->rescale(true);
            placeholder_widget->xAxis->rescale(true);
        }*/
        placeholder_widget->addData(cnt, qSin(cnt / 50.0) + qSin(cnt / 50.0 / 0.3843) * 0.25 * i, i);
    }
    // placeholder_widget->xAxis->rescale();
    // placeholder_widget->xAxis->setRange(placeholder_widget->xAxis->range().upper, 100, Qt::AlignRight);

    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    // double graph1Value = mGraph1->dataMainValue(mGraph1->dataCount() - 1);
    // mTag1->updatePosition(graph1Value);
    // mTag1->setText(QString::number(graph1Value, 'f', 2));

    placeholder_widget->replot();
}