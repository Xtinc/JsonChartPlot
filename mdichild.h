#ifndef MDICHILD_H
#define MDICHILD_H

#include <QJsonObject>
#include <QMap>

class UChart;

class MdiChild : public QWidget
{
    Q_OBJECT

public:
    MdiChild();

    void newFile(const QMap<QString, QString> &varmap);
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }
    bool plotJsonObj(const QJsonObject &obj);
    bool plotJsonObj2();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void graphWasModified();

private:
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    UChart *chart;
    QString curFile;
    bool isUntitled;
    bool isModified;
    QMap<QString, QString> mp;
};

#endif
