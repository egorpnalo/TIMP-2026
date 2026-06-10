# Используем Ubuntu 22.04 как основу
FROM ubuntu:22.04

# Отключаем интерактивные запросы при установке пакетов
ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем необходимые пакеты
RUN apt-get update && apt-get install -y \
    qt5-qmake \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5core5a \
    libqt5network5 \
    libqt5sql5 \
    libqt5sql5-sqlite \
    libssl-dev \
    build-essential \
    cmake \
    make \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Создаём рабочую директорию внутри контейнера
WORKDIR /app

# Копируем исходники серверов
COPY server/ /app/server/

# ========== СБОРКА DbServer ==========
RUN mkdir -p /app/server/DbServer/build && \
    cd /app/server/DbServer/build && \
    qmake ../DbServer.pro && \
    make

# ========== СБОРКА MainServer ==========
RUN mkdir -p /app/server/Server/build && \
    cd /app/server/Server/build && \
    qmake ../Server.pro && \
    make

# Открываем порты
EXPOSE 33333 33334

# Скрипт запуска серверов
RUN echo '#!/bin/bash\n\
echo "=== Запуск серверов в Docker ==="\n\
echo "[1/2] Запуск DbServer (порт 33334)..."\n\
/app/server/DbServer/build/DbServer &\n\
sleep 2\n\
echo "[2/2] Запуск MainServer (порт 33333)..."\n\
/app/server/Server/build/Server &\n\
echo "=== Оба сервера запущены! ==="\n\
echo "DbServer: порт 33334"\n\
echo "MainServer: порт 33333"\n\
wait\n\
' > /app/start.sh && chmod +x /app/start.sh

CMD ["/app/start.sh"]