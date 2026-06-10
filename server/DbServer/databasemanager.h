#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool init(const QString& dbPath = "Test.db");
    QString execute(const QString& sql);

private:
    DatabaseManager();
    ~DatabaseManager();
    Q_DISABLE_COPY(DatabaseManager)
    bool ensureOpen() const;
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
