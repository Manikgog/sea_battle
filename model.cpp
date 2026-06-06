#include <iostream>
#include <random>
#include <QDebug>
#include "model.hpp"


Model::Model() {
    int amountShips = 5;    // количество кораблей
    for(int cells = 1; cells <= 5; cells++) {               // cell - количество палуб на корабле
        _amountShipsByCells.insert({cells, amountShips});
        amountShips--;
    }
    _playingField.resize(_rows * _columns);
}

bool Model::isAllShipsIsDestroyed() {
    for(const Ship& ship : _ships) {
        if(!ship.isDestroyed())
            return false;
    }
    return true;
}

/**
 * @brief Model::automaticShipPlacing метод для выполнения автоматического размещения кораблей на поле
 * @return true в случае удачного размещения
 */
bool Model::automaticShipsPlacing() {
    std::vector<int> vec_cells;
    for(const auto p : _amountShipsByCells) {
        vec_cells.push_back(p.first);
    }
    std::sort(vec_cells.rbegin(), vec_cells.rend());

    int amount_ships = 1;
    for(int cells : vec_cells) {
        for(int ship_number = 0; ship_number < amount_ships; ship_number++) {
            if(!automaticPlacingShip(cells)) {
                return false;
            }

        }
        amount_ships++;
    }
    return true;
}

/**
 * @brief Model::isCellFree метод для определения занята ли указанная позиция
 * @param row
 * @param column
 * @return true если позиция занята
 */
bool Model::isCellFree(int row, int column) {
    int index = row * 10 + column;
    if(index < 0 || index >= _playingField.size()) {
        qDebug() << "index выходит за пределы поля";
        return false;
    }
    return !_playingField[index]._isOccupied && _playingField[index]._isAllowed;
}



/**
 * @brief placingLeft метод для размещения корабля слева от указанной точки
 * @param row строка
 * @param column колонка
 * @param cells количество палуб у корабля
 */
bool Model::placingLeft(int row, int column, int cells) {
    for(int i = column; i > column - cells; i--) {
        int index = row * 10 + i;
        if(index < 0 || index >= _playingField.size()) {
            qDebug() << "index выходит за пределы поля";
            return false;
        }
        _playingField[index]._isOccupied = true;
        _playingField[index]._isAllowed = false;
        _playingField[index]._position = getPoint(index + 1);
    }
    return true;
}


/**
 * @brief placingLeft метод для размещения корабля справа от указанной точки
 * @param row строка
 * @param column колонка
 * @param cells количество палуб у корабля
 */
bool Model::placingRight(int row, int column, int cells) {
    for(int i = column; i < column + cells; i++) {
        int index = row * 10 + i;
        if(index < 0 || index >= _playingField.size()) {
            qDebug() << "index выходит за пределы поля";
            return false;
        }
        _playingField[index]._isOccupied = true;
        _playingField[index]._isAllowed = false;
        _playingField[index]._position = getPoint(index + 1);
    }
    return true;
}


/**
 * @brief placingLeft метод для размещения корабля вверх от указанной точки
 * @param row строка
 * @param column колонка
 * @param cells количество палуб у корабля
 */
bool Model::placingUp(int row, int column, int cells) {
    for(int i = row; i > row - cells; i--) {
        int index = row * 10 + i;
        if(index < 0 || index >= _playingField.size()) {
            qDebug() << "index выходит за пределы поля";
            return false;
        }
        _playingField[index]._isOccupied = true;
        _playingField[index]._isAllowed = false;
        _playingField[index]._position = getPoint(index + 1);
    }
    return true;
}



/**
 * @brief placingLeft метод для размещения корабля вверх от указанной точки
 * @param row строка
 * @param column колонка
 * @param cells количество палуб у корабля
 */
