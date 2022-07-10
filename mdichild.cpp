#include <QtWidgets>
#include "chart.h"
#include "mdichild.h"
#include "expression.h"

MdiChild::MdiChild(QMap<QString, QString> &map) : isUntitled(true), isModified(false), mMap(map)
{
    setAttribute(Qt::WA_DeleteOnClose);
    chart = new UChart(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(chart);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    curFile = "Untitled.png";
}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("graph%1.png").arg(sequenceNumber++);
    setWindowTitle(curFile);
    graphWasModified();
    mTimer = new QTimer(this);
    timeCnt = 0;
    connect(mTimer, &QTimer::timeout, this, [&]()
            { plotJsonObj(QJsonObject{{"Xvalue", timeCnt}, {"Sin", (100.0/(timeCnt+1))*sin(0.1*timeCnt)}, {"Cos", (100.0/(timeCnt+1))*cos(0.1*timeCnt)}});
            timeCnt++;
            mTimer->start(4); });
    mTimer->start(1000);
}

bool MdiChild::loadFile(const QString &fileName)
{
    auto fileSuffix = QFileInfo(fileName).suffix();
    if (fileSuffix != "json" && fileSuffix != "cbor")
    {
        qWarning() << "Unknow file type!";
        return false;
    }
    QFile loadFile(fileName);

    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(fileSuffix == "json"
                              ? QJsonDocument::fromJson(saveData)
                              : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QJsonArray jsonArray = loadDoc.array();

    for (const QJsonValue &value : jsonArray)
    {
        QJsonObject obj = value.toObject();
        if (obj.contains("Xvalue"))
        {
            plotJsonObj(obj);
        }
    }
    QGuiApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}

bool MdiChild::save()
{
    if (isUntitled)
    {
        return saveAs();
    }
    else
    {
        return saveFile(curFile);
    }
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    chart->savePng(fileName);
    QGuiApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MdiChild::graphWasModified()
{
    isModified = true;
    setWindowModified(isModified);
}

bool MdiChild::maybeSave()
{
    if (!isModified)
        return true;
    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("MDI"),
                                                                 tr("'%1' has been modified.\n"
                                                                    "Do you want to save your changes?")
                                                                     .arg(userFriendlyCurrentFile()),
                                                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret)
    {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MdiChild::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    isModified = false;
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile());
    chart->setTitle(userFriendlyCurrentFile());
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

bool MdiChild::plotJsonObj(const QJsonObject &obj)
{
    const auto &map = obj.toVariantMap();
    double x = map["Xvalue"].toDouble();
    for (auto iter = map.cbegin(); iter != map.cend(); ++iter)
    {
        if (iter.key() != "Xvalue" && iter.value().canConvert(QMetaType::Double))
        {
            chart->addData(x, iter.value().toDouble(), iter.key());
        }
    }
    return true;
}

void MdiChild::refreshVar()
{
}
