#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>

class UTcpServer : public QObject
{
    Q_OBJECT
public:
    UTcpServer(QObject *parent = nullptr);
    ~UTcpServer();
    void start(const QHostAddress &hostaddress, quint16 port);
    void close();

private:
    QTcpServer *server;
    QList<QTcpSocket *> clients;

private:
    void update();
};

#endif