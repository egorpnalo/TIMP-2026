#include "functions_to_server.h"
#include "dbclient.h"
#include <QDebug>
#include <cmath>
#include <QSet>
#include <QDateTime>
#include <cstring>
#include <random>
#include <chrono>
#include <QElapsedTimer>
#include <QQueue>
#include <climits>

// ==================== СОБСТВЕННАЯ РЕАЛИЗАЦИЯ MD5 ====================

class MyMD5
{
private:
    // Константы MD5 (синусы)
    static const unsigned int K[64];

    // Сдвиги для каждого раунда
    static const unsigned int S[64];

    // Начальные векторы
    static const unsigned int H0[4];

    // Вспомогательные функции
    unsigned int F(unsigned int x, unsigned int y, unsigned int z) {
        return (x & y) | (~x & z);
    }

    unsigned int G(unsigned int x, unsigned int y, unsigned int z) {
        return (x & z) | (y & ~z);
    }

    unsigned int H(unsigned int x, unsigned int y, unsigned int z) {
        return x ^ y ^ z;
    }

    unsigned int I(unsigned int x, unsigned int y, unsigned int z) {
        return y ^ (x | ~z);
    }

    // Циклический сдвиг влево
    unsigned int leftRotate(unsigned int x, int c) {
        return (x << c) | (x >> (32 - c));
    }

public:
    QString calculate(const QString &input) {
        return calculate(input.toUtf8());
    }

    QString calculate(const QByteArray &input) {
        QByteArray hash = compute(input);
        return hash.toHex();
    }

private:
    QByteArray compute(const QByteArray &input)
    {
        QByteArray data = input;

        // 1. Добавление бита "1" (0x80)
        data.append(static_cast<char>(0x80));

        // 2. Добавление нулей
        quint64 originalLength = input.size() * 8;

        while ((data.size() * 8) % 512 != 448) {
            data.append(static_cast<char>(0x00));
        }

        // 3. Добавление длины (64 бита, little-endian)
        for (int i = 0; i < 8; i++) {
            data.append(static_cast<char>((originalLength >> (i * 8)) & 0xFF));
        }

        // 4. Инициализация переменных
        unsigned int a0 = H0[0];
        unsigned int b0 = H0[1];
        unsigned int c0 = H0[2];
        unsigned int d0 = H0[3];

        // 5. Обработка блоков по 64 байта
        for (size_t i = 0; i < (size_t)data.size(); i += 64) {
            unsigned int M[16];

            // Разбиваем блок на 16 слов по 32 бита
            for (int j = 0; j < 16; j++) {
                M[j] = 0;
                for (int k = 0; k < 4; k++) {
                   M[j] |= (static_cast<unsigned char>(data.at(i + j * 4 + k)) << (k * 8));
                }
            }

            unsigned int A = a0;
            unsigned int B = b0;
            unsigned int C = c0;
            unsigned int D = d0;

            // Основной цикл (64 шага)
            for (int j = 0; j < 64; j++) {
                unsigned int F_val, g;

                if (j < 16) {
                    F_val = F(B, C, D);
                    g = j;
                } else if (j < 32) {
                    F_val = G(B, C, D);
                    g = (5 * j + 1) % 16;
                } else if (j < 48) {
                    F_val = H(B, C, D);
                    g = (3 * j + 5) % 16;
                } else {
                    F_val = I(B, C, D);
                    g = (7 * j) % 16;
                }

                unsigned int temp = D;
                D = C;
                C = B;
                B = B + leftRotate(A + F_val + K[j] + M[g], S[j]);
                A = temp;
            }

            a0 += A;
            b0 += B;
            c0 += C;
            d0 += D;
        }

        // 6. Формирование результата
        QByteArray result;

        for (int i = 0; i < 4; i++) result.append(static_cast<char>((a0 >> (i * 8)) & 0xFF));
        for (int i = 0; i < 4; i++) result.append(static_cast<char>((b0 >> (i * 8)) & 0xFF));
        for (int i = 0; i < 4; i++) result.append(static_cast<char>((c0 >> (i * 8)) & 0xFF));
        for (int i = 0; i < 4; i++) result.append(static_cast<char>((d0 >> (i * 8)) & 0xFF));

        return result;
    }
};

// Константы K[i] = floor(2^32 * abs(sin(i+1)))
const unsigned int MyMD5::K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// Сдвиги для каждого шага
const unsigned int MyMD5::S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

// Начальные значения хэша
const unsigned int MyMD5::H0[4] = {
    0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476
};

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================

static QStringList splitLines(const QString& resp)
{
    return resp.split("\r\n", Qt::SkipEmptyParts);
}

// ==================== ВЗВЕШЕННЫЙ ГРАФ ====================

struct Graph {
    // Для каждой вершины храним список (сосед, вес)
    QHash<int, QVector<QPair<int, int>>> adj;

    void addEdge(int u, int v, int weight = 1) {
        // Проверяем, не существует ли уже такое ребро
        for (const auto& edge : adj.value(u)) {
            if (edge.first == v) return;
        }
        adj[u].append(qMakePair(v, weight));
        adj[v].append(qMakePair(u, weight));
    }

    void removeEdge(int u, int v) {
        if (adj.contains(u)) {
            for (int i = 0; i < adj[u].size(); i++) {
                if (adj[u][i].first == v) {
                    adj[u].removeAt(i);
                    break;
                }
            }
            if (adj[u].isEmpty()) adj.remove(u);
        }

        if (adj.contains(v)) {
            for (int i = 0; i < adj[v].size(); i++) {
                if (adj[v][i].first == u) {
                    adj[v].removeAt(i);
                    break;
                }
            }
            if (adj[v].isEmpty()) adj.remove(v);
        }
    }

    bool hasEdge(int u, int v) const {
        for (const auto& edge : adj.value(u)) {
            if (edge.first == v) return true;
        }
        return false;
    }

    int getWeight(int u, int v) const {
        for (const auto& edge : adj.value(u)) {
            if (edge.first == v) return edge.second;
        }
        return -1;
    }

    QVector<int> getNeighbors(int u) const {
        QVector<int> neighbors;
        for (const auto& edge : adj.value(u)) {
            neighbors.append(edge.first);
        }
        return neighbors;
    }

    QString getEdgesString() const {
        QStringList edges;
        QSet<QString> added;

        QHashIterator<int, QVector<QPair<int, int>>> it(adj);
        while (it.hasNext()) {
            it.next();
            int u = it.key();
            for (const auto& edge : it.value()) {
                int v = edge.first;
                int w = edge.second;
                int from = qMin(u, v);
                int to = qMax(u, v);
                QString edgeStr = QString("%1-%2 [%3]").arg(from).arg(to).arg(w);
                if (!added.contains(edgeStr)) {
                    edges << edgeStr;
                    added.insert(edgeStr);
                }
            }
        }

        edges.sort();
        return edges.join(", ");
    }
};

static Graph graph;
static bool graphInited = false;

static void initGraph()
{
    if (graphInited) return;

    // Граф с весами
    graph.addEdge(1, 2, 2);
    graph.addEdge(2, 3, 1);
    graph.addEdge(3, 4, 3);
    graph.addEdge(4, 1, 1);
    graph.addEdge(1, 3, 4);

    graphInited = true;
}

// ==================== МЕТОД СЕКУЩИХ ====================

static double secantSqrt(double a, double eps = 1e-7)
{
    if (a < 0) return -1;
    if (a == 0) return 0;

    double x0 = 0;
    double x1 = (a > 1) ? a : 1;

    for (int i = 0; i < 100; ++i) {
        double f0 = x0 * x0 - a;
        double f1 = x1 * x1 - a;

        if (std::fabs(f1) < eps) break;

        double x2 = x1 - f1 * (x1 - x0) / (f1 - f0);
        x0 = x1;
        x1 = x2;
    }

    return x1;
}

// ==================== РЕГИСТРАЦИЯ ====================

QString handleRegister(const QString &payload)
{
    qDebug() << "=== handleRegister called with payload:" << payload;

    QStringList parts = payload.split(',');
    if (parts.size() != 2) {
        return "REGISTER_ERR: Bad format (expected login,password)";
    }

    QString login = parts[0].trimmed();
    QString password = parts[1].trimmed();

    if (login.isEmpty() || password.isEmpty()) {
        return "REGISTER_ERR: Login and password cannot be empty";
    }

    QString checkSql = QString("SELECT COUNT(*) FROM User WHERE login='%1';").arg(login);
    QString checkResp = DbClient::instance().sendQuery(checkSql);

    if (checkResp.startsWith("ERROR")) {
        return "REGISTER_ERR: DB error: " + checkResp;
    }

    auto lines = splitLines(checkResp);
    int count = (lines.size() > 1) ? lines[1].toInt() : 0;

    if (count > 0) {
        return "REGISTER_ERR: User already exists";
    }

    QString insertSql = QString(
                            "INSERT INTO User(login, password, status) VALUES('%1','%2','offline');"
                            ).arg(login, password);

    QString insertResp = DbClient::instance().sendQuery(insertSql);

    if (insertResp.startsWith("ERROR")) {
        return "REGISTER_ERR: DB error: " + insertResp;
    }

    return "REGISTER_OK: User created successfully";
}

