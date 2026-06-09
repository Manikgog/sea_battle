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
    void test_playingFieldInit();
    void test_automaticShipsPlacing();

  private:
    int calculateAllowedCellsAnglePlacing(int cells);       // подсчёт количества запрещенных к размещению других кораблей ячеек при размещении корабля в углу
    int calculateAllowedCellsAlongSidePlacing(int cells);   // подсчёт количества запрещенных к размещению других кораблей ячеек при размещении корабля вдоль стенки
    int calculateAllowedCellsNoseToSidePlacing(int cells);  // подсчёт количества запрещенных к размещению других кораблей ячеек при размещении корабля торцом к стенке
    int calculateAllowedCellFreePlacing(int cells);         // подсчёт количества запрещенных к размещению других кораблей ячеек при размещении корабля не касаясь стенок

    void test_anglePlacing(int row, int column, int cells, Side side);     // тест для проверки правильности размещения в углу при размещении влево от точки
    void test_alongSidePlacing(int row, int column, int cells, Side side);     // тест для проверки правильности размещения если корабль находится вдоль стенки
    void test_noseToSidePlacing(int row, int column, int cells, Side side);    // тест для проверки правильности размещения если корабль находится торцом к стенке
    void test_freePlacing(int row, int column, int cells, Side side);          // тест для проверки правильности размещения если корабль не касается стенок

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

    for(int i = 1; i <= 4; ++i) {
        test_anglePlacing(0, 9, i, Side::left);
        test_anglePlacing(9, 9, i, Side::left);
        test_alongSidePlacing(0, 8, i, Side::left);
        test_alongSidePlacing(9, 8, i, Side::left);
        test_noseToSidePlacing(1, 9, i, Side::left);
        test_noseToSidePlacing(3, 9, i, Side::left);
        test_freePlacing(1, 8, i, Side::left);
        test_freePlacing(2, 8, i, Side::left);
    }

}



/**
 * @brief Test::test_placingRight тестирование метода placingRight
 */
void Test::test_placingRight()
{

    for(int i = 1; i <= 4; ++i) {
        test_anglePlacing(0, 0, i, Side::right);
        test_anglePlacing(9, 0, i, Side::right);
        test_alongSidePlacing(0, 1, i, Side::right);
        test_alongSidePlacing(9, 2, i, Side::right);
        test_noseToSidePlacing(1, 0, i, Side::right);
        test_noseToSidePlacing(3, 0, i, Side::right);
        test_freePlacing(1, 1, i, Side::right);
        test_freePlacing(2, 2, i, Side::right);
    }

}




void Test::test_placingUp()
{
    for(int i = 1; i <= 4; ++i) {
        test_anglePlacing(9, 0, i, Side::up);
        test_anglePlacing(9, 9, i, Side::up);
        test_alongSidePlacing(8, 0, i, Side::up);
        test_alongSidePlacing(8, 9, i, Side::up);
        test_noseToSidePlacing(9, 2, i, Side::up);
        test_noseToSidePlacing(9, 4, i, Side::up);
        test_freePlacing(8, 1, i, Side::up);
        test_freePlacing(7, 3, i, Side::up);
    }

}



void Test::test_placingDown()
{
    for(int i = 1; i <= 4; ++i) {
        test_anglePlacing(0, 0, i, Side::down);
        test_anglePlacing(0, 9, i, Side::down);
        test_alongSidePlacing(1, 0, i, Side::down);
        test_alongSidePlacing(1, 9, i, Side::down);
        test_noseToSidePlacing(0, 2, i, Side::down);
        test_noseToSidePlacing(0, 4, i, Side::down);
        test_freePlacing(1, 1, i, Side::down);
        test_freePlacing(2, 3, i, Side::down);
    }
}


/**
 * @brief Test::test_playingFieldInit проверка инициализации ячеек игрового поля
 */
