#include <QCoreApplication>
#include "mytcpserver.h"
#include <QDebug>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Парсинг аргументов командной строки
    QCommandLineParser parser;
    parser.setApplicationDescription("TCP Server with Echo mode");
    parser.addHelpOption();

    QCommandLineOption echoOption(QStringList() << "e" << "echo",
                                  "Run in echo mode (just echoes back received messages)");
    parser.addOption(echoOption);

    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Server port (default: 33333)",
                                  "port");
    parser.addOption(portOption);

    parser.process(a);

    quint16 port = 33333;
    if (parser.isSet(portOption)) {
        port = parser.value(portOption).toUShort();
    }

    MyTcpServer server(port);

    // Включаем эхо-режим если указан флаг
    if (parser.isSet(echoOption)) {
        server.setEchoMode(true);
        qDebug() << "Server started in ECHO mode on port" << port;
    } else {
        qDebug() << "Server started in NORMAL mode on port" << port;
        qDebug() << "To enable echo mode, run with --echo or -e flag";
    }

    return a.exec();
}
