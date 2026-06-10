#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "regandauth.h"
#include <QMenu>
#include <QCursor>
#include <QInputDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ========== ПОДКЛЮЧЕНИЕ СУЩЕСТВУЮЩИХ КНОПОК ==========
    connect(ui->md5Button, &QPushButton::clicked, this, &MainWindow::on_md5Button_clicked);
    connect(ui->secantButton, &QPushButton::clicked, this, &MainWindow::on_secantButton_clicked);
    connect(ui->cycleButton, &QPushButton::clicked, this, &MainWindow::on_cycleButton_clicked);
    connect(ui->adminButton, &QPushButton::clicked, this, &MainWindow::on_adminButton_clicked);
    connect(ui->backButton, &QPushButton::clicked, this, &MainWindow::on_backButton_clicked);

    // ========== ПОДКЛЮЧЕНИЕ КНОПОК ДЛЯ РАБОТЫ С ГРАФОМ ==========
    if (ui->refreshGraphButton) {
        connect(ui->refreshGraphButton, &QPushButton::clicked,
                this, &MainWindow::on_refreshGraphButton_clicked);
    }
    if (ui->addEdgeConfirmButton) {
        connect(ui->addEdgeConfirmButton, &QPushButton::clicked,
                this, &MainWindow::on_addEdgeConfirmButton_clicked);
    }
    if (ui->removeEdgeConfirmButton) {
        connect(ui->removeEdgeConfirmButton, &QPushButton::clicked,
                this, &MainWindow::on_removeEdgeConfirmButton_clicked);
    }
    if (ui->setWeightButton) {
        connect(ui->setWeightButton, &QPushButton::clicked,
                this, &MainWindow::on_setWeightButton_clicked);
    }
    if (ui->dijkstraButton) {
        connect(ui->dijkstraButton, &QPushButton::clicked,
                this, &MainWindow::on_dijkstraButton_clicked);
    }
    if (ui->geneticButton) {
        connect(ui->geneticButton, &QPushButton::clicked,
                this, &MainWindow::on_geneticButton_clicked);
    }
    if (ui->applyGeneticParamsButton) {
        connect(ui->applyGeneticParamsButton, &QPushButton::clicked,
                this, &MainWindow::on_applyGeneticParamsButton_clicked);
    }

    // ========== ПОДКЛЮЧЕНИЕ КНОПОК ДЛЯ RSA ДЕМОНСТРАЦИИ ==========
    if (ui->generateKeysButton) {
        connect(ui->generateKeysButton, &QPushButton::clicked,
                this, &MainWindow::on_generateKeysButton_clicked);
    }
    if (ui->encryptButton) {
        connect(ui->encryptButton, &QPushButton::clicked,
                this, &MainWindow::on_encryptButton_clicked);
    }
    if (ui->decryptButton) {
        connect(ui->decryptButton, &QPushButton::clicked,
                this, &MainWindow::on_decryptButton_clicked);
    }

    // ========== ПОДКЛЮЧЕНИЕ ОТВЕТОВ ОТ СЕРВЕРА ==========
    connect(Client::getInstance(), &Client::msgFromServer,
            this, &MainWindow::onServerResponse);

    // ========== ПОДКЛЮЧЕНИЕ ИЗМЕНЕНИЯ СТАТУСА АДМИНИСТРАТОРА ==========
    connect(Client::getInstance(), &Client::adminStatusChanged,
            this, &MainWindow::updateAdminUI);

    // ========== НАСТРОЙКА ВИДИМОСТИ КНОПКИ АДМИНИСТРАТОРА ==========
    ui->adminButton->setVisible(Client::getInstance()->isAdmin());

    // ========== НАСТРОЙКА ПОЛЕЙ RSA ДЕМОНСТРАЦИИ ==========
    if (ui->encryptedResultEdit) {
        ui->encryptedResultEdit->setPlainText("Здесь появится зашифрованное сообщение...");
        ui->encryptedResultEdit->setReadOnly(false);
    }
    if (ui->decryptedResultEdit) {
        ui->decryptedResultEdit->setPlainText("Здесь появится расшифрованное сообщение...");
        ui->decryptedResultEdit->setReadOnly(true);
    }
    if (ui->messageEdit) {
        ui->messageEdit->setPlainText("Hello, RSA! Это тестовое сообщение для демонстрации шифрования");
    }

    // ========== НАСТРОЙКА ПОЛЕЙ ГРАФА ==========
    if (ui->edgesListTextEdit) {
        ui->edgesListTextEdit->setReadOnly(true);
        ui->edgesListTextEdit->setPlainText("Нажмите 'Обновить список' для загрузки графа");
    }
    if (ui->shortestResultEdit) {
        ui->shortestResultEdit->setReadOnly(true);
        ui->shortestResultEdit->setPlainText("Результаты поиска пути будут отображаться здесь...");
    }
    if (ui->popSizeEdit) ui->popSizeEdit->setText("100");
    if (ui->maxGenEdit) ui->maxGenEdit->setText("200");
    if (ui->mutationRateEdit) ui->mutationRateEdit->setText("0.1");
    if (ui->eliteRatioEdit) ui->eliteRatioEdit->setText("0.1");
    if (ui->shortestStartEdit) ui->shortestStartEdit->setText("1");
    if (ui->shortestTargetEdit) ui->shortestTargetEdit->setText("4");
    if (ui->addEdgeWeightEdit) ui->addEdgeWeightEdit->setText("1");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateAdminUI(bool isAdmin)
{
    ui->adminButton->setVisible(isAdmin);
}

