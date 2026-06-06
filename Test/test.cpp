#include <QtTest>

#include "../model.hpp"

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();

private slots:
    void test_getPoint();
};

Test::Test() {


}

Test::~Test() {}

void Test::test_getPoint() {
    Model model;

    for(int i = 1; i <= 100; ++i) {
        Point p = model.getPoint(i);

        // Ожидаемая буква (по строкам)
        QString expectedLetter;
        if (i >= 1 && i <= 10) expectedLetter = "а";
        else if (i >= 11 && i <= 20) expectedLetter = "б";
        else if (i >= 21 && i <= 30) expectedLetter = "в";
        else if (i >= 31 && i <= 40) expectedLetter = "г";
        else if (i >= 41 && i <= 50) expectedLetter = "д";
        else if (i >= 51 && i <= 60) expectedLetter = "е";
        else if (i >= 61 && i <= 70) expectedLetter = "ж";
        else if (i >= 71 && i <= 80) expectedLetter = "з";
        else if (i >= 81 && i <= 90) expectedLetter = "и";
        else if (i >= 91 && i <= 100) expectedLetter = "к";

        // Ожидаемое число (колонка 1-10)
        int expectedNumber = ((i - 1) % 10) + 1;

        // Правильные сравнения согласно структуре Point
        QCOMPARE(p._y, expectedLetter);  // _y - QString (буква)
        QCOMPARE(p._x, expectedNumber);  // _x - int (число)

        // Отладочный вывод
        // qDebug() << "Number:" << i
        //          << "-> Expected:" << expectedLetter << expectedNumber
        //          << "-> Got:" << p._y << p._x;
    }
}

QTEST_APPLESS_MAIN(Test)

#include "test.moc"
