#include "regandauth.h"
#include "ui_regandauth.h"
#include "client.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

RegAndAuth::RegAndAuth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegAndAuth),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);

    // ========== НАСТРОЙКА НАЧАЛЬНОЙ ВИДИМОСТИ ==========
    ui->groupLogin->setVisible(true);
    ui->groupRegister->setVisible(false);
    ui->showRegisterButton->setVisible(true);
    ui->showLoginButton->setVisible(false);

    // ========== ПОДКЛЮЧЕНИЕ КНОПОК ПЕРЕКЛЮЧЕНИЯ ==========
    connect(ui->showRegisterButton, &QPushButton::clicked,
            this, &RegAndAuth::on_showRegisterButton_clicked);
    connect(ui->showLoginButton, &QPushButton::clicked,
            this, &RegAndAuth::on_showLoginButton_clicked);

    // ========== ПОДКЛЮЧЕНИЕ ОСНОВНЫХ КНОПОК ==========
    connect(ui->regButton, &QPushButton::clicked,
            this, &RegAndAuth::on_regButton_clicked);
    connect(ui->logButton, &QPushButton::clicked,
            this, &RegAndAuth::on_logButton_clicked);
    connect(ui->statsButton, &QPushButton::clicked,
            this, &RegAndAuth::on_statsButton_clicked);
    connect(ui->adminButton, &QPushButton::clicked,
            this, &RegAndAuth::on_adminButton_clicked);

    // ========== ПОДКЛЮЧЕНИЕ ОТВЕТОВ ОТ СЕРВЕРА ==========
    connect(Client::getInstance(), &Client::msgFromServer,
            this, &RegAndAuth::onServerResponse);
}

RegAndAuth::~RegAndAuth()
{
    delete ui;
}

// ========== ПЕРЕКЛЮЧЕНИЕ НА ФОРМУ РЕГИСТРАЦИИ ==========
void RegAndAuth::on_showRegisterButton_clicked()
{
    ui->groupLogin->setVisible(false);
    ui->groupRegister->setVisible(true);
    ui->showRegisterButton->setVisible(false);
    ui->showLoginButton->setVisible(true);
    ui->regLoginEdit->clear();
    ui->regPassEdit->clear();
    ui->regResultLabel->clear();
    qDebug() << "Switched to registration form";
}

// ========== ПЕРЕКЛЮЧЕНИЕ НА ФОРМУ ВХОДА ==========
void RegAndAuth::on_showLoginButton_clicked()
{
    ui->groupRegister->setVisible(false);
    ui->groupLogin->setVisible(true);
    ui->showLoginButton->setVisible(false);
    ui->showRegisterButton->setVisible(true);
    ui->logLoginEdit->clear();
    ui->logPassEdit->clear();
    ui->statsResultLabel->clear();
    qDebug() << "Switched to login form";
}

// ========== РЕГИСТРАЦИЯ ==========
void RegAndAuth::on_regButton_clicked()
{
    QString login = ui->regLoginEdit->text().trimmed();
    QString password = ui->regPassEdit->text().trimmed();

    if (login.isEmpty() || password.isEmpty()) {
        ui->regResultLabel->setText("❌ Заполните логин и пароль");
        return;
    }

    QString cmd = QString("reg&%1,%2").arg(login, password);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->regResultLabel->setText("⏳ Отправка запроса...");
}

// ========== АВТОРИЗАЦИЯ ==========
void RegAndAuth::on_logButton_clicked()
{
    QString login = ui->logLoginEdit->text().trimmed();
    QString password = ui->logPassEdit->text().trimmed();

    if (login.isEmpty() || password.isEmpty()) {
        ui->statsResultLabel->setText("❌ Заполните логин и пароль");
        return;
    }

    QString cmd = QString("auth&%1,%2").arg(login, password);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->statsResultLabel->setText("⏳ Отправка запроса...");
}

// ========== СТАТИСТИКА ==========
void RegAndAuth::on_statsButton_clicked()
{
    Client::getInstance()->sendMsgToServer("stats&");
    ui->statsResultLabel->setText("⏳ Запрос статистики...");
}