// ==================== АВТОРИЗАЦИЯ ====================

QString handleAuth(const QString &payload)
{
    QStringList parts = payload.split(',');
    if (parts.size() != 2) {
        return "AUTH_ERR: Bad format (expected login,password)";
    }

    QString login = parts[0].trimmed();
    QString password = parts[1].trimmed();

    QString sql = QString(
                      "SELECT login FROM User WHERE login='%1' AND password='%2';"
                      ).arg(login, password);

    QString resp = DbClient::instance().sendQuery(sql);

    if (resp.startsWith("ERROR")) {
        return "AUTH_ERR: DB error: " + resp;
    }

    auto lines = splitLines(resp);
    if (lines.size() < 2) {
        return "AUTH_ERR: Invalid credentials";
    }

    QString updateSql = QString("UPDATE User SET status='online' WHERE login='%1';").arg(login);
    DbClient::instance().sendQuery(updateSql);

    return QString("AUTH_OK: Welcome %1").arg(login);
}

// ==================== СТАТИСТИКА ====================

QString handleStats(const QString &payload)
{
    Q_UNUSED(payload);

    QString sql = "SELECT COUNT(*) FROM User;";
    QString resp = DbClient::instance().sendQuery(sql);

    if (resp.startsWith("ERROR")) {
        return "STATS_ERR: " + resp;
    }

    auto lines = splitLines(resp);
    if (lines.size() < 2) {
        return "STATS_ERR: No data";
    }

    int count = lines[1].toInt();
    return QString("STATS: Total users = %1").arg(count);
}

// ==================== MD5 ====================

QString handleMD5(const QString &payload)
{
    if (payload.isEmpty()) {
        return "MD5_ERR: No input";
    }

    MyMD5 md5;
    QString hash = md5.calculate(payload);
    return QString("MD5: %1").arg(hash);
}

// ==================== МЕТОД СЕКУЩИХ ====================

QString handleSecant(const QString &payload)
{
    bool ok;
    double value = payload.toDouble(&ok);

    if (!ok) {
        return "SECANT_ERR: Invalid number format";
    }

    if (value < 0) {
        return "SECANT_ERR: Cannot compute square root of negative number";
    }

    double result = secantSqrt(value);
    return QString("SECANT_RESULT: %1").arg(result, 0, 'f', 10);
}

// ==================== ПРОВЕРКА ЦИКЛА ====================

QString handleCycle(const QString &payload)
{
    initGraph();

    // 1. Очищаем от пробелов и разбиваем
    QString cleaned = payload;
    cleaned.remove(' ');
    cleaned.remove('\t');
    cleaned.remove('\n');
    cleaned.remove('\r');

    QStringList vertices = cleaned.split(',', Qt::SkipEmptyParts);

    // 2. Проверяем минимальную длину
    if (vertices.size() < 3) {
        return "CYCLE_ERR: Cycle must have at least 3 vertices";
    }

    // 3. Проверяем замыкание
    if (vertices.first() != vertices.last()) {
        return QString("CYCLE_ERR: Not closed (first=%1, last=%2)")
        .arg(vertices.first(), vertices.last());
    }

    // 4. Проверяем рёбра и дубликаты
    QSet<QString> seen;

    for (int i = 0; i < vertices.size() - 1; ++i) {
        QString uStr = vertices[i];
        QString vStr = vertices[i + 1];

        bool uOk, vOk;
        int u = uStr.toInt(&uOk);
        int v = vStr.toInt(&vOk);

        if (!uOk || !vOk) {
            return QString("CYCLE_ERR: Invalid vertex at position %1")
            .arg(i);
        }

        // Проверка ребра
        if (!graph.hasEdge(u, v)) {
            return QString("CYCLE_ERR: No edge %1-%2 at position %3")
            .arg(u).arg(v).arg(i);
        }

        // Проверка дубликатов (кроме последнего)
        if (i < vertices.size() - 2) {
            if (seen.contains(uStr)) {
                return QString("CYCLE_ERR: Duplicate vertex %1 at position %2")
                .arg(u).arg(i);
            }
            seen.insert(uStr);
        }
    }

    int edgeCount = vertices.size() - 1;
    return QString("CYCLE_OK: Valid cycle with %1 edges").arg(edgeCount);
}

