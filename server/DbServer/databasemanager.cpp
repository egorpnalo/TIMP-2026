#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QFile>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::init(const QString& dbPath)
{
    qDebug() << "DatabaseManager::init() called with path:" << dbPath;

    // Проверяем, существует ли соединение
    if (QSqlDatabase::contains("db_connection")) {
        m_db = QSqlDatabase::database("db_connection");
        qDebug() << "Using existing database connection";
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "db_connection");
        m_db.setDatabaseName(dbPath);
        qDebug() << "Created new database connection to:" << dbPath;
    }

    // Пытаемся открыть базу данных
    if (!m_db.open()) {
        qCritical() << "Cannot open database:" << m_db.lastError().text();
        qCritical() << "Database path:" << m_db.databaseName();
        return false;
    }
    qDebug() << "Database opened successfully";

    // Создаём таблицу User
    QSqlQuery query(m_db);
    QString createTable = "CREATE TABLE IF NOT EXISTS User("
                          "login VARCHAR(20) NOT NULL PRIMARY KEY, "
                          "password VARCHAR(20) NOT NULL, "
                          "status VARCHAR(20) NOT NULL DEFAULT 'offline')";

    qDebug() << "Executing SQL:" << createTable;
    if (!query.exec(createTable)) {
        qCritical() << "Failed to create table:" << query.lastError().text();
        return false;
    }
    qDebug() << "Table User created/verified successfully";

    // Проверяем, что таблица действительно создалась
    if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='User'")) {
        if (query.next()) {
            qDebug() << "Table 'User' exists in database";
        } else {
            qCritical() << "Table 'User' was not created!";
            return false;
        }
    }

    qDebug() << "Database initialization completed successfully";
    return true;
}

bool DatabaseManager::ensureOpen() const
{
    if (!m_db.isValid() || !m_db.isOpen()) {
        qCritical() << "Database is not open!";
        return false;
    }
    return true;
}

QString DatabaseManager::execute(const QString& sql)
{
    if (!ensureOpen())
        return "ERROR: DB not open";

    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        return QString("ERROR: %1").arg(query.lastError().text());
    }

    QString cmd = sql.trimmed().toUpper();
    if (cmd.startsWith("SELECT")) {
        QSqlRecord rec = query.record();
        int cols = rec.count();
        QStringList lines;

        QStringList headers;
        for (int i = 0; i < cols; ++i)
            headers << rec.fieldName(i);
        lines << headers.join("\t");

        while (query.next()) {
            QStringList row;
            for (int i = 0; i < cols; ++i)
                row << query.value(i).toString();
            lines << row.join("\t");
        }
        return lines.join("\r\n");
    } else {
        int changes = query.numRowsAffected();
        return QString("OK: Rows affected = %1").arg(changes);
    }
}
