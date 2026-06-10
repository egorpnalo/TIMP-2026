#ifndef FUNCTIONS_TO_SERVER_H
#define FUNCTIONS_TO_SERVER_H

#include <QString>

// ==================== ОСНОВНЫЕ ФУНКЦИИ ====================

/**
 * @brief Обрабатывает регистрацию нового пользователя
 * @param payload Строка формата "login,password"
 * @return "REGISTER_OK: User created successfully" или "REGISTER_ERR: ..."
 */
QString handleRegister(const QString &payload);

/**
 * @brief Обрабатывает авторизацию пользователя
 * @param payload Строка формата "login,password"
 * @return "AUTH_OK: Welcome {login}" или "AUTH_ERR: ..."
 */
QString handleAuth(const QString &payload);

/**
 * @brief Возвращает количество зарегистрированных пользователей
 * @param payload (не используется)
 * @return "STATS: Total users = N"
 */
QString handleStats(const QString &payload);

// ==================== ФУНКЦИИ ЗАДАЧ ====================

/**
 * @brief Вычисляет MD5 хэш входной строки (собственная реализация)
 * @param payload Строка для хэширования
 * @return "MD5: {32-символьный хэш}" или "MD5_ERR: ..."
 */
QString handleMD5(const QString &payload);

/**
 * @brief Вычисляет квадратный корень методом секущих
 * @param payload Число в виде строки
 * @return "SECANT_RESULT: {значение}" или "SECANT_ERR: ..."
 */
QString handleSecant(const QString &payload);

/**
 * @brief Проверяет, является ли последовательность вершин циклом в графе
 * @param payload Последовательность вершин через запятую (например, "1,2,3,1")
 * @return "CYCLE_OK: Valid cycle with N edges" или "CYCLE_ERR: ..."
 */
QString handleCycle(const QString &payload);

// ==================== ФУНКЦИИ ДЛЯ РАБОТЫ С ГРАФОМ ====================

/**
 * @brief Добавляет новое ребро в граф
 * @param payload Формат: "u,v" или "u,v,weight" (например, "5,6" или "5,6,3")
 * @return "GRAPH_OK: Edge added with weight N" или "GRAPH_ERR: ..."
 */
QString handleAddEdge(const QString &payload);

/**
 * @brief Удаляет ребро из графа
 * @param payload Формат: "u,v" (например, "5,6")
 * @return "GRAPH_OK: Edge removed successfully" или "GRAPH_ERR: ..."
 */
QString handleRemoveEdge(const QString &payload);

/**
 * @brief Возвращает список всех рёбер графа
 * @param payload (не используется)
 * @return "GRAPH_EDGES: {список рёбер с весами}"
 */
QString handleGetGraph(const QString &payload);

/**
 * @brief Изменяет вес существующего ребра
 * @param payload Формат: "u,v,weight" (например, "1,2,5")
 * @return "GRAPH_OK: Edge weight changed" или "GRAPH_ERR: ..."
 */
QString handleSetEdgeWeight(const QString &payload);

/**
 * @brief Находит кратчайший путь между вершинами (алгоритм Дейкстры)
 * @param payload Формат: "s,t" (например, "1,4")
 * @return "GRAPH: Shortest path from s to t (total weight=W): path"
 */
QString handleShortest(const QString &payload);

/**
 * @brief Находит кратчайший путь с помощью генетического алгоритма
 * @param payload Формат: "s,t" (например, "1,4")
 * @return "GENETIC_RESULT: Path from s to t (weight=W): path"
 */
QString handleGeneticPath(const QString &payload);

// ==================== АДМИНИСТРАТИВНЫЕ ФУНКЦИИ ====================

/**
 * @brief Аутентификация администратора
 * @param payload Формат: "login,password"
 * @return "ADMIN_AUTH_OK: Access granted" или "ADMIN_AUTH_ERR: ..."
 */
QString handleAdminAuth(const QString &payload);

/**
 * @brief Возвращает список всех пользователей (только для администратора)
 * @param payload (не используется)
 * @return "ADMIN_USERS: {список login | status}"
 */
QString handleAdminGetUsers(const QString &payload);

/**
 * @brief Изменяет статус пользователя (только для администратора)
 * @param payload Формат: "login,status" (status: online, offline, banned, admin)
 * @return "ADMIN_OK: Status updated for user {login}" или "ADMIN_ERR: ..."
 */
QString handleAdminSetStatus(const QString &payload);

// ==================== RSA ФУНКЦИИ ====================

/**
 * @brief Возвращает публичный ключ RSA
 * @param payload (не используется)
 * @return "RSA_PUBKEY:n,e"
 */
QString handleRsaPubkey(const QString &payload);

/**
 * @brief Расшифровывает сообщение, зашифрованное публичным ключом
 * @param payload Зашифрованное сообщение в base64
 * @return "RSA_DECRYPT:{расшифрованное сообщение}" или "RSA_ERR: ..."
 */
QString handleRsaEncrypt(const QString &payload);

#endif // FUNCTIONS_TO_SERVER_H
