
#include <QDebug>
#include <iostream>
#include "model.hpp"
#include "utils.hpp"


Model::Model() {
    int amountShips = 4;    // количество кораблей
    for(int cells = 1; cells <= 4; cells++) {               // cell - количество палуб на корабле
        _amountShipsByCells.insert({cells, amountShips});
        amountShips--;
    }
    _playingField.resize(_rows * _columns);
    // инициализация поля
    for(int i = 0; i < _playingField.size(); ++i) {
        _playingField[i]._position = getPoint(i + 1);
    }
}

bool Model::isAllShipsIsDestroyed() const {
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
        int ship_number = 0;
        // if(cells == 2)
        //     qDebug() << "|||||||||||||";
        while(ship_number < amount_ships) {
            if(automaticPlacingShip(cells)) {
                ship_number++;

                // int count_cells = 0;
                // for(int i = 0; i < _playingField.size(); ++i) {
                //     if(i%10 == 0) {
                //         std::cout << std::endl;
                //     }
                //     if(_playingField[i]._isOccupied) {
                //         count_cells++;
                //         std::cout << "+ ";
                //     } else if(_playingField[i]._isAllowed == false) {
                //         std::cout << "* ";
                //     }
                //     else{
                //         std::cout << "- ";
                //     }

                // }
                // std::cout << std::endl;
                // std::cout << std::endl;
            }
        }
        amount_ships++;
    }
    return true;
}


void Model::reset() {
    for(int i = 0; i < 100; ++i) {
        _playingField[i] = Cell();
    }
    _ships.clear();
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
        qDebug() << __FUNCTION__ << "index " << index << " выходит за пределы поля";
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
    bool ok = true;
    if((column + 1) - cells >= 0) {
        // далее определяем не заняты ли уже эти позиции слева
        for(int i = column; i > column - cells; i--) {
            if(!isCellFree(row, i)) {
                ok = false;
                break;
            }
        }
        if(ok) {
            for(int i = column; i > column - cells; i--) {
                int index = row * 10 + i;
                if(index < 0 || index >= _playingField.size()) {
                    qDebug() << "index " << index << " выходит за пределы поля при попытке размещения влево";
                    return false;
                }
                _playingField[index]._isOccupied = true;
                _playingField[index]._isAllowed = false;
                _playingField[index]._position = getPoint(index + 1);
            }
            markNotAllowedPlacingPoint(column, row, Side::left, cells);
            addShip(row, column, Side::left, cells);
            return true;
        }
    }
    return false;
}


/**
 * @brief placingLeft метод для размещения корабля справа от указанной точки
 * @param row строка
 * @param column колонка
 * @param cells количество палуб у корабля
 */
bool Model::placingRight(int row, int column, int cells) {
    bool ok = true;
    // определяем не выходим ли за пределы строки
    if(column + cells - 1 < _columns) {
        // далее определяем не заняты ли уже эти позиции справа
        for(int i = column; i < column + cells; i++) {
            if(!isCellFree(row, i)) {
                ok = false;
                break;
            }
        }
        if(ok) {
            for(int i = column; i < column + cells; i++) {
                int index = row * 10 + i;
                if(index < 0 || index >= _playingField.size()) {
                    qDebug() << "index " << index << " выходит за пределы поля при попытке размещения вправо";
                    return false;
                }
                _playingField[index]._isOccupied = true;
                _playingField[index]._isAllowed = false;
                _playingField[index]._position = getPoint(index + 1);
            }
            markNotAllowedPlacingPoint(column, row, Side::right, cells);
            addShip(row, column, Side::right, cells);
            return true;
        }
    }
    return false;
}


/**
 * @brief placingLeft метод для размещения корабля вверх от указанной точки
 * @param row строка
 * @param column колонка
 * @param cells количество палуб у корабля
 */
bool Model::placingUp(int row, int column, int cells) {
    bool ok = true;
    // пробуем разместить корабль вверх от найденной позиции
    if(row - cells + 1 >= 0) {
        // далее определяем не заняты ли уже эти позиции вверху
        for(int i = row; i > row - cells; i--) {
            if(!isCellFree(i, column)) {
                ok = false;
                break;
            }
        }
        if(ok) {
            for(int i = row; i > row - cells; i--) {
                int index = i * _rows + column;
                if(index < 0 || index >= _playingField.size()) {
                    qDebug() << "index " << index << " выходит за пределы поля при попытке размещения вверх";
                    return false;
                }
                _playingField[index]._isOccupied    = true;
                _playingField[index]._isAllowed     = false;
                _playingField[index]._position      = getPoint(index + 1);
            }
            markNotAllowedPlacingPoint(column, row, Side::up, cells);
            addShip(row, column, Side::up, cells);
            return true;
        }
    }
    return false;
}



/**
 * @brief placingLeft метод для размещения корабля вверх от указанной точки
 * @param row строка
 * @param column колонка
 * @param cells количество палуб у корабля
 */
bool Model::placingDown(int row, int column, int cells) {
    bool ok = true;
    // пробуем разместить корабль вниз от найденной позиции
    if(row + cells - 1 < _rows) {
        // далее определяем не заняты ли уже эти позиции слева
        for(int i = row; i < row + cells; i++) {
            if(!isCellFree(i, column)) {
                ok = false;
                break;
            }
        }
        if(ok) {
            for(int i = row; i < row + cells; i++) {
                int index = i * _rows + column;
                if(index < 0 || index >= _playingField.size()) {
                    qDebug() << "index " << index << " выходит за пределы поля при попытке размещения вниз";
                    return false;
                }
                _playingField[index]._isOccupied = true;
                _playingField[index]._isAllowed = false;
                _playingField[index]._position = getPoint(index + 1);
            }
            markNotAllowedPlacingPoint(column, row, Side::down, cells);
            addShip(row, column, Side::down, cells);
            return true;
        }
    }
    return false;
}



const std::vector<Ship>& Model::getShips() const {
    return _ships;
}


int Model::getDestroyedShipsAmount() const {
    int destroyed_ships_counter = 0;
    for(const Ship& ship : _ships) {
        if(ship.isDestroyed()) {
            destroyed_ships_counter++;
        }
    }
    return destroyed_ships_counter;
}



/**
 * @brief placingLeft метод для пометки всех точек в радиусе одной клетки вокруг для запрещения размещения кораблей
 * @param index индекс точки в векторе _playingField
 */
void Model::markNotAllowedPlacingPoint(int column, int row, Side side, int cells) {
    if(side == Side::left) {        // если корабль построен от начальной точки с координатами column и row влево
        if(column + 1 < _columns) {
            int index = row * 10 + column + 1;          // отмечаем клетку правее самой правой палубы корабля, что там запрещено размещать корабли
            _playingField[index]._isAllowed = false;
        }
        int last_column = column - cells;
        if(last_column >= 0) {
            int index = row * 10 + last_column;
            _playingField[index]._isAllowed = false;
        }
        int upper_row = row - 1;    // следующая строка над кораблём

        if(upper_row >= 0) {
            if(column + 1 < _columns) {
                int index = upper_row * 10 + column + 1;
                _playingField[index]._isAllowed = false;
            }
            for(int col = column; col > column - cells; --col) {
                int index = upper_row * 10 + col;
                _playingField[index]._isAllowed = false;
            }
            int left_column = column - cells;
            if(left_column >= 0) {
                int index = upper_row * 10 + left_column;
                _playingField[index]._isAllowed = false;
                index = row * 10 + left_column;
                _playingField[index]._isAllowed = false;
            }
        }
        int lower_row = row + 1;
        if(lower_row < _rows) {
            if(column + 1 < _columns) {
                int index = lower_row * 10 + column + 1;          // отмечаем клетку правее самой правой палубы корабля и ниже на одну строку, что там запрещено размещать корабли
                _playingField[index]._isAllowed = false;
            }
            for(int col = column; col > column - cells; --col) {    // отмечаем клетки ниже на одну строку
                int index = lower_row * 10 + col;
                _playingField[index]._isAllowed = false;
            }
            int left_column = column - cells;                     // клетка левее самой левой палубы корабля
            if(left_column >= 0) {                                // отмечаем клетку левее самой левой палубы корабля и на одну строку ниже
                int index = lower_row * 10 + left_column;
                _playingField[index]._isAllowed = false;
            }
        }
    } else if(side == Side::right) {    // если корабль построен от начальной точки с координатами column и row вправо
        if(column - 1 >= 0) {
            int index = row * 10 + column - 1;
            _playingField[index]._isAllowed = false;
        }
        int last_column = column + cells;
        if(last_column < _columns) {
            int index = row * _rows + last_column;
            _playingField[index]._isAllowed = false;
        }
        int upper_row = row - 1;    // следующая строка над кораблём
        if(upper_row >= 0) {
            if(column - 1 >= 0) {
                int index = upper_row * 10 + column - 1;
                _playingField[index]._isAllowed = false;
            }
            for(int col = column; col < column + cells; ++col) {
                int index = upper_row * 10 + col;
                _playingField[index]._isAllowed = false;
            }
            int right_column = column + cells;
            if(right_column < _columns) {
                int index = row * 10 + right_column;
                _playingField[index]._isAllowed = false;
                index = upper_row * 10 + right_column;
                _playingField[index]._isAllowed = false;
            }
        }
        int lower_row = row + 1;
        if(lower_row < _rows) {
            int right_column = column + cells;
            if(right_column < _columns) {
                int index = lower_row * 10 + right_column;          // отмечаем клетку правее самой правой палубы корабля и ниже на одну строку, что там запрещено размещать корабли
                _playingField[index]._isAllowed = false;
            }
            for(int col = column; col < column + cells; ++col) {
                int index = lower_row * 10 + col;
                _playingField[index]._isAllowed = false;
            }
            int left_column = column - 1;                           // клетка левее самой левой палубы корабля
            if(left_column >= 0) {                                // отмечаем клетку левее самой левой палубы корабля и на одну строку ниже
                int index = lower_row * 10 + left_column;
                _playingField[index]._isAllowed = false;
            }
        }
    } else if(side == Side::down) {     // если корабль построен от начальной точки с координатами column и row вниз
        int upper_row = row - 1;
        if(upper_row >= 0) {
            int index = upper_row * 10 + column;
            _playingField[index]._isAllowed = false;
            if(column - 1 >= 0) {
                index = upper_row * 10 + column - 1;
                _playingField[index]._isAllowed = false;
            }
            if(column + 1 < _columns) {
                index = upper_row * 10 + column + 1;
                _playingField[index]._isAllowed = false;
            }
        }
        if(column - 1 >= 0) {
            int index = 0;
            for(int r = row; r < row + cells; ++r) {
                index = r * 10 + column - 1;
                _playingField[index]._isAllowed = false;
            }
        }
        if(column + 1 < _columns) {
            int index = 0;
            for(int r = row; r < row + cells; ++r) {
                index = r * 10 + column + 1;
                _playingField[index]._isAllowed = false;
            }
        }
        int lower_row = row + cells;
        if(lower_row < _rows) {
            int index = lower_row * 10 + column;
            _playingField[index]._isAllowed = false;
            if(column - 1 >= 0) {
                index = lower_row * 10 + column - 1;
                _playingField[index]._isAllowed = false;
            }
            if(column + 1 < _columns) {
                index = lower_row * 10 + column + 1;
                _playingField[index]._isAllowed = false;
            }
        }
    } else if(side == Side::up) {
        int upper_row = row - cells;
        if(upper_row >= 0) {
            int index = upper_row * _rows + column;
            _playingField[index]._isAllowed = false;
            if(column - 1 >= 0) {
                index = upper_row * _rows + column - 1;
                _playingField[index]._isAllowed = false;
            }
            if(column + 1 < _columns) {
                index = upper_row * _rows + column + 1;
                _playingField[index]._isAllowed = false;
            }
        }
        if(column - 1 >= 0) {
            int index = 0;
            for(int r = row; r > row - cells; --r) {
                index = r * _rows + column - 1;
                _playingField[index]._isAllowed = false;
            }
        }
        if(column + 1 < _columns) {
            int index = 0;
            for(int r = row; r > row - cells; --r) {
                index = r * _rows + column + 1;
                _playingField[index]._isAllowed = false;
            }
        }
        int lower_row = row + 1;
        if(lower_row < _rows) {
            int index = lower_row * _rows + column;
            _playingField[index]._isAllowed = false;
            if(column - 1 >= 0) {
                index = lower_row * _rows + column - 1;
                _playingField[index]._isAllowed = false;
            }
            if(column + 1 < _columns) {
                index = lower_row * _rows + column + 1;
                _playingField[index]._isAllowed = false;
            }
        }
    }

}




