#include "mytcpserver.h"
#include "functions_to_server.h"
#include <QDebug>

MyTcpServer::MyTcpServer(quint16 port, QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_port(port)
{
    connect(m_server, &QTcpServer::newConnection, this, &MyTcpServer::onNewConnection);
    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Failed to start server on port" << port;
    } else {
        qDebug() << "Server listening on port" << port;
    }
}

MyTcpServer::~MyTcpServer()
{
    m_server->close();
    qDeleteAll(m_clients);
}

void MyTcpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        m_clients.append(client);
        m_buffers[client].clear();

        connect(client, &QTcpSocket::readyRead, this, &MyTcpServer::onReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &MyTcpServer::onClientDisconnected);

        qDebug() << "Client connected:" << client->peerAddress().toString();
        sendResponse(client, "CONNECTED: Welcome to the server!\r\n");
    }
}

void MyTcpServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_buffers[client] += client->readAll();
    int idx;
    while ((idx = m_buffers[client].indexOf('\n')) != -1) {
        QByteArray line = m_buffers[client].left(idx).trimmed();
        m_buffers[client].remove(0, idx + 1);
        QString command = QString::fromUtf8(line);
        qDebug() << "Command from" << client->peerAddress() << ":" << command;

        QString result = processCommand(client, command);
        sendResponse(client, result.toUtf8() + "\n");
    }
}

void MyTcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        qDebug() << "Client disconnected:" << client->peerAddress().toString();
        m_clients.removeAll(client);
        m_buffers.remove(client);
        client->deleteLater();
    }
}

QString MyTcpServer::processCommand(QTcpSocket *client, const QString &command)
{
    // Пока заглушка
    Q_UNUSED(client);
    return QString("ECHO: %1").arg(command);
}

void MyTcpServer::sendResponse(QTcpSocket *client, const QByteArray &response)
{
    if (client && client->state() == QTcpSocket::ConnectedState) {
        client->write(response);
        client->waitForBytesWritten(1000);
    }
}
