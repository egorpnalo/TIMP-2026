#ifndef REGANDAUTH_H
#define REGANDAUTH_H

#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>

// Forward declaration
class MainWindow;

namespace Ui {
class RegAndAuth;
}

class RegAndAuth : public QWidget
{
    Q_OBJECT

public:
    explicit RegAndAuth(QWidget *parent = nullptr);
    ~RegAndAuth();

private slots:
    void on_showRegisterButton_clicked();
    void on_showLoginButton_clicked();
    void on_regButton_clicked();
    void on_logButton_clicked();
    void on_statsButton_clicked();
    void on_adminButton_clicked();
    void onServerResponse(const QString &response);

private:
    void openMainWindow();  // ← добавить

private:
    Ui::RegAndAuth *ui;
    MainWindow *m_mainWindow;
};

#endif // REGANDAUTH_H