void Model::addShip(int row, int column, Side side, int cells) {
    int index = row * 10 + column;
    std::vector<Cell*> cells_;
    if(side == Side::left) {
        for(int i = index; i > index - cells; --i) {
            if(i < 0 || i >= _playingField.size()) {
                qDebug() << __FUNCTION__ << "индекс " << i << " вышел за границы массива.";
                return;
            }
            cells_.push_back(&_playingField[i]);
        }
    } else if(side == Side::right) {
        for(int i = index; i < index + cells; ++i) {
            if(i < 0 || i >= _playingField.size()) {
                qDebug() << __FUNCTION__ << "индекс " << i << " вышел за границы массива.";
                return;
            }
            cells_.push_back(&_playingField[i]);
        }
    } else if(side == Side::up) {
        int last_index = (row - cells) * 10 + column;
        for(int i = index; i > last_index; i-=_columns) {
            if(i < 0 || i >= _playingField.size()) {
                qDebug() << __FUNCTION__ << "индекс " << i << " вышел за границы массива.";
                return;
            }
            cells_.push_back(&_playingField[i]);
        }
    } else if(side == Side::down) {
        int last_index = (row + cells) * 10 + column;
        for(int i = index; i < last_index; i+=_columns) {
            if(i < 0 || i >= _playingField.size()) {
                qDebug() << __FUNCTION__ << "индекс " << i << " вышел за границы массива.";
                return;
            }
            cells_.push_back(&_playingField[i]);
        }
    }
    Ship ship(cells_);
    _ships.push_back(ship);
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
        while(!isCellFree(row, column)) {
            position = getRandomNumber(0, 99);
            row = position / 10;
            column = position % 10;
        }
        bool ok = true;
        std::vector<Side> sides{Side::left, Side::right, Side::up, Side::down};
        shuffleVector(sides);
        for(int i = 0; i < sides.size(); ++i) {
            if(sides[i] == Side::left) {
                if(placingLeft(row, column, cells)) {
                    return true;
                }
            } else if(sides[i] == Side::right) {
                if(placingRight(row, column, cells)) {
                    return true;
                }
            } else if(sides[i] == Side::up) {
                if(placingUp(row, column, cells)) {
                    return true;
                }
            } else if(sides[i] == Side::down) {
                if(placingDown(row, column, cells)) {
                    return true;
                }
            }
        }
    }
    return false;
}



const std::vector<Cell>& Model::getPlayingField() const {
    return _playingField;
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
    if (row >= 0 && row < y_arr.size()) {
        p._y = y_arr[row];
    } else {
        p._y = "?";
        qDebug() << "Error: row index out of range:" << row;
    }

    p._x = col + 1;  // колонка от 1 до 10

    // qDebug() << "Number:" << number
    //          << "-> Index:" << index
    //          << "-> Row:" << row
    //          << "-> Col:" << col
    //          << "-> Point:" << p._y << p._x;

    return p;
}