void MainWindow::onServerResponse(const QString &response)
{
    qDebug() << "MainWindow received:" << response;

    // ========== ОТВЕТЫ ДЛЯ ГРАФА ==========
    if (response.startsWith("GRAPH_EDGES:")) {
        QString edges = response.mid(12);
        if (ui->edgesListTextEdit) {
            ui->edgesListTextEdit->setPlainText(edges);
        }
        ui->outputLabel->setText("✅ Граф загружен");
        return;
    }
    else if (response.startsWith("GRAPH_OK")) {
        ui->outputLabel->setText("✅ " + response);
        if (ui->refreshGraphButton) on_refreshGraphButton_clicked();
        return;
    }
    else if (response.startsWith("GRAPH_ERR")) {
        ui->outputLabel->setText("❌ " + response);
        return;
    }

    // ========== ПРОВЕРКА ЦИКЛА ==========
    else if (response.startsWith("CYCLE_OK")) {
        ui->outputLabel->setText("✅ " + response);
        return;
    }
    else if (response.startsWith("CYCLE_ERR")) {
        ui->outputLabel->setText("❌ " + response);
        return;
    }

    // ========== ПОИСК ПУТИ (ДЕЙКСТРА) ==========
    else if (response.startsWith("GRAPH:")) {
        if (ui->shortestResultEdit) {
            ui->shortestResultEdit->append("📏 " + response);
        }
        ui->outputLabel->setText("✅ Точный путь найден");
        return;
    }

    // ========== ПОИСК ПУТИ (ГЕНЕТИЧЕСКИЙ) ==========
    else if (response.startsWith("GENETIC_RESULT:")) {
        if (ui->shortestResultEdit) {
            ui->shortestResultEdit->append("🧬 " + response);
        }
        ui->outputLabel->setText("✅ Генетический путь найден");
        return;
    }
    else if (response.startsWith("GENETIC_ERR")) {
        if (ui->shortestResultEdit) {
            ui->shortestResultEdit->append("❌ " + response);
        }
        ui->outputLabel->setText("❌ Ошибка генетического поиска");
        return;
    }

    // ========== MD5 ==========
    else if (response.startsWith("MD5:")) {
        ui->md5ResultLabel->setText("🔐 " + response);
    }
    else if (response.startsWith("MD5_ERR")) {
        ui->md5ResultLabel->setText("❌ " + response);
    }

    // ========== МЕТОД СЕКУЩИХ ==========
    else if (response.startsWith("SECANT_RESULT:")) {
        ui->secantResultLabel->setText("📐 " + response);
    }
    else if (response.startsWith("SECANT_ERR")) {
        ui->secantResultLabel->setText("❌ " + response);
    }

    // ========== АДМИНИСТРАТИВНЫЕ ОТВЕТЫ ==========
    else if (response.startsWith("ADMIN_USERS:")) {
        QString formatted = response.mid(12);
        formatted.replace("\t", " | ");
        QMessageBox::information(this, "Список пользователей", formatted);
        ui->outputLabel->setText("✅ Список пользователей получен");
    }
    else if (response.startsWith("ADMIN_OK")) {
        ui->outputLabel->setText("✅ " + response);
    }
    else if (response.startsWith("ADMIN_ERR")) {
        ui->outputLabel->setText("❌ " + response);
    }

    // ========== СТАТИСТИКА ==========
    else if (response.startsWith("STATS")) {
        ui->outputLabel->setText("📊 " + response);
    }

    // ========== ОБЩИЕ СООБЩЕНИЯ ==========
    else {
        ui->outputLabel->setText(response);
    }
}