void Test::test_playingFieldInit()
{
    Model model;
    const std::array<QString, 10> y_arr = {"а", "б", "в", "г", "д", "е", "ж", "з", "и", "к"};
    QString y = y_arr[0];
    int x = 1;
    Point p_expected = Point(1, y_arr[0]);
    Point p_actual = model.getPoint(1);
    for(int i = 1; i <= 40; ++i) {
        if(i >= 1 && i <= 10) {
            x = i;
            y = y_arr[0];
        } else if(i >= 11 && i <= 20) {
            x = i-10;
            y = y_arr[1];
        } else if(i >= 21 && i <= 30) {
            x = i-20;
            y = y_arr[2];
        } else if(i >= 31 && i <= 40) {
            x = i-30;
            y = y_arr[3];
        } else if(i >= 41 && i <= 50) {
            x = i-40;
            y = y_arr[4];
        } else if(i >= 51 && i <= 60) {
            x = i-50;
            y = y_arr[5];
        } else if(i >= 61 && i <= 70) {
            x = i-60;
            y = y_arr[6];
        } else if(i >= 71 && i <= 80) {
            x = i-70;
            y = y_arr[7];
        } else if(i >= 81 && i <= 90) {
            x = i-80;
            y = y_arr[8];
        } else if(i >= 91 && i <= 100) {
            x = i-70;
            y = y_arr[9];
        }

        p_expected = Point(x, y);
        //qDebug() << "p_expected =" << p_expected._y << p_expected._x ;
        p_actual = model.getPoint(i);
        //qDebug() << "p_actual =" << p_expected._y << p_expected._x ;
        QCOMPARE(p_actual == p_expected, true);
    }

}




/**
 * @brief Test::test_automaticShipsPlacing проверка автоматической расстановки кораблей
 */
void Test::test_automaticShipsPlacing()
{
    // Model model;
    // bool placing_result = model.automaticShipsPlacing();

    // QCOMPARE(placing_result, true);

    // std::vector<Ship> ships = model.getShips();
    // int _5_cells_ships_count = 0;
    // int _4_cells_ships_count = 0;
    // int _3_cells_ships_count = 0;
    // int _2_cells_ships_count = 0;
    // int _1_cells_ships_count = 0;
    // for(const Ship& s : ships) {
    //     if(s.getCellsAmount() == 5) {
    //         _5_cells_ships_count++;
    //     } else if(s.getCellsAmount() == 4) {
    //         _4_cells_ships_count++;
    //     } else if(s.getCellsAmount() == 3) {
    //         _3_cells_ships_count++;
    //     } else if(s.getCellsAmount() == 2) {
    //         _2_cells_ships_count++;
    //     } else if(s.getCellsAmount() == 1) {
    //         _1_cells_ships_count++;
    //     }
    // }

    // QCOMPARE(_4_cells_ships_count, 1);
    // QCOMPARE(_3_cells_ships_count, 2);
    // QCOMPARE(_2_cells_ships_count, 3);
    // QCOMPARE(_1_cells_ships_count, 4);
}



int Test::calculateAllowedCellsAnglePlacing(int cells)
{
    return cells * 2 + 2;
}

int Test::calculateAllowedCellsAlongSidePlacing(int cells)
{
    return cells * 2 + 4;
}

int Test::calculateAllowedCellsNoseToSidePlacing(int cells)
{
    return cells * 3 + 3;
}

int Test::calculateAllowedCellFreePlacing(int cells)
{
    return cells * 3 + 6;
}




void Test::test_anglePlacing(int row, int column, int cells, Side side)
{
    int number_of_is_not_allowed_cells = calculateAllowedCellsAnglePlacing(cells);
    Model model;
    bool placing_result = false;
    if(side == Side::left) {
        placing_result = model.placingLeft(row, column, cells);
    } else if(side == Side::right) {
        placing_result = model.placingRight(row, column, cells);
    } else if(side == Side::up) {
        placing_result = model.placingUp(row, column, cells);
    } else if(side == Side::down) {
        placing_result = model.placingDown(row, column, cells);
    }

    QCOMPARE(placing_result, true);

    int is_not_allowed_counter = 0;
    for(const Cell& cell : model.getPlayingField()) {
        if(cell._isAllowed == false) {
            //qDebug() << cell._position._y << cell._position._x;
            is_not_allowed_counter++;
        }
    }
    //qDebug() << "cells amount =" << cells << "; is_not_allowed_counter =" << is_not_allowed_counter << "number_of_is_not_allowed_cells" << number_of_is_not_allowed_cells;
    QCOMPARE(is_not_allowed_counter, number_of_is_not_allowed_cells);
}



