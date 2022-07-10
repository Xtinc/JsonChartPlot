#ifndef MDICHILD_H
#define MDICHILD_H

#include <QJsonObject>

class UChart;

class MdiChild : public QWidget
{
    Q_OBJECT

public:
    MdiChild(QMap<QString, QString> &map);

    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void graphWasModified();

private:
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    void refreshVar();
    bool plotJsonObj(const QJsonObject &obj);
    QString strippedName(const QString &fullFileName);

    int timeCnt;

    UChart *chart;
    QString curFile;
    bool isUntitled;
    bool isModified;
    QTimer *mTimer;
    QMap<QString, QString> &mMap;
    QMap<QString, double> mPriMap;
};

#endif
