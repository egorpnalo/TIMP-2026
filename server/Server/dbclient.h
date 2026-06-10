#ifndef DBCLIENT_H
#define DBCLIENT_H

#include <QString>
#include <QTcpSocket>
#include <QMutex>

class DbClient
{
public:
    static DbClient& instance();

    QString sendQuery(const QString& sql);

private:
    DbClient();
    ~DbClient();
    Q_DISABLE_COPY(DbClient)

    bool ensureConnection();

    QTcpSocket *m_socket;
    QMutex m_mutex;
};

#endif // DBCLIENT_H
