#include <QApplication>
#include "client.h"
#include "regandauth.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client* client = Client::getInstance();
    client->waitForConnection(3000);
    RegAndAuth w;
    w.show();
    return a.exec();
}
