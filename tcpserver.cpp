#include "tcpserver.h"
#include <QHostAddress>

UTcpServer::UTcpServer(QObject *parent) : QObject(parent)
{
}

UTcpServer::~UTcpServer()
{
    close();
}

void UTcpServer::start(const QHostAddress &hostaddress, quint16 port)
{
    server = new QTcpServer(this);
    server->listen(hostaddress, port);
    connect(server, &QTcpServer::newConnection, this, [this]()
            {
                while (server->hasPendingConnections())
                {
                    QTcpSocket *socket = server->nextPendingConnection();
                    clients.append(socket);
                    qInfo() << QString("[%1%2] Socket Connected!").arg(socket->peerAddress().toString().arg(socket->peerPort()));
                    connect(socket, &QAbstractSocket::disconnected,socket, &QObject::deleteLater);
                    connect(socket,&QAbstractSocket::disconnected,this,[this,socket](){
                        clients.removeOne(socket);
                    });
                    connect(socket,&QTcpSocket::readyRead,this,[this,socket](){
                    if(socket->bytesAvailable()<=0){
                        return;
                    }
                    const QString recvData=QString::fromUtf8(socket->readAll());
                    qInfo()<<recvData;
                });
                } });
}

void UTcpServer::close()
{
}
