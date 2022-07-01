#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QPlainTextEdit>
#include <QDateTime>

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
    QApplication a(argc, argv);
    MainWindow w;
    msgConsole = w.getConsole();
    qInstallMessageHandler(outputMessage);
    QFont font;
	font.setFamily("consolas");
    w.setFont(font);
    w.show();
    return a.exec();
}
