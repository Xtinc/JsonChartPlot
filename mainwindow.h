#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QAction;
class QWidget;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

class UChart;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    void loadFile(const QString &fileName);
    QPlainTextEdit *getConsole() const
    {
        return msgConsole;
    }

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:

    void timerSlot();
    void open();
    bool saveAs();

private:
    void constructUI();
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool saveFile(const QString &fileName);
    QString strippedName(const QString &fullFileName)
    {
        return QFileInfo(fullFileName).fileName();
    };

private:
    int cnt = 0;
    QTimer mDataTimer;
    UChart *placeholder_widget;
    QWidget *mainWidget;
    QPlainTextEdit *msgConsole;
};

#endif
