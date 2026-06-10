#include <QCoreApplication>
#include "databasemanager.h"
#include "dbtcpserver.h"
#include <QDebug>
#include <QSqlDatabase>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "=== DbServer starting ===";

    // Проверяем доступность драйвера SQLite
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qCritical() << "ERROR: SQLite driver is not available!";
        qCritical() << "Available drivers:" << QSqlDatabase::drivers();
        return -1;
    }
    qDebug() << "SQLite driver is available";

    // Инициализируем базу данных
    qDebug() << "Initializing database...";
    if (!DatabaseManager::instance().init("Test.db")) {
        qCritical() << "Failed to initialize database";
        return -1;
    }
    qDebug() << "Database initialized successfully";

    // ========== СОЗДАНИЕ АДМИНИСТРАТОРА ==========
    qDebug() << "Checking for admin user...";

    // Проверяем, существует ли администратор
    QString checkAdmin = "SELECT COUNT(*) FROM User WHERE login='admin';";
    QString resp = DatabaseManager::instance().execute(checkAdmin);

    qDebug() << "Admin check response:" << resp;

    // Если администратора нет, создаём
    if (resp.contains("0") || resp.contains("COUNT(*)\r\n0")) {
        QString createAdmin = "INSERT INTO User(login, password, status) VALUES('admin', 'admin123', 'admin');";
        QString result = DatabaseManager::instance().execute(createAdmin);
        qDebug() << "Creating admin user...";
        qDebug() << "Result:" << result;

        if (result.startsWith("OK")) {
            qDebug() << "✅ Admin user created successfully!";
            qDebug() << "   Login: admin";
            qDebug() << "   Password: admin123";
        } else {
            qDebug() << "❌ Failed to create admin user:" << result;
        }
    } else {
        qDebug() << "✅ Admin user already exists";
        qDebug() << "   Login: admin";
        qDebug() << "   Password: admin123";
    }

    // Проверяем, что администратор действительно создался
    QString verifyAdmin = "SELECT login, status FROM User WHERE login='admin';";
    QString verifyResp = DatabaseManager::instance().execute(verifyAdmin);
    qDebug() << "Admin verification:" << verifyResp;

    // Запускаем сервер БД
    qDebug() << "Starting DbTcpServer on port 33334...";
    DbTcpServer server(33334);

    qDebug() << "=== DbServer is running, waiting for connections ===";

    return a.exec();
}