// ==================== ФУНКЦИИ ДЛЯ РАБОТЫ С ГРАФОМ ====================

QString handleAddEdge(const QString &payload)
{
    QStringList parts = payload.split(',');
    if (parts.size() != 2 && parts.size() != 3) {
        return "GRAPH_ERR: Expected format: u,v or u,v,weight (e.g., 5,6 or 5,6,3)";
    }

    bool uOk, vOk, wOk = true;
    int u = parts[0].trimmed().toInt(&uOk);
    int v = parts[1].trimmed().toInt(&vOk);
    int weight = 1;

    if (parts.size() == 3) {
        weight = parts[2].trimmed().toInt(&wOk);
        if (!wOk || weight <= 0) {
            return "GRAPH_ERR: Weight must be a positive integer";
        }
    }

    if (!uOk || !vOk) {
        return "GRAPH_ERR: Invalid vertex numbers (must be integers)";
    }

    if (u == v) {
        return "GRAPH_ERR: Cannot add self-loop edge";
    }

    initGraph();

    if (graph.hasEdge(u, v)) {
        return QString("GRAPH_ERR: Edge %1-%2 already exists").arg(u).arg(v);
    }

    graph.addEdge(u, v, weight);

    return QString("GRAPH_OK: Edge %1-%2 added with weight %3").arg(u).arg(v).arg(weight);
}

QString handleRemoveEdge(const QString &payload)
{
    QStringList parts = payload.split(',');
    if (parts.size() != 2) {
        return "GRAPH_ERR: Expected format: u,v (e.g., 5,6)";
    }

    bool uOk, vOk;
    int u = parts[0].trimmed().toInt(&uOk);
    int v = parts[1].trimmed().toInt(&vOk);

    if (!uOk || !vOk) {
        return "GRAPH_ERR: Invalid vertex numbers (must be integers)";
    }

    initGraph();

    if (!graph.hasEdge(u, v)) {
        return QString("GRAPH_ERR: Edge %1-%2 does not exist").arg(u).arg(v);
    }

    graph.removeEdge(u, v);

    return QString("GRAPH_OK: Edge %1-%2 removed successfully").arg(u).arg(v);
}

QString handleGetGraph(const QString &payload)
{
    Q_UNUSED(payload);
    initGraph();

    QString edges = graph.getEdgesString();

    if (edges.isEmpty()) {
        return "GRAPH_EDGES: Graph has no edges";
    }

    return "GRAPH_EDGES: " + edges;
}

QString handleSetEdgeWeight(const QString &payload)
{
    QStringList parts = payload.split(',');
    if (parts.size() != 3) {
        return "GRAPH_ERR: Expected format: u,v,weight (e.g., 1,2,5)";
    }

    bool uOk, vOk, wOk;
    int u = parts[0].trimmed().toInt(&uOk);
    int v = parts[1].trimmed().toInt(&vOk);
    int weight = parts[2].trimmed().toInt(&wOk);

    if (!uOk || !vOk || !wOk) {
        return "GRAPH_ERR: Invalid input (must be integers)";
    }

    if (weight <= 0) {
        return "GRAPH_ERR: Weight must be positive";
    }

    initGraph();

    if (!graph.hasEdge(u, v)) {
        return QString("GRAPH_ERR: Edge %1-%2 does not exist").arg(u).arg(v);
    }

    int oldWeight = graph.getWeight(u, v);
    graph.removeEdge(u, v);
    graph.addEdge(u, v, weight);

    return QString("GRAPH_OK: Edge %1-%2 weight changed from %3 to %4")
        .arg(u).arg(v).arg(oldWeight).arg(weight);
}

// ==================== ПОИСК КРАТЧАЙШЕГО ПУТИ (ДЕЙКСТРА) ====================