bool Model::placingDown(int row, int column, int cells) {
    for(int i = row; i < row + cells; i++) {
        int index = row * 10 + i;
        if(index < 0 || index >= _playingField.size()) {
            qDebug() << "index выходит за пределы поля";
            return false;
        }
        _playingField[index]._isOccupied = true;
        _playingField[index]._isAllowed = false;
        _playingField[index]._position = getPoint(index + 1);
    }
    return true;
}



/**
 * @brief placingLeft метод для пометки всех точек в радиусе одной клетки вокруг для запрещения размещения кораблей
 * @param index индекс точки в векторе _playingField
 */
void Model::markNotAllowedPlacingPoint(int column, int row) {
    int index = row * 10 + column;
    if(column - 1 >= 0) {
        _playingField[index - 1]._isAllowed = false;
    }
    if(column + 1 < _columns) {
        _playingField[index + 1]._isAllowed = false;
    }
    if(row - 1 >= 0) {
        _playingField[row - 1]._isAllowed = false;
    }
    if(row + 1 < _rows) {
        _playingField[row + 1]._isAllowed = false;
    }
}




/**
 * @brief Model::automaticPlacingShip метод для автоматического размещения корабля
 * @param cells - количество палуб корабля
 * @return true в случае удачного размещения
 */
bool Model::automaticPlacingShip(int cells) {
    while(true) {
        int position = getRandomNumber(0, 99);
        int row = position / 10;
        int column = position % 10;
        bool ok = true;
        // сначала пробуем разместить корабль влево от найденной позиции
        if((column + 1) - cells < 0) {
            // далее определяем не заняты ли уже эти позиции слева
            for(int i = column; i > column - cells; i--) {
                if(!isCellFree(row, i)) {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                if(placingLeft(row, column, cells)) {
                    markNotAllowedPlacingPoint(column, row);
                    return true;
                }
                return false;
            }
        }
        // пробуем разместить корабль вправо от найденной позиции
        if(column + cells - 1 >= _columns) {
            // далее определяем не заняты ли уже эти позиции слева
            for(int i = column; i < column + cells; i++) {
                if(!isCellFree(row, i)) {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                if(placingRight(row, column, cells)) {
                    markNotAllowedPlacingPoint(column, row);
                    return true;
                }
                return false;
            }
        }
        // пробуем разместить корабль вверх от найденной позиции
        if(row + cells - 1 >= _rows) {
            // далее определяем не заняты ли уже эти позиции слева
            for(int i = row; i > row - cells; i--) {
                if(!isCellFree(i, column)) {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                if(placingUp(row, column, cells)) {
                    markNotAllowedPlacingPoint(column, row);
                    return true;
                }
                return false;
            }
        }
        // пробуем разместить корабль вниз от найденной позиции
        if(row + cells - 1 >= _rows) {
            // далее определяем не заняты ли уже эти позиции слева
            for(int i = row; i < row + cells; i++) {
                if(!isCellFree(i, column)) {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                if(placingDown(row, column, cells)) {
                    markNotAllowedPlacingPoint(column, row);
                    return true;
                }
                return false;
            }
        }
    }
    return false;
}



int Model::getRandomNumber(int start_number, int end_number) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(start_number, end_number);
    return dis(gen);
}




Point Model::getPoint(int number) {
    if (number < 1 || number > 100) {
        qDebug() << "Error: number out of range (1-100):" << number;
        Point p;
        p._y = "ошибка";
        p._x = -1;
        return p;
    }

    Point p;
    // Исправленная индексация: number от 1 до 100
    int index = number - 1;  // Переводим в 0-99
    int col = index % _columns;      // строка 0-9
    int row = index / _rows;      // колонка 0-9

    // Проверяем границы массива x_arr
    if (row >= 0 && row < x_arr.size()) {
        p._y = x_arr[row];
    } else {
        p._y = "?";
        qDebug() << "Error: row index out of range:" << row;
    }

    p._x = col + 1;  // колонка от 1 до 10

    qDebug() << "Number:" << number
             << "-> Index:" << index
             << "-> Row:" << row
             << "-> Col:" << col
             << "-> Point:" << p._y << p._x;

    return p;
}