// ========== MD5 ==========
void MainWindow::on_md5Button_clicked()
{
    QString text = ui->md5InputEdit->text().trimmed();
    if (text.isEmpty()) {
        ui->md5ResultLabel->setText("❌ Введите текст");
        return;
    }
    Client::getInstance()->sendMsgToServer("md5&" + text);
    ui->md5ResultLabel->setText("⏳ Вычисление MD5...");
}

// ========== МЕТОД СЕКУЩИХ ==========
void MainWindow::on_secantButton_clicked()
{
    QString value = ui->secantInputEdit->text().trimmed();
    if (value.isEmpty()) {
        ui->secantResultLabel->setText("❌ Введите число");
        return;
    }
    Client::getInstance()->sendMsgToServer("secant&" + value);
    ui->secantResultLabel->setText("⏳ Вычисление корня...");
}

// ========== ПРОВЕРКА ЦИКЛА ==========
void MainWindow::on_cycleButton_clicked()
{
    QString sequence = ui->cycleInputEdit->text().trimmed();
    if (sequence.isEmpty()) {
        ui->outputLabel->setText("❌ Введите последовательность вершин");
        return;
    }
    Client::getInstance()->sendMsgToServer("cycle&" + sequence);
    ui->outputLabel->setText("⏳ Проверка цикла...");
}

// ========== ОБНОВИТЬ СПИСОК РЁБЕР ==========
void MainWindow::on_refreshGraphButton_clicked()
{
    Client::getInstance()->sendMsgToServer("get_graph&");
    if (ui->edgesListTextEdit) {
        ui->edgesListTextEdit->setPlainText("⏳ Загрузка графа...");
    }
    ui->outputLabel->setText("⏳ Запрос графа...");
}

// ========== ДОБАВИТЬ РЕБРО ==========
void MainWindow::on_addEdgeConfirmButton_clicked()
{
    QString edge = ui->addEdgeInputEdit->text().trimmed();
    QString weight = ui->addEdgeWeightEdit->text().trimmed();
    if (weight.isEmpty()) weight = "1";

    if (edge.isEmpty()) {
        ui->outputLabel->setText("❌ Введите ребро (например, 5,6)");
        return;
    }

    if (!edge.contains(',')) {
        ui->outputLabel->setText("❌ Используйте формат: u,v (например, 5,6)");
        return;
    }

    Client::getInstance()->sendMsgToServer("add_edge&" + edge + "," + weight);
    ui->addEdgeInputEdit->clear();
    ui->outputLabel->setText("⏳ Добавление ребра...");
}