QString handleShortest(const QString &payload)
{
    QStringList parts = payload.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2) {
        return "GRAPH_ERR: Bad format (expected s,t)";
    }

    bool ok1 = false, ok2 = false;
    int s = parts[0].trimmed().toInt(&ok1);
    int t = parts[1].trimmed().toInt(&ok2);

    if (!ok1 || !ok2) {
        return "GRAPH_ERR: Invalid vertices";
    }

    initGraph();

    // Алгоритм Дейкстры
    QHash<int, int> dist;
    QHash<int, int> prev;
    QSet<int> unvisited;

    // Инициализация
    QHashIterator<int, QVector<QPair<int, int>>> it(graph.adj);
    while (it.hasNext()) {
        it.next();
        int v = it.key();
        dist[v] = INT_MAX;
        prev[v] = -1;
        unvisited.insert(v);
    }

    // Добавляем вершины, которые есть только как соседи
    QHashIterator<int, QVector<QPair<int, int>>> it2(graph.adj);
    while (it2.hasNext()) {
        it2.next();
        for (const auto& edge : it2.value()) {
            int v = edge.first;
            if (!dist.contains(v)) {
                dist[v] = INT_MAX;
                prev[v] = -1;
                unvisited.insert(v);
            }
        }
    }

    if (!dist.contains(s)) {
        return "GRAPH_ERR: Start vertex " + QString::number(s) + " not in graph";
    }
    if (!dist.contains(t)) {
        return "GRAPH_ERR: Target vertex " + QString::number(t) + " not in graph";
    }

    dist[s] = 0;

    while (!unvisited.isEmpty()) {
        int u = -1;
        int minDist = INT_MAX;
        for (int v : unvisited) {
            if (dist[v] < minDist) {
                minDist = dist[v];
                u = v;
            }
        }

        if (u == -1 || u == t) break;

        unvisited.remove(u);

        for (const auto& edge : graph.adj.value(u)) {
            int v = edge.first;
            int weight = edge.second;
            if (unvisited.contains(v)) {
                int newDist = dist[u] + weight;
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                }
            }
        }
    }

    if (dist[t] == INT_MAX) {
        return "GRAPH: No path from " + QString::number(s) + " to " + QString::number(t);
    }

    QVector<int> path;
    for (int at = t; at != -1; at = prev[at]) {
        path.prepend(at);
    }

    QStringList pathStr;
    for (int v : path) {
        pathStr << QString::number(v);
    }

    return QString("GRAPH: Shortest path from %1 to %2 (total weight=%3): %4")
        .arg(s).arg(t).arg(dist[t]).arg(pathStr.join(" -> "));
}

// ==================== ГЕНЕТИЧЕСКИЙ АЛГОРИТМ ====================

class GeneticPathFinder
{
private:
    static int POPULATION_SIZE;
    static int MAX_GENERATIONS;
    static double MUTATION_RATE;
    static double ELITE_RATIO;

    int startVertex;
    int targetVertex;
    std::mt19937 rng;

    struct Individual {
        QVector<int> path;
        int totalWeight;
        double fitness;
    };

    int calculatePathWeight(const QVector<int>& path) {
        int weight = 0;
        for (int i = 0; i < path.size() - 1; i++) {
            int w = graph.getWeight(path[i], path[i + 1]);
            if (w == -1) return INT_MAX;
            weight += w;
        }
        return weight;
    }

    QVector<int> generateRandomPath(int maxSteps) {
        QVector<int> path;
        path.append(startVertex);

        int current = startVertex;
        QSet<int> visited;
        visited.insert(current);

        for (int step = 0; step < maxSteps && current != targetVertex; step++) {
            QVector<int> neighbors = graph.getNeighbors(current);
            if (neighbors.isEmpty()) break;

            std::uniform_int_distribution<int> dist(0, neighbors.size() - 1);
            int next = neighbors[dist(rng)];
            path.append(next);

            if (std::count(path.begin(), path.end(), next) > 2) break;

            current = next;
        }

        return path;
    }

    void evaluateIndividual(Individual& ind) {
        if (ind.path.isEmpty()) {
            ind.totalWeight = INT_MAX;
            ind.fitness = 0;
            return;
        }

        ind.totalWeight = calculatePathWeight(ind.path);

        if (ind.path.last() == targetVertex && ind.totalWeight < INT_MAX) {
            ind.fitness = 1.0 / (ind.totalWeight + 1);
        } else {
            int distToTarget = abs(targetVertex - ind.path.last());
            ind.fitness = 0.001 / (ind.totalWeight + distToTarget + 1);
        }
    }

    QVector<Individual> createInitialPopulation() {
        QVector<Individual> population;
        int maxSteps = graph.adj.size() * 3;

        for (int i = 0; i < POPULATION_SIZE; i++) {
            Individual ind;
            ind.path = generateRandomPath(maxSteps);
            evaluateIndividual(ind);
            population.append(ind);
        }

        return population;
    }

