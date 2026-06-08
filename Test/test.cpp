#include <QtTest>
#include <QDebug>

#include "../model.hpp"

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();

private slots:
    void test_getPoint();
    void test_getRandomNumber();
    void test_placingLeft();
    void test_placingRight();
    void test_placingUp();
    void test_placingDown();
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


/**
* @brief Test::test_getRandomNumber метод для проверки, что метод getRandomNumber генерирует все числа от начального до конечного включительно
*/
void Test::test_getRandomNumber() {
    Model model;

    std::vector<int> results;
    for(int i = 1; i <= 5; ++i) {
        results.push_back(i);
    }

    while(!results.empty()) {
        int number = model.getRandomNumber(1, 5);
        auto it = std::find(results.begin(), results.end(), number);
        if(it != results.end()) {
            results.erase(it);
        }
    }
    QCOMPARE(results.empty(), true);
}


/**
 * @brief Test::test_placingLeft тестирование метода placingLeft
 */
void Test::test_placingLeft() {
    Model model;
    int row = 1;
    int column = 4;
    int cells = 5;
    bool result_placing = model.placingLeft(row, column, cells);
    QCOMPARE(result_placing, true);
    std::vector<Cell> playingField = model.getPlayingField();
    for(int i = 0; i < 6; ++i) {
        QCOMPARE(playingField[i]._isAllowed, false);
    }
    for(int i = 10; i < 15; ++i) {
        QCOMPARE(playingField[i]._isOccupied, true);
    }
    QCOMPARE(playingField[15]._isAllowed, false);
    for(int i = 20; i < 26; ++i) {
        QCOMPARE(playingField[i]._isAllowed, false);
    }

    result_placing = model.placingLeft(row, column + 5, cells);
    QCOMPARE(result_placing, false);

    std::vector<Ship> ships = model.getShips();
    QCOMPARE(ships.size(), 1);

    QCOMPARE(ships[0].getCells().size(), 5);

    playingField = model.getPlayingField();
    int count_cells = 0;                            // общее количество палуб на поле
    for(int i = 0; i < playingField.size(); ++i) {
        if(playingField[i]._isOccupied) {
            count_cells++;
        }
    }
    QCOMPARE(count_cells, 5);
}



/**
 * @brief Test::test_placingRight тестирование метода placingRight
 */
void Test::test_placingRight()
{
    Model model;
    int row = 1;
    int column = 5;
    int cells = 5;
    bool result_placing = model.placingRight(row, column, cells);
    QCOMPARE(result_placing, true);
    std::vector<Cell> playingField = model.getPlayingField();
    for(int i = 4; i <= 9; ++i) {
        QCOMPARE(playingField[i]._isAllowed, false);
    }
    for(int i = 15; i <= 19; ++i) {
        QCOMPARE(playingField[i]._isOccupied, true);
    }
    QCOMPARE(playingField[14]._isAllowed, false);
    for(int i = 24; i <= 29; ++i) {
        QCOMPARE(playingField[i]._isAllowed, false);
    }

    row = 1;
    column = 0;
    result_placing = model.placingRight(row, column, cells);
    QCOMPARE(result_placing, false);

    row = 2;
    column = 0;
    result_placing = model.placingRight(row, column, cells);
    QCOMPARE(result_placing, false);

    std::vector<Ship> ships = model.getShips();
    QCOMPARE(ships.size(), 1);

    QCOMPARE(ships[0].getCells().size(), 5);

    playingField = model.getPlayingField();
    int count_cells = 0;                            // общее количество палуб на поле
    for(int i = 0; i < playingField.size(); ++i) {
        if(playingField[i]._isOccupied) {
            count_cells++;
        }
    }
    QCOMPARE(count_cells, 5);
}




void Test::test_placingUp()
{
    Model model;
    int row = 5;
    int column = 1;
    int cells = 5;
    bool result_placing = model.placingUp(row, column, cells);
    QCOMPARE(result_placing, true);

    std::vector<Cell> playingField = model.getPlayingField();

    int first_index = (row - cells + 1) * 10 + column;
    int last_index = (row + 1) * 10 + column;
    for(int i = first_index; i < last_index; i+=10) {
        QCOMPARE(playingField[i]._isOccupied, true);
    }

    row = 4;
    column = 2;
    result_placing = model.placingUp(row, column, cells);
    QCOMPARE(result_placing, false);

    row = 9;
    column = 1;
    result_placing = model.placingUp(row, column, cells);
    QCOMPARE(result_placing, false);

    column = 0;
    result_placing = model.placingUp(row, column, cells);
    QCOMPARE(result_placing, false);

    std::vector<Ship> ships = model.getShips();
    QCOMPARE(ships.size(), 1);

    QCOMPARE(ships[0].getCells().size(), 5);

    int count_cells = 0;                            // общее количество палуб на поле
    for(int i = 0; i < playingField.size(); ++i) {
        if(playingField[i]._isOccupied) {
            count_cells++;
        }
    }
    QCOMPARE(count_cells, 5);

    int count_not_allowed = 0;                      // общее количество клеток где нельзя размещать палубу корабля
    for(int i = 0; i < playingField.size(); ++i) {
        if(playingField[i]._isAllowed == false) {
            count_not_allowed++;
        }
    }
    qDebug() << "count_not_allowed =" << count_not_allowed;
    QCOMPARE(count_not_allowed, 16);
}



void Test::test_placingDown()
{
    Model model;
    int row = 5;
    int column = 1;
    int cells = 5;
    bool result_placing = model.placingDown(row, column, cells);
    QCOMPARE(result_placing, true);

    std::vector<Cell> playingField = model.getPlayingField();

    for(int i = 0; i < playingField.size(); ++i) {
        if(playingField[i]._isOccupied) {
            qDebug() << "index =" << i;
            qDebug() << playingField[i]._position._y << playingField[i]._position._x;
        }
    }

    row = 0;
    result_placing = model.placingDown(row, column, cells);
    QCOMPARE(result_placing, false);

    row = 1;
    result_placing = model.placingDown(row, column, cells);
    QCOMPARE(result_placing, false);

    row = 6;
    column = 3;
    result_placing = model.placingDown(row, column, cells);
    QCOMPARE(result_placing, false);

    std::vector<Ship> ships = model.getShips();
    QCOMPARE(ships.size(), 1);

    QCOMPARE(ships[0].getCells().size(), 5);

    int count_cells = 0;                            // общее количество палуб на поле
    for(int i = 0; i < playingField.size(); ++i) {
        if(playingField[i]._isOccupied) {
            count_cells++;
        }
    }
    QCOMPARE(count_cells, 5);
}



QTEST_APPLESS_MAIN(Test)

#include "test.moc"
