#include "dbclient.h"
#include <QDebug>

DbClient::DbClient() : m_socket(nullptr)
{
    qDebug() << "DbClient constructor called";
}

DbClient::~DbClient()
{
    qDebug() << "DbClient destructor called";
    if (m_socket) {
        m_socket->disconnectFromHost();
        delete m_socket;
    }
}

DbClient& DbClient::instance()
{
    static DbClient inst;
    qDebug() << "DbClient::instance() called";
    return inst;
}

bool DbClient::ensureConnection()
{
    qDebug() << "DbClient::ensureConnection()";

    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        qDebug() << "DbClient: already connected";
        return true;
    }

    if (m_socket) {
        delete m_socket;
        m_socket = nullptr;
    }

    m_socket = new QTcpSocket();
    qDebug() << "DbClient: connecting to 127.0.0.1:33334...";
    m_socket->connectToHost("127.0.0.1", 33334);

    if (!m_socket->waitForConnected(3000)) {
        qWarning() << "DbClient: Failed to connect to DbServer:" << m_socket->errorString();
        return false;
    }

    qDebug() << "DbClient: connected successfully";
    return true;
}

QString DbClient::sendQuery(const QString& sql)
{
    qDebug() << "DbClient::sendQuery() called with:" << sql;

    if (!ensureConnection()) {
        qDebug() << "DbClient: connection failed";
        return "ERROR: Cannot connect to DB server";
    }

    // Отправляем запрос
    QByteArray request = sql.toUtf8() + '\n';
    qint64 bytesWritten = m_socket->write(request);
    qDebug() << "DbClient: wrote" << bytesWritten << "bytes";

    if (bytesWritten == -1) {
        qDebug() << "DbClient: write failed";
        return "ERROR: Write failed";
    }

    if (!m_socket->waitForBytesWritten(2000)) {
        qDebug() << "DbClient: write timeout";
        return "ERROR: Write timeout";
    }

    // Ждём ответ
    if (!m_socket->waitForReadyRead(5000)) {
        qDebug() << "DbClient: read timeout";
        return "ERROR: Read timeout";
    }

    // Читаем ответ
    QByteArray response = m_socket->readAll();
    while (m_socket->waitForReadyRead(100)) {
        response += m_socket->readAll();
    }

    qDebug() << "DbClient: received response:" << response;
    return QString::fromUtf8(response).trimmed();
}