// ========== УДАЛИТЬ РЕБРО ==========
void MainWindow::on_removeEdgeConfirmButton_clicked()
{
    QString edge = ui->removeEdgeInputEdit->text().trimmed();
    if (edge.isEmpty()) {
        ui->outputLabel->setText("❌ Введите ребро для удаления");
        return;
    }

    if (!edge.contains(',')) {
        ui->outputLabel->setText("❌ Используйте формат: u,v (например, 1,2)");
        return;
    }

    Client::getInstance()->sendMsgToServer("remove_edge&" + edge);
    ui->removeEdgeInputEdit->clear();
    ui->outputLabel->setText("⏳ Удаление ребра...");
}

// ========== ИЗМЕНИТЬ ВЕС РЕБРА ==========
void MainWindow::on_setWeightButton_clicked()
{
    QString edge = ui->setWeightEdgeInputEdit->text().trimmed();
    QString weight = ui->setWeightValueEdit->text().trimmed();

    if (edge.isEmpty() || weight.isEmpty()) {
        ui->outputLabel->setText("❌ Введите ребро и новый вес");
        return;
    }

    if (!edge.contains(',')) {
        ui->outputLabel->setText("❌ Используйте формат: u,v (например, 1,2)");
        return;
    }

    bool weightOk;
    int newWeight = weight.toInt(&weightOk);
    if (!weightOk || newWeight <= 0) {
        ui->outputLabel->setText("❌ Вес должен быть положительным целым числом");
        return;
    }

    Client::getInstance()->sendMsgToServer("set_weight&" + edge + "," + weight);
    ui->setWeightEdgeInputEdit->clear();
    ui->setWeightValueEdit->clear();
    ui->outputLabel->setText("⏳ Изменение веса ребра...");
}

// ========== ТОЧНЫЙ ПУТЬ (ДЕЙКСТРА) ==========
void MainWindow::on_dijkstraButton_clicked()
{
    QString start = ui->shortestStartEdit->text().trimmed();
    QString target = ui->shortestTargetEdit->text().trimmed();

    if (start.isEmpty() || target.isEmpty()) {
        if (ui->shortestResultEdit) {
            ui->shortestResultEdit->append("❌ Введите начальную и целевую вершины");
        }
        return;
    }

    bool startOk, targetOk;
    start.toInt(&startOk);
    target.toInt(&targetOk);

    if (!startOk || !targetOk) {
        if (ui->shortestResultEdit) {
            ui->shortestResultEdit->append("❌ Вершины должны быть целыми числами");
        }
        return;
    }

    Client::getInstance()->sendMsgToServer("shortest&" + start + "," + target);
    if (ui->shortestResultEdit) {
        ui->shortestResultEdit->append("📏 Запрос точного пути от " + start + " до " + target + "...");
    }
    ui->outputLabel->setText("⏳ Поиск точного пути...");
}

// ========== ГЕНЕТИЧЕСКИЙ ПУТЬ ==========
void MainWindow::on_geneticButton_clicked()
{
    QString start = ui->shortestStartEdit->text().trimmed();
    QString target = ui->shortestTargetEdit->text().trimmed();

    if (start.isEmpty() || target.isEmpty()) {
        if (ui->shortestResultEdit) {
            ui->shortestResultEdit->append("❌ Введите начальную и целевую вершины");
        }
        return;
    }

    bool startOk, targetOk;
    start.toInt(&startOk);
    target.toInt(&targetOk);

    if (!startOk || !targetOk) {
        if (ui->shortestResultEdit) {
            ui->shortestResultEdit->append("❌ Вершины должны быть целыми числами");
        }
        return;
    }

    Client::getInstance()->sendMsgToServer("genetic_path&" + start + "," + target);
    if (ui->shortestResultEdit) {
        ui->shortestResultEdit->append("🧬 Запрос генетического пути от " + start + " до " + target + "...");
    }
    ui->outputLabel->setText("⏳ Поиск генетическим алгоритмом...");
}

