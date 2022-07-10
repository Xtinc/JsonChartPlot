#ifndef MDICHILD_H
#define MDICHILD_H

#include <QJsonObject>

class UChart;

class MdiChild : public QWidget
{
    Q_OBJECT

public:
    MdiChild();

    int timeCnt;

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
    bool plotJsonObj(const QJsonObject &obj);
    QString strippedName(const QString &fullFileName);

    UChart *chart;
    QString curFile;
    bool isUntitled;
    bool isModified;
    QTimer *timer;
};

#endif
