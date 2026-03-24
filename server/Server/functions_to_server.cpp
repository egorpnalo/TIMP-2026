#include "functions_to_server.h"
#include <QDebug>

QString handleRegister(const QString &payload)
{
    Q_UNUSED(payload);
    return "REGISTER_ERR: Not implemented";
}

QString handleAuth(const QString &payload)
{
    Q_UNUSED(payload);
    return "AUTH_ERR: Not implemented";
}

QString handleStats(const QString &payload)
{
    Q_UNUSED(payload);
    return "STATS_ERR: Not implemented";
}

QString handleMD5(const QString &payload)
{
    Q_UNUSED(payload);
    return "MD5_ERR: Not implemented";
}

QString handleSecant(const QString &payload)
{
    Q_UNUSED(payload);
    return "SECANT_ERR: Not implemented";
}

QString handleCycle(const QString &payload)
{
    Q_UNUSED(payload);
    return "CYCLE_ERR: Not implemented";
}

QString handleAdminAuth(const QString &payload)
{
    Q_UNUSED(payload);
    return "ADMIN_AUTH_ERR: Not implemented";
}

QString handleAdminGetUsers(const QString &payload)
{
    Q_UNUSED(payload);
    return "ADMIN_ERR: Not implemented";
}

QString handleAdminSetStatus(const QString &payload)
{
    Q_UNUSED(payload);
    return "ADMIN_ERR: Not implemented";
}