    Individual tournamentSelection(const QVector<Individual>& population) {
        std::uniform_int_distribution<int> dist(0, population.size() - 1);
        int tournamentSize = 3;
        Individual best = population[dist(rng)];

        for (int i = 1; i < tournamentSize; i++) {
            Individual contender = population[dist(rng)];
            if (contender.fitness > best.fitness) {
                best = contender;
            }
        }

        return best;
    }

    Individual crossover(const Individual& parent1, const Individual& parent2) {
        Individual child;

        int crossPoint1 = -1, crossPoint2 = -1;

        for (int i = 1; i < parent1.path.size() - 1 && crossPoint1 == -1; i++) {
            for (int j = 1; j < parent2.path.size() - 1 && crossPoint2 == -1; j++) {
                if (parent1.path[i] == parent2.path[j]) {
                    crossPoint1 = i;
                    crossPoint2 = j;
                }
            }
        }

        if (crossPoint1 != -1 && crossPoint2 != -1) {
            for (int i = 0; i <= crossPoint1; i++) {
                child.path.append(parent1.path[i]);
            }
            for (int i = crossPoint2 + 1; i < parent2.path.size(); i++) {
                child.path.append(parent2.path[i]);
            }
        } else {
            child.path = (parent1.fitness > parent2.fitness) ? parent1.path : parent2.path;
        }

        QSet<int> seen;
        QVector<int> uniquePath;
        for (int v : child.path) {
            if (!seen.contains(v)) {
                seen.insert(v);
                uniquePath.append(v);
            }
        }
        child.path = uniquePath;

        evaluateIndividual(child);
        return child;
    }

    void mutate(Individual& ind) {
        std::uniform_real_distribution<double> probDist(0.0, 1.0);

        for (int i = 0; i < ind.path.size() - 1; i++) {
            if (probDist(rng) < MUTATION_RATE) {
                int current = ind.path[i];
                QVector<int> neighbors = graph.getNeighbors(current);

                if (!neighbors.isEmpty()) {
                    std::uniform_int_distribution<int> dist(0, neighbors.size() - 1);
                    int newVertex = neighbors[dist(rng)];
                    ind.path[i + 1] = newVertex;
                }
            }
        }

        evaluateIndividual(ind);
    }

public:
    GeneticPathFinder() : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}

    static void setParameters(int popSize, int maxGen, double mutationRate, double eliteRatio) {
        POPULATION_SIZE = popSize;
        MAX_GENERATIONS = maxGen;
        MUTATION_RATE = mutationRate;
        ELITE_RATIO = eliteRatio;
    }

    QPair<QVector<int>, int> findShortestPath(int s, int t) {
        startVertex = s;
        targetVertex = t;

        QVector<Individual> population = createInitialPopulation();
        std::sort(population.begin(), population.end(),
                  [](const Individual& a, const Individual& b) { return a.fitness > b.fitness; });

        int bestWeight = INT_MAX;
        QVector<int> bestPath;
        int generationsWithoutImprovement = 0;

        for (int gen = 0; gen < MAX_GENERATIONS; gen++) {
            QVector<Individual> newPopulation;

            int eliteCount = static_cast<int>(POPULATION_SIZE * ELITE_RATIO);
            for (int i = 0; i < eliteCount && i < population.size(); i++) {
                newPopulation.append(population[i]);
            }

            while (newPopulation.size() < POPULATION_SIZE) {
                Individual parent1 = tournamentSelection(population);
                Individual parent2 = tournamentSelection(population);
                Individual child = crossover(parent1, parent2);
                mutate(child);
                newPopulation.append(child);
            }

            population = newPopulation;
            std::sort(population.begin(), population.end(),
                      [](const Individual& a, const Individual& b) { return a.fitness > b.fitness; });

            if (population[0].path.last() == targetVertex && population[0].totalWeight < bestWeight) {
                bestWeight = population[0].totalWeight;
                bestPath = population[0].path;
                generationsWithoutImprovement = 0;
            } else {
                generationsWithoutImprovement++;
            }

            if (generationsWithoutImprovement > 30) break;
        }

        return QPair<QVector<int>, int>(bestPath, bestWeight);
    }

    QString pathToString(const QVector<int>& path) {
        if (path.isEmpty()) return "No path found";
        QStringList vertices;
        for (int v : path) vertices.append(QString::number(v));
        return vertices.join(" -> ");
    }
};