// ========== ПРИМЕНИТЬ ПАРАМЕТРЫ ГЕНЕТИЧЕСКОГО АЛГОРИТМА ==========
void MainWindow::on_applyGeneticParamsButton_clicked()
{
    QString popSize = ui->popSizeEdit->text().trimmed();
    QString maxGen = ui->maxGenEdit->text().trimmed();
    QString mutationRate = ui->mutationRateEdit->text().trimmed();
    QString eliteRatio = ui->eliteRatioEdit->text().trimmed();

    bool ok1, ok2, ok3, ok4;
    popSize.toInt(&ok1);
    maxGen.toInt(&ok2);
    mutationRate.toDouble(&ok3);
    eliteRatio.toDouble(&ok4);

    if (!ok1 || !ok2 || !ok3 || !ok4) {
        ui->outputLabel->setText("❌ Неверные параметры");
        return;
    }

    QString params = popSize + "," + maxGen + "," + mutationRate + "," + eliteRatio;
    Client::getInstance()->sendMsgToServer("set_genetic_params&" + params);
    if (ui->shortestResultEdit) {
        ui->shortestResultEdit->append("⚙️ Параметры генетического алгоритма обновлены: популяция=" + popSize + ", поколения=" + maxGen + ", мутация=" + mutationRate + ", элита=" + eliteRatio);
    }
    ui->outputLabel->setText("✅ Параметры применены");
}

// ========== ГЕНЕРАЦИЯ КЛЮЧЕЙ (ДЕМОНСТРАЦИОННАЯ ВКЛАДКА) ==========
void MainWindow::on_generateKeysButton_clicked()
{
    localRSA.generateKeys();

    if (ui->publicNEdit) ui->publicNEdit->setText(QString::number(localRSA.getN()));
    if (ui->publicEEdit) ui->publicEEdit->setText(QString::number(localRSA.getE()));
    if (ui->privateNEdit) ui->privateNEdit->setText(QString::number(localRSA.getN()));
    if (ui->privateDEdit) ui->privateDEdit->setText(QString::number(localRSA.getD()));

    if (ui->encryptedResultEdit) {
        ui->encryptedResultEdit->setPlainText("Ключи сгенерированы. Введите сообщение и нажмите 'Зашифровать'.");
    }
    if (ui->decryptedResultEdit) {
        ui->decryptedResultEdit->setPlainText("");
    }

    qDebug() << "Keys generated: n=" << localRSA.getN() << "e=" << localRSA.getE() << "d=" << localRSA.getD();
}

// ========== ШИФРОВАНИЕ (ДЕМОНСТРАЦИОННАЯ ВКЛАДКА) ==========
void MainWindow::on_encryptButton_clicked()
{
    if (!ui->messageEdit) return;

    QString message = ui->messageEdit->toPlainText().trimmed();
    if (message.isEmpty()) {
        if (ui->encryptedResultEdit) {
            ui->encryptedResultEdit->setPlainText("❌ Ошибка: Введите сообщение для шифрования");
        }
        return;
    }

    long long n = ui->publicNEdit ? ui->publicNEdit->text().toLongLong() : 0;
    long long e = ui->publicEEdit ? ui->publicEEdit->text().toLongLong() : 0;

    if (n <= 0 || e <= 0) {
        if (ui->encryptedResultEdit) {
            ui->encryptedResultEdit->setPlainText("❌ Ошибка: Введите публичный ключ (n, e) или нажмите 'Сгенерировать ключи'");
        }
        return;
    }

    localRSA.setPublicKey(n, e);
    QString encrypted = localRSA.encryptString(message);

    if (encrypted.startsWith("ERROR")) {
        if (ui->encryptedResultEdit) {
            ui->encryptedResultEdit->setPlainText("❌ " + encrypted);
        }
        return;
    }

    if (ui->encryptedResultEdit) {
        ui->encryptedResultEdit->setPlainText(encrypted);
    }
    if (ui->decryptedResultEdit) {
        ui->decryptedResultEdit->setPlainText("");
    }

    qDebug() << "Encrypted message length:" << encrypted.length();
}

