#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QHash>
#include <QSet>

class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpServer(quint16 port = 33333, QObject *parent = nullptr);
    ~MyTcpServer();

    // Включение/выключение режима эхо
    void setEchoMode(bool enabled);
    bool isEchoMode() const { return m_echoMode; }

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QString processCommand(QTcpSocket *client, const QString &command);
    void sendResponse(QTcpSocket *client, const QByteArray &response);

    QByteArray stripTelnetCommands(const QByteArray &data);

    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    QSet<QTcpSocket*> m_adminClients;
    quint16 m_port;
    bool m_echoMode;
};

#endif // MYTCPSERVER_H