// Статические переменные генетического алгоритма
int GeneticPathFinder::POPULATION_SIZE = 100;
int GeneticPathFinder::MAX_GENERATIONS = 200;
double GeneticPathFinder::MUTATION_RATE = 0.1;
double GeneticPathFinder::ELITE_RATIO = 0.1;

// ==================== ПОИСК ПУТИ ГЕНЕТИЧЕСКИМ АЛГОРИТМОМ ====================

QString handleGeneticPath(const QString &payload)
{
    QStringList parts = payload.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2) {
        return "GENETIC_ERR: Bad format (expected s,t)";
    }

    bool ok1 = false, ok2 = false;
    int s = parts[0].trimmed().toInt(&ok1);
    int t = parts[1].trimmed().toInt(&ok2);

    if (!ok1 || !ok2) {
        return "GENETIC_ERR: Invalid vertices (must be integers)";
    }

    initGraph();

    // Проверка существования вершин
    bool sExists = false, tExists = false;
    QHashIterator<int, QVector<QPair<int, int>>> it(graph.adj);
    while (it.hasNext()) {
        it.next();
        if (it.key() == s) sExists = true;
        if (it.key() == t) tExists = true;
        for (const auto& edge : it.value()) {
            if (edge.first == s) sExists = true;
            if (edge.first == t) tExists = true;
        }
    }

    if (!sExists && s != t) {
        return "GENETIC_ERR: Start vertex " + QString::number(s) + " not in graph";
    }
    if (!tExists && s != t) {
        return "GENETIC_ERR: Target vertex " + QString::number(t) + " not in graph";
    }

    QElapsedTimer timer;
    timer.start();

    GeneticPathFinder finder;
    QPair<QVector<int>, int> result = finder.findShortestPath(s, t);
    QVector<int> path = result.first;
    int distance = result.second;

    qDebug() << "Genetic algorithm took" << timer.elapsed() << "ms";

    if (distance == INT_MAX || path.isEmpty()) {
        return "GENETIC_RESULT: No path found from " + QString::number(s) +
               " to " + QString::number(t);
    }

    QString pathStr = finder.pathToString(path);
    return QString("GENETIC_RESULT: Path from %1 to %2 (weight=%3): %4")
        .arg(s).arg(t).arg(distance).arg(pathStr);
}

// ==================== АДМИНИСТРАТИВНЫЕ ФУНКЦИИ ====================

QString handleAdminAuth(const QString &payload)
{
    QStringList parts = payload.split(',');
    if (parts.size() != 2) {
        return "ADMIN_AUTH_ERR: Expected login,password";
    }

    QString login = parts[0].trimmed();
    QString password = parts[1].trimmed();

    QString sql = QString(
                      "SELECT login FROM User WHERE login='%1' AND password='%2' AND status='admin';"
                      ).arg(login, password);

    QString resp = DbClient::instance().sendQuery(sql);

    if (resp.startsWith("ERROR")) {
        return "ADMIN_AUTH_ERR: DB error";
    }

    auto lines = splitLines(resp);
    if (lines.size() < 2) {
        return "ADMIN_AUTH_ERR: Invalid credentials or not admin";
    }

    return "ADMIN_AUTH_OK: Access granted";
}

QString handleAdminGetUsers(const QString &payload)
{
    Q_UNUSED(payload);

    QString sql = "SELECT login, status FROM User ORDER BY login;";
    QString resp = DbClient::instance().sendQuery(sql);

    if (resp.startsWith("ERROR")) {
        return "ADMIN_ERR: " + resp;
    }

    // Преобразуем многострочный ответ в одну строку
    QStringList lines = resp.split("\r\n", Qt::SkipEmptyParts);
    QStringList formattedLines;

    for (const QString &line : lines) {
        // Пропускаем заголовок "login\tstatus"
        if (line.contains("login") && line.contains("status")) continue;

        QStringList parts = line.split("\t");
        if (parts.size() >= 2) {
            formattedLines << parts[0] + " | " + parts[1];
        }
    }

    QString result = formattedLines.join(", ");
    return "ADMIN_USERS: " + result;
}

