#include <QtWidgets>
#include "chart.h"
#include "mdichild.h"

MdiChild::MdiChild()
{
    setAttribute(Qt::WA_DeleteOnClose);
    chart = new UChart(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(chart);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    curFile = "Untitled.png";
    isUntitled = true;
    isModified = false;
}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("graph%1.png").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");
    documentWasModified();
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
    if (!plotJsonObj(loadDoc.object()))
    {
        return false;
    };
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

void MdiChild::documentWasModified()
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
    if (obj.empty())
    {
        return false;
    }
    if (obj.contains("Plot") && obj["Plot"].isObject())
    {
        const auto &map = obj["Plot"].toVariant().toMap();
        if (map.contains("Xvalue"))
        {
            double x = map["Xvalue"].toDouble();
            for (auto iter = map.cbegin(); iter != map.cend(); ++iter)
            {
                if (iter.key() != "Xvalue" && iter.value().canConvert(QMetaType::Double))
                {
                    chart->addData(x, iter.value().toDouble(), iter.key());
                    qInfo() << x << ":" << iter.value().toDouble() << ":" << iter.key();
                }
            }
        }
        else
        {
            qWarning() << "No Xaxis specified!";
        }
    }
    else
    {
        qWarning() << R"(No section named "Plot" in Json object)";
    }
    return true;
}