// ========== ВХОД АДМИНИСТРАТОРА ==========
void RegAndAuth::on_adminButton_clicked()
{
    bool ok;
    QString login = QInputDialog::getText(this, "Вход администратора",
                                          "Логин администратора:", QLineEdit::Normal, "", &ok);
    if (!ok || login.isEmpty()) return;

    QString password = QInputDialog::getText(this, "Вход администратора",
                                             "Пароль:", QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;

    QString cmd = QString("admin_auth&%1,%2").arg(login, password);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->statsResultLabel->setText("⏳ Проверка администратора...");
}

// ========== ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ДЛЯ ОТКРЫТИЯ ГЛАВНОГО ОКНА ==========
void RegAndAuth::openMainWindow()
{
    if (!m_mainWindow) {
        qDebug() << "Creating new MainWindow";
        m_mainWindow = new MainWindow();
        m_mainWindow->show();
        this->close();
    } else {
        qDebug() << "MainWindow already exists, bringing to front";
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
        this->close();
    }
}

// ========== ОБРАБОТКА ОТВЕТОВ ОТ СЕРВЕРА ==========
void RegAndAuth::onServerResponse(const QString &response)
{
    qDebug() << "RegAndAuth received:" << response;

    // Регистрация
    if (response.startsWith("REGISTER_OK")) {
        ui->regResultLabel->setText("✅ " + response);
        ui->regLoginEdit->clear();
        ui->regPassEdit->clear();
        on_showLoginButton_clicked();
        ui->statsResultLabel->setText("✅ Регистрация успешна! Теперь войдите.");
    }
    else if (response.startsWith("REGISTER_ERR")) {
        ui->regResultLabel->setText("❌ " + response);
    }

    // Авторизация (обычный пользователь)
    else if (response.startsWith("AUTH_OK")) {
        ui->statsResultLabel->setText("✅ " + response);
        openMainWindow();
    }
    else if (response.startsWith("AUTH_ERR")) {
        ui->statsResultLabel->setText("❌ " + response);
    }

    // Статистика
    else if (response.startsWith("STATS")) {
        ui->statsResultLabel->setText("📊 " + response);
    }
    else if (response.startsWith("STATS_ERR")) {
        ui->statsResultLabel->setText("❌ " + response);
    }

    // Администратор
    else if (response.startsWith("ADMIN_AUTH_OK")) {
        qDebug() << "=== ADMIN_AUTH_OK received ===";
        Client::getInstance()->setAdmin(true);
        qDebug() << "Admin status set to:" << Client::getInstance()->isAdmin();

        QMessageBox::information(this, "Администратор",
                                 "Вы успешно вошли как администратор");
        openMainWindow();
    }
    else if (response.startsWith("ADMIN_AUTH_ERR")) {
        QMessageBox::warning(this, "Ошибка",
                             "Неверный логин/пароль или у вас нет прав администратора");
    }

    // Административные ответы (список пользователей с HTML-таблицей)
    // Административные ответы (список пользователей с HTML-таблицей)
    else if (response.startsWith("ADMIN_USERS:")) {
        QString data = response.mid(12);
        QStringList users = data.split(", ");

        QString html = "<style>"
                       "table { border-collapse: collapse; width: 100%; font-family: Arial; }"
                       "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
                       "th { background-color: #4CAF50; color: white; }"
                       "tr:nth-child(even) { background-color: #f2f2f2; }"
                       "tr:hover { background-color: #ddd; }"
                       "</style>";

        html += "<tr>";
        html += "<tr><th>Логин</th><th>Статус</th></tr>";

        for (const QString &user : users) {
            QStringList parts = user.split(" | ");
            if (parts.size() >= 2) {
                QString login = parts[0];
                QString status = parts[1];

                QString color = "black";
                if (status == "admin") color = "#d32f2f";
                else if (status == "online") color = "#388e3c";
                else if (status == "offline") color = "#757575";
                else if (status == "banned") color = "#e64a19";

                html += QString("<tr>"
                                "<td style='font-weight: bold;'>%1</td>"
                                "<td style='color: %2; font-weight: bold;'>%3</td>"
                                "</tr>")
                            .arg(login.toHtmlEscaped())
                            .arg(color)
                            .arg(status);
            }
        }
        html += "</table>";

        QMessageBox msgBox;
        msgBox.setWindowTitle("📋 Список пользователей");
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText(html);
        msgBox.setStyleSheet("QLabel{ min-width: 400px; }");
        msgBox.exec();

        // Используем существующую метку
        ui->statsResultLabel->setText("✅ Список пользователей получен");
    }
}