QString handleAdminSetStatus(const QString &payload)
{
    QStringList parts = payload.split(',');
    if (parts.size() != 2) {
        return "ADMIN_ERR: Invalid format, expected login,status";
    }

    QString login = parts[0].trimmed();
    QString status = parts[1].trimmed().toLower();

    const QStringList allowedStatuses = {"online", "offline", "banned", "admin"};
    if (!allowedStatuses.contains(status)) {
        return "ADMIN_ERR: Invalid status. Use: online, offline, banned, admin";
    }

    // Защита от изменения последнего администратора
    if (status != "admin") {
        QString adminCountSql = "SELECT COUNT(*) FROM User WHERE status='admin';";
        QString adminCountResp = DbClient::instance().sendQuery(adminCountSql);
        auto adminLines = splitLines(adminCountResp);
        int adminCount = (adminLines.size() > 1) ? adminLines[1].toInt() : 0;

        if (adminCount <= 1) {
            QString checkAdminSql = QString("SELECT COUNT(*) FROM User WHERE login='%1' AND status='admin';").arg(login);
            QString checkResp = DbClient::instance().sendQuery(checkAdminSql);
            auto checkLines = splitLines(checkResp);
            int isAdmin = (checkLines.size() > 1) ? checkLines[1].toInt() : 0;

            if (isAdmin > 0) {
                return "ADMIN_ERR: Cannot change status of the last admin";
            }
        }
    }

    QString updateSql = QString("UPDATE User SET status='%1' WHERE login='%2';").arg(status, login);
    QString result = DbClient::instance().sendQuery(updateSql);

    if (result.startsWith("ERROR")) {
        return "ADMIN_ERR: Database error: " + result;
    }

    return QString("ADMIN_OK: Status updated for user %1").arg(login);
}

// ==================== RSA ====================

class SimpleRSA {
private:
    long long n;      // модуль
    long long e;      // открытая экспонента
    long long d;      // закрытая экспонента

    long long gcd(long long a, long long b) {
        while (b != 0) {
            long long t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    long long modInverse(long long a, long long m) {
        long long m0 = m, t, q;
        long long x0 = 0, x1 = 1;

        if (m == 1) return 0;

        while (a > 1) {
            q = a / m;
            t = m;
            m = a % m;
            a = t;
            t = x0;
            x0 = x1 - q * x0;
            x1 = t;
        }

        if (x1 < 0) x1 += m0;
        return x1;
    }

    long long modPow(long long base, long long exp, long long mod) {
        long long result = 1;
        base = base % mod;

        while (exp > 0) {
            if (exp & 1) {
                result = (result * base) % mod;
            }
            base = (base * base) % mod;
            exp >>= 1;
        }
        return result;
    }

    bool isPrime(long long num) {
        if (num < 2) return false;
        if (num == 2) return true;
        if (num % 2 == 0) return false;

        for (long long i = 3; i * i <= num; i += 2) {
            if (num % i == 0) return false;
        }
        return true;
    }

    long long generatePrime(long long min, long long max) {
        static unsigned int seed = static_cast<unsigned int>(QDateTime::currentMSecsSinceEpoch());
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;

        long long range = max - min + 1;
        long long num;
        do {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            num = min + (seed % range);
        } while (!isPrime(num));

        return num;
    }

public:
    SimpleRSA() {
        long long p = generatePrime(50, 100);
        long long q = generatePrime(50, 100);

        n = p * q;
        long long phi = (p - 1) * (q - 1);

        e = 17;
        while (gcd(e, phi) != 1) {
            e++;
        }

        d = modInverse(e, phi);

        qDebug() << "RSA Keys generated:";
        qDebug() << "n=" << n;
        qDebug() << "e=" << e;
        qDebug() << "d=" << d;
    }

    QString getPublicKey() {
        return QString("RSA_PUBKEY:%1,%2").arg(n).arg(e);
    }

    long long encrypt(long long message) {
        return modPow(message, e, n);
    }

    long long decrypt(long long cipher) {
        return modPow(cipher, d, n);
    }

    QString decryptString(const QString &cipher) {
        QStringList parts = cipher.split(",");
        QString result;
        for (const QString &part : parts) {
            bool ok;
            long long c = part.toLongLong(&ok);
            if (ok) {
                long long m = decrypt(c);
                result.append(static_cast<QChar>(static_cast<char16_t>(m)));
            }
        }
        return result;
    }
};

static SimpleRSA* rsa = nullptr;

void initRSA() {
    if (!rsa) {
        rsa = new SimpleRSA();
    }
}

QString handleRsaPubkey(const QString &payload) {
    Q_UNUSED(payload);
    initRSA();
    return rsa->getPublicKey();
}

QString handleRsaEncrypt(const QString &payload) {
    initRSA();
    QString decrypted = rsa->decryptString(payload);
    return "RSA_DECRYPT:" + decrypted;
}
