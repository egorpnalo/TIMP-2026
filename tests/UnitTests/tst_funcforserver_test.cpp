#include <QtTest>
#include "../../server/Server/functions_to_server.h"
#include <QDebug>

class FuncForServer_Test : public QObject
{
    Q_OBJECT

private slots:
    void test_md5();
    void test_secant();
    void test_cycle();
};

void FuncForServer_Test::test_md5()
{
    qDebug() << "=== Тест MD5 ===";

    QString result = handleMD5("Hello");
    qDebug() << "MD5('Hello') =" << result;
    QVERIFY(result.startsWith("MD5:"));
}

void FuncForServer_Test::test_secant()
{
    qDebug() << "=== Тест метода секущих ===";

    QString result = handleSecant("16");
    qDebug() << "secant(16) =" << result;
    QVERIFY(result.contains("4.0"));
}

void FuncForServer_Test::test_cycle()
{
    qDebug() << "=== Тест проверки цикла ===";

    QString result = handleCycle("1,2,3,4,1");
    qDebug() << "cycle(1,2,3,4,1) =" << result;
    QVERIFY(result.startsWith("CYCLE_OK"));
}

QTEST_APPLESS_MAIN(FuncForServer_Test)
#include "tst_funcforserver_test.moc"