// ========== РАСШИФРОВКА (ДЕМОНСТРАЦИОННАЯ ВКЛАДКА) ==========
void MainWindow::on_decryptButton_clicked()
{
    if (!ui->encryptedResultEdit) return;

    QString encrypted = ui->encryptedResultEdit->toPlainText().trimmed();
    if (encrypted.isEmpty()) {
        if (ui->decryptedResultEdit) {
            ui->decryptedResultEdit->setPlainText("❌ Ошибка: Нет зашифрованного сообщения");
        }
        return;
    }

    if (encrypted.startsWith("❌") || encrypted.startsWith("Ключи") ||
        encrypted.startsWith("Здесь") || encrypted.startsWith("✅")) {
        if (ui->decryptedResultEdit) {
            ui->decryptedResultEdit->setPlainText("❌ Ошибка: Некорректное зашифрованное сообщение");
        }
        return;
    }

    long long n = ui->privateNEdit ? ui->privateNEdit->text().toLongLong() : 0;
    long long d = ui->privateDEdit ? ui->privateDEdit->text().toLongLong() : 0;

    if (n <= 0 || d <= 0) {
        if (ui->decryptedResultEdit) {
            ui->decryptedResultEdit->setPlainText("❌ Ошибка: Введите закрытый ключ (n, d) или нажмите 'Сгенерировать ключи'");
        }
        return;
    }

    localRSA.setPrivateKey(n, d);
    QString decrypted = localRSA.decryptString(encrypted);

    if (decrypted.startsWith("ERROR")) {
        if (ui->decryptedResultEdit) {
            ui->decryptedResultEdit->setPlainText("❌ " + decrypted);
        }
        return;
    }

    if (ui->decryptedResultEdit) {
        ui->decryptedResultEdit->setPlainText(decrypted);
    }

    qDebug() << "Decrypted message:" << decrypted;
}

// ========== АДМИНИСТРАТИВНОЕ МЕНЮ ==========
void MainWindow::on_adminButton_clicked()
{
    QMenu adminMenu;
    adminMenu.setTitle("Администрирование");

    QAction *usersAction = adminMenu.addAction("📋 Список пользователей");
    QAction *statusAction = adminMenu.addAction("🔄 Изменить статус пользователя");
    QAction *logoutAction = adminMenu.addAction("🚪 Выйти из режима администратора");

    QAction *selected = adminMenu.exec(QCursor::pos());

    if (selected == usersAction) {
        Client::getInstance()->sendMsgToServer("admin_users&");
        ui->outputLabel->setText("⏳ Запрос списка пользователей...");
    }
    else if (selected == statusAction) {
        bool ok;
        QString login = QInputDialog::getText(this, "Изменение статуса",
                                              "Введите логин пользователя:",
                                              QLineEdit::Normal, "", &ok);
        if (!ok || login.isEmpty()) return;

        QStringList statuses = {"online", "offline", "banned", "admin"};
        QString status = QInputDialog::getItem(this, "Изменение статуса",
                                               "Выберите новый статус:",
                                               statuses, 0, false, &ok);
        if (!ok || status.isEmpty()) return;

        Client::getInstance()->sendMsgToServer(QString("admin_setstatus&%1,%2").arg(login, status));
        ui->outputLabel->setText(QString("⏳ Изменение статуса пользователя %1...").arg(login));
    }
    else if (selected == logoutAction) {
        Client::getInstance()->setAdmin(false);
        ui->outputLabel->setText("Вы вышли из режима администратора");
    }
}

// ========== ВЫХОД В ОКНО РЕГИСТРАЦИИ ==========
void MainWindow::on_backButton_clicked()
{
    this->close();
    RegAndAuth *regWindow = new RegAndAuth();
    regWindow->show();
}
