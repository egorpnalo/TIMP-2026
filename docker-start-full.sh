#!/bin/bash

echo "========================================="
echo "=== ЗАПУСК ПОЛНОГО ПРИЛОЖЕНИЯ В DOCKER ==="
echo "========================================="

# Запускаем VNC-сервер в фоне
echo "[1/4] Запуск VNC-сервера..."
/app/vnc-start.sh &
VNC_PID=$!
echo "      VNC-сервер запущен (PID: $VNC_PID)"

# Ждём инициализации VNC
sleep 5

# Запускаем DbServer (сервер базы данных)
echo "[2/4] Запуск DbServer (порт 33334)..."
/app/server/DbServer/build/DbServer &
DB_PID=$!
echo "      DbServer запущен (PID: $DB_PID)"

sleep 2

# Запускаем MainServer (основной сервер)
echo "[3/4] Запуск MainServer (порт 33333)..."
/app/server/Server/build/Server &
MAIN_PID=$!
echo "      MainServer запущен (PID: $MAIN_PID)"

sleep 2

# Запускаем клиент в виртуальном дисплее
echo "[4/4] Запуск Клиента..."
DISPLAY=:99 /app/client/client/build/client &
CLIENT_PID=$!
echo "      Клиент запущен (PID: $CLIENT_PID)"

echo "========================================="
echo "=== ВСЕ КОМПОНЕНТЫ ЗАПУЩЕНЫ! ==="
echo "========================================="
echo "=== VNC: порт 5900 ==="
echo "=== Пароль VNC: 123456 ==="
echo "=== DbServer: порт 33334 ==="
echo "=== MainServer: порт 33333 ==="
echo "========================================="
echo ""
echo "Подключитесь к VNC через клиент: 127.0.0.1:5900"
echo "Для выхода нажмите Ctrl+C"
echo ""

# Ожидаем завершения процессов
wait $DB_PID $MAIN_PID $CLIENT_PID $VNC_PID