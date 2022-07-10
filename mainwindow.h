#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class MdiChild;
class UTable;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QPlainTextEdit;
class QMdiSubWindow;
class QTableWidget;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    bool openFile(const QString &fileName);

    QPlainTextEdit *getConsole() const;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void newGraph();
    void open();
    void updateRecentFileActions();
    void openRecentFile();
    void about();
    void updateMenus();
    void updateWindowMenu();
    MdiChild *createMdiChild();

private:
    enum
    {
        MaxRecentFiles = 5
    };

    void createUI();
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool loadFile(const QString &fileName);
    static bool hasRecentFiles();
    void prependToRecentFiles(const QString &fileName);
    void setRecentFilesVisible(bool visible);
    MdiChild *activeMdiChild() const;
    QMdiSubWindow *findMdiChild(const QString &fileName) const;

    QMdiArea *mdiArea;
    QTabWidget *mTagContainer;
    UTable *mTable;
    QPlainTextEdit *msgConsole;

    QMenu *windowMenu;
    QAction *newGraphAct;
    QAction *recentFileActs[MaxRecentFiles];
    QAction *recentFileSeparator;
    QAction *recentFileSubMenuAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *windowMenuSeparatorAct;
    QMap<QString, QString> varPool;
};

#endif
