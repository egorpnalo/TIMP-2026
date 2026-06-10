#include "mytcpserver.h"
#include "functions_to_server.h"
#include <QDebug>

QByteArray MyTcpServer::stripTelnetCommands(const QByteArray &data)
{
    QByteArray result;
    int i = 0;
    while (i < data.size()) {
        unsigned char ch = static_cast<unsigned char>(data[i]);

        if (ch == 0xFF) {
                       i += 2;
            continue;
        }
        result.append(ch);
        i++;
    }
    return result;
}

// ========== КОНСТРУКТОР / ДЕСТРУКТОР ==========
MyTcpServer::MyTcpServer(quint16 port, QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_port(port), m_echoMode(false)
{
    connect(m_server, &QTcpServer::newConnection, this, &MyTcpServer::onNewConnection);
    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Failed to start server on port" << port;
    } else {
        qDebug() << "Server listening on port" << port;
        qDebug() << "Echo mode is OFF. Use setEchoMode(true) to enable.";
    }
}

MyTcpServer::~MyTcpServer()
{
    m_server->close();
    qDeleteAll(m_clients);
}

// ========== ВКЛЮЧЕНИЕ/ВЫКЛЮЧЕНИЕ РЕЖИМА ЭХО ==========
void MyTcpServer::setEchoMode(bool enabled)
{
    m_echoMode = enabled;
    qDebug() << "=== Echo mode:" << (enabled ? "ON" : "OFF") << "===";
}

// ========== ОБРАБОТКА НОВЫХ ПОДКЛЮЧЕНИЙ ==========
void MyTcpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        m_clients.append(client);
        m_buffers[client].clear();

        connect(client, &QTcpSocket::readyRead, this, &MyTcpServer::onReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &MyTcpServer::onClientDisconnected);

        qDebug() << "Client connected:" << client->peerAddress().toString();

        if (m_echoMode) {
            sendResponse(client, "Connected to ECHO SERVER. Send anything, I'll echo it back!\r\n");
        } else {
            sendResponse(client, "CONNECTED: Welcome to the server!\r\n");
        }
    }
}

// ========== ОБРАБОТКА ВХОДЯЩИХ ДАННЫХ ==========
void MyTcpServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    // Читаем сырые данные
    QByteArray rawData = client->readAll();

    // Удаляем Telnet IAC-последовательности
    QByteArray cleanedData = stripTelnetCommands(rawData);

    // Нормализуем концы строк: \r\n → \n, \r → \n
    cleanedData.replace("\r\n", "\n");
    cleanedData.replace("\r", "\n");

    m_buffers[client] += cleanedData;

    int idx;
    while ((idx = m_buffers[client].indexOf('\n')) != -1) {
        QByteArray line = m_buffers[client].left(idx).trimmed();
        m_buffers[client].remove(0, idx + 1);

        QString command = QString::fromUtf8(line);
        qDebug() << "Command from" << client->peerAddress() << ":" << command;

        QString result;

        // ========== РЕЖИМ ЭХО ==========
        if (m_echoMode) {
            // Эхо-режим: просто возвращаем команду обратно
            result = "ECHO: " + command;
            qDebug() << "Echo mode - returning:" << result;
        }
        // ========== ОБЫЧНЫЙ РЕЖИМ ==========
        else {
            result = processCommand(client, command);
        }

        sendResponse(client, result.toUtf8() + "\n");
    }
}

// ========== ОТКЛЮЧЕНИЕ КЛИЕНТА ==========
void MyTcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        qDebug() << "Client disconnected:" << client->peerAddress().toString();
        m_clients.removeAll(client);
        m_buffers.remove(client);
        m_adminClients.remove(client);
        client->deleteLater();
    }
}

// ========== ОБРАБОТКА КОМАНД (ОБЫЧНЫЙ РЕЖИМ) ==========
QString MyTcpServer::processCommand(QTcpSocket *client, const QString &command)
{
    int separatorPos = command.indexOf('&');
    if (separatorPos == -1) {
        return "ERROR: Invalid command format (expected 'command&params')";
    }

    QString action = command.left(separatorPos).toLower();
    QString payload = command.mid(separatorPos + 1);
    QString result;

    // Регистрация и авторизация
    if (action == "reg") {
        result = handleRegister(payload);
    }
    else if (action == "auth") {
        result = handleAuth(payload);
    }
    else if (action == "stats") {
        result = handleStats(payload);
    }
    // Функции задач
    else if (action == "md5") {
        result = handleMD5(payload);
    }
    else if (action == "secant") {
        result = handleSecant(payload);
    }
    else if (action == "cycle") {
        result = handleCycle(payload);
    }
    // Управление графом
    else if (action == "add_edge") {
        result = handleAddEdge(payload);
    }
    else if (action == "remove_edge") {
        result = handleRemoveEdge(payload);
    }
    else if (action == "get_graph") {
        result = handleGetGraph(payload);
    }
    else if (action == "genetic_path") {
        result = handleGeneticPath(payload);
    }
    else if (action == "shortest") {
        result = handleShortest(payload);
    }
    // Административные функции
    else if (action == "admin_auth") {
        result = handleAdminAuth(payload);
        if (result.startsWith("ADMIN_AUTH_OK")) {
            m_adminClients.insert(client);
            qDebug() << "Admin authenticated:" << client->peerAddress().toString();
        }
    }
    else if (action == "admin_users") {
        if (m_adminClients.contains(client)) {
            result = handleAdminGetUsers(payload);
        } else {
            result = "ADMIN_ERR: Not authenticated as admin";
        }
    }
    else if (action == "admin_setstatus") {
        if (m_adminClients.contains(client)) {
            result = handleAdminSetStatus(payload);
        } else {
            result = "ADMIN_ERR: Not authenticated as admin";
        }
    }
    // RSA функции
    else if (action == "rsa_pubkey") {
        result = handleRsaPubkey(payload);
    }
    else if (action == "rsa_encrypt") {
        result = handleRsaEncrypt(payload);
    }
    // Неизвестная команда
    else {
        result = "ERROR: Unknown command '" + action + "'";
    }

    qDebug() << "Command:" << action << "Result:" << result.left(50);
    return result;
}

// ========== ОТПРАВКА ОТВЕТА ==========
void MyTcpServer::sendResponse(QTcpSocket *client, const QByteArray &response)
{
    if (client && client->state() == QTcpSocket::ConnectedState) {
        // Для совместимости с Telnet отправляем \r\n
        QByteArray telnetResponse = response;
        telnetResponse.replace("\n", "\r\n");
        client->write(telnetResponse);
        client->waitForBytesWritten(1000);
    }
}
