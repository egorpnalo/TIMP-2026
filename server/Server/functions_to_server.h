#ifndef FUNCTIONS_TO_SERVER_H
#define FUNCTIONS_TO_SERVER_H

#include <QString>

// Заглушки для будущих функций
QString handleRegister(const QString &payload);
QString handleAuth(const QString &payload);
QString handleStats(const QString &payload);
QString handleMD5(const QString &payload);
QString handleSecant(const QString &payload);
QString handleCycle(const QString &payload);
QString handleAdminAuth(const QString &payload);
QString handleAdminGetUsers(const QString &payload);
QString handleAdminSetStatus(const QString &payload);

#endif // FUNCTIONS_TO_SERVER_H
