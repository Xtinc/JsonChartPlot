#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QPlainTextEdit>
#include <QDateTime>
#include <QCommandLineParser>
#include <QCommandLineOption>

QPlainTextEdit *msgConsole = nullptr;

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (msgConsole != nullptr)
    {
        QString text;
        bool ierror = false;
        switch (type)
        {
        case QtDebugMsg:
            text = " MSGS:";
            break;
        case QtWarningMsg:
            text = " WARN:";
            ierror = true;
            break;
        case QtCriticalMsg:
            text = " CRIT:";
            ierror = true;
            break;
        case QtFatalMsg:
            text = " CRIT:";
            ierror = true;
            break;
        case QtInfoMsg:
            text = " INFO:";
            break;
        default:
            break;
        }
        QString date_time = QDateTime::currentDateTime().toString("[MM-dd hh:mm:ss]");
        if (ierror)
        {
            msgConsole->appendPlainText(QStringLiteral("%1%2 File:%3 Line: %4 %5").arg(date_time).arg(text).arg(context.file).arg(context.line).arg(msg));
        }
        else
        {
            msgConsole->appendPlainText(QStringLiteral("%1%2 %3").arg(date_time).arg(text).arg(msg));
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("MDI Example");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt MDI Example");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    MainWindow mainWin;
    const QStringList posArgs = parser.positionalArguments();
    for (const QString &fileName : posArgs)
    {
        mainWin.openFile(fileName);
    }
    
    msgConsole = mainWin.getConsole();
    qInstallMessageHandler(outputMessage);
    QFont font;
    font.setFamily("consolas");
    mainWin.setFont(font);
    mainWin.show();
    return app.exec();
}
