#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
class QAction;
class QWidget;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

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
    QWidget *mainWidget;
    QPlainTextEdit *msgConsole;
};

#endif
