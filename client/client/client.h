#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class Client : public QObject
{
    Q_OBJECT

    static Client* p_instance;
    class ClientDestroyer {
        Client *p;
    public:
        ClientDestroyer() : p(nullptr) {}
        ~ClientDestroyer() { delete p; }
        void init(Client *ptr) { p = ptr; }
    };
    static ClientDestroyer destroyer;
    explicit Client(QObject *parent = nullptr);
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    friend class ClientDestroyer;

public:
    static Client* getInstance();
    ~Client() override;

    void sendMsgToServer(const QString &msg);
    bool isConnected() const;
    bool waitForConnection(int msecs = 3000);

signals:
    void msgFromServer(const QString &msg);
    void connectionStateChanged(bool connected);

private slots:
    void slotServerRead();
    void slotConnected();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *mTcpSocket;
    QTimer *m_reconnectTimer;
    int m_reconnectAttempts = 0;
    bool m_isConnected = false;
};

#endif // CLIENT_H