void Test::test_alongSidePlacing(int row, int column, int cells, Side side)
{
    int number_of_is_not_allowed_cells = calculateAllowedCellsAlongSidePlacing(cells);
    Model model;
    bool placing_result = false;
    if(side == Side::left) {
        placing_result = model.placingLeft(row, column, cells);
    } else if(side == Side::right) {
        placing_result = model.placingRight(row, column, cells);
    } else if(side == Side::up) {
        placing_result = model.placingUp(row, column, cells);
    } else if(side == Side::down) {
        placing_result = model.placingDown(row, column, cells);
    }

    QCOMPARE(placing_result, true);

    int is_not_allowed_counter = 0;
    for(const Cell& cell : model.getPlayingField()) {
        if(cell._isAllowed == false) {
            //qDebug() << cell._position._y << cell._position._x;
            is_not_allowed_counter++;
        }
    }
    //qDebug() << "cells amount =" << cells << "; is_not_allowed_counter =" << is_not_allowed_counter << "number_of_is_not_allowed_cells" << number_of_is_not_allowed_cells;
    QCOMPARE(is_not_allowed_counter, number_of_is_not_allowed_cells);
}




void Test::test_noseToSidePlacing(int row, int column, int cells, Side side)
{
    int number_of_is_not_allowed_cells = calculateAllowedCellsNoseToSidePlacing(cells);
    Model model;
    bool placing_result = false;
    if(side == Side::left) {
        placing_result = model.placingLeft(row, column, cells);
    } else if(side == Side::right) {
        placing_result = model.placingRight(row, column, cells);
    } else if(side == Side::up) {
        placing_result = model.placingUp(row, column, cells);
    } else if(side == Side::down) {
        placing_result = model.placingDown(row, column, cells);
    }

    QCOMPARE(placing_result, true);

    int is_not_allowed_counter = 0;
    for(const Cell& cell : model.getPlayingField()) {
        if(cell._isAllowed == false) {
            //qDebug() << cell._position._y << cell._position._x;
            is_not_allowed_counter++;
        }
    }
    //qDebug() << "cells amount =" << cells << "; is_not_allowed_counter =" << is_not_allowed_counter << "number_of_is_not_allowed_cells" << number_of_is_not_allowed_cells;
    QCOMPARE(is_not_allowed_counter, number_of_is_not_allowed_cells);
}




void Test::test_freePlacing(int row, int column, int cells, Side side)
{
    int number_of_is_not_allowed_cells = calculateAllowedCellFreePlacing(cells);
    Model model;
    bool placing_result = false;
    if(side == Side::left) {
        placing_result = model.placingLeft(row, column, cells);
    } else if(side == Side::right) {
        placing_result = model.placingRight(row, column, cells);
    } else if(side == Side::up) {
        placing_result = model.placingUp(row, column, cells);
    } else if(side == Side::down) {
        placing_result = model.placingDown(row, column, cells);
    }

    QCOMPARE(placing_result, true);

    int is_not_allowed_counter = 0;
    for(const Cell& cell : model.getPlayingField()) {
        if(cell._isAllowed == false) {
            //qDebug() << cell._position._y << cell._position._x;
            is_not_allowed_counter++;
        }
    }
    //qDebug() << "cells amount =" << cells << "; is_not_allowed_counter =" << is_not_allowed_counter << "number_of_is_not_allowed_cells" << number_of_is_not_allowed_cells;
    QCOMPARE(is_not_allowed_counter, number_of_is_not_allowed_cells);
}







QTEST_APPLESS_MAIN(Test)

#include "test.moc"
