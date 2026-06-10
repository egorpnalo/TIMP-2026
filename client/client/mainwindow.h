#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

// ==================== ЛОКАЛЬНЫЙ RSA ДЛЯ ДЕМОНСТРАЦИИ ====================
class LocalRSA {
private:
    long long n, e, d;

    long long modPow(long long base, long long exp, long long mod) {
        long long result = 1;
        base = base % mod;
        while (exp > 0) {
            if (exp & 1) result = (result * base) % mod;
            base = (base * base) % mod;
            exp >>= 1;
        }
        return result;
    }

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
        long long range = max - min + 1;
        long long num;
        do {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            num = min + (seed % range);
        } while (!isPrime(num));
        return num;
    }

public:
    LocalRSA() : n(0), e(0), d(0) {}

    void generateKeys() {
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

    void setPublicKey(long long n, long long e) {
        this->n = n;
        this->e = e;
    }

    void setPrivateKey(long long n, long long d) {
        this->n = n;
        this->d = d;
    }

    long long getN() const { return n; }
    long long getE() const { return e; }
    long long getD() const { return d; }

    bool hasPublicKey() const { return n > 0 && e > 0; }
    bool hasPrivateKey() const { return n > 0 && d > 0; }

    long long encrypt(long long message) {
        return modPow(message, e, n);
    }

    long long decrypt(long long cipher) {
        return modPow(cipher, d, n);
    }

    QString encryptString(const QString &plain) {
        if (!hasPublicKey()) return "ERROR: No public key";

        QStringList encrypted;
        for (QChar ch : plain) {
            long long m = ch.unicode();
            long long c = encrypt(m);
            encrypted << QString::number(c);
        }
        return encrypted.join(",");
    }

    QString decryptString(const QString &cipher) {
        if (!hasPrivateKey()) return "ERROR: No private key";

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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateAdminUI(bool isAdmin);

private slots:
    void onServerResponse(const QString &response);

    // Слоты для существующих кнопок
    void on_md5Button_clicked();
    void on_secantButton_clicked();
    void on_cycleButton_clicked();
    void on_adminButton_clicked();
    void on_backButton_clicked();

    // Слоты для работы с графом
    void on_refreshGraphButton_clicked();
    void on_addEdgeConfirmButton_clicked();
    void on_removeEdgeConfirmButton_clicked();
    void on_setWeightButton_clicked();
    void on_dijkstraButton_clicked();
    void on_geneticButton_clicked();
    void on_applyGeneticParamsButton_clicked();

    // Слоты для RSA демонстрации
    void on_generateKeysButton_clicked();
    void on_encryptButton_clicked();
    void on_decryptButton_clicked();

private:
    Ui::MainWindow *ui;
    LocalRSA localRSA;
};

#endif // MAINWINDOW_H
