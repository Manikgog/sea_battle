#include "bot.hpp"
#include "../core/utils.hpp"

Bot::Bot() {
    _model = Model();
    int size = _model.getPlayingField().size();
    _shots = std::vector<Cell>(size, Cell());
}

void Bot::reset() {
    _model = Model();
    int size = _model.getPlayingField().size();
    _shots = std::vector<Cell>(size, Cell());
}

int Bot::shoot() {
    // Определяем порядок поиска случайным образом
    std::vector<std::function<std::optional<int>()>> search_methods = {
        [this]() { return horizontalSearch(); },
        [this]() { return verticalSearch(); }
    };

    if (getRandomNumber(0, 1) == 1) {
        std::swap(search_methods[0], search_methods[1]);
    }

    for (auto& method : search_methods) {
        auto result = method();
        if (result.has_value()) {
            return result.value();
        }
    }

    // Случайный выстрел, если поиск не дал результатов
    return randomShot();
}

int Bot::randomShot() {
    if (isAllIndexesTrue()) {
        return -1;
    }

    int index = getRandomNumber(0, _shots.size() - 1);
    while (_shots[index]._isShoted) {
        index = getRandomNumber(0, _shots.size() - 1);
    }
    _shots[index]._isShoted = true;
    return index;
}

std::optional<int> Bot::searchInDirection(int startIndex, int step, int maxSteps, bool isHorizontal) {
    int row = startIndex / 10;
    int col = startIndex % 10;

    auto isValidInx = [&](int index) {
        if (!isValidIndex(index)) {
            return false;
        }
        if (isHorizontal) {
            return (index / 10) == row; // Не выходим за пределы строки
        }
        return true;
    };

    // Проверяем обе стороны от найденной клетки
    std::vector<int> directions = {step, -step};

    for (int dir : directions) {
        for (int i = 1; i < maxSteps; ++i) {
            int currentIndex = startIndex + i * dir;
            if (!isValidInx(currentIndex)) {
                break;
            }

            if (_shots[currentIndex]._isShoted) {
                if (_shots[currentIndex]._isOccupied) {
                    continue; // Продолжаем искать дальше
                } else {
                    break; // Наткнулись на пустую клетку
                }
            } else {
                // Нашли необстрелянную клетку
                _shots[currentIndex]._isShoted = true;
                return currentIndex;
            }
        }
    }

    return {};
}

std::optional<int> Bot::horizontalSearch() {
    for (int i = 0; i < _shots.size(); ++i) {
        if (_shots[i]._isShoted && _shots[i]._isOccupied) {
            auto result = searchInDirection(i, 1, 5, true);
            if (result.has_value()) {
                return result;
            }
        }
    }
    return {};
}

std::optional<int> Bot::verticalSearch() {
    for (int i = 0; i < _shots.size(); ++i) {
        if (_shots[i]._isShoted && _shots[i]._isOccupied) {
            auto result = searchInDirection(i, 10, 4, false);
            if (result.has_value()) {
                return result;
            }
        }
    }
    return {};
}

void Bot::setHit(int index, bool isDestroyed) {
    markShot(index);
    markDiagonalCells(index);

    if (isDestroyed) {
        markSurroundingCells(index);
    }
}

void Bot::markShot(int index) {
    _shots[index]._isShoted = true;
    _shots[index]._isOccupied = true;
}

void Bot::markDiagonalCells(int index) {
    int row = index / 10;
    int col = index % 10;

    std::vector<std::pair<int, int>> diagonals = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    for (auto [dr, dc] : diagonals) {
        int newRow = row + dr;
        int newCol = col + dc;
        if (isWithinBounds(newRow, newCol)) {
            int newIndex = newRow * 10 + newCol;
            _shots[newIndex]._isShoted = true;
        }
    }
}

void Bot::markSurroundingCells(int index) {
    // Определяем ориентацию корабля
    bool isHorizontal = false;
    bool isVertical = false;

    // Проверяем соседние клетки по горизонтали
    if (index % 10 > 0 && _shots[index - 1]._isOccupied) {
        isHorizontal = true;
    }
    if (index % 10 < 9 && _shots[index + 1]._isOccupied) {
        isHorizontal = true;
    }

    // Проверяем соседние клетки по вертикали
    if (index >= 10 && _shots[index - 10]._isOccupied) {
        isVertical = true;
    }
    if (index < 90 && _shots[index + 10]._isOccupied) {
        isVertical = true;
    }

    if (isHorizontal) {
        markHorizontalShip(index);
    } else if (isVertical) {
        markVerticalShip(index);
    } else {
        // Одиночный корабль (1 палуба)
        markHorizontalNeighbors(index);
        markVerticalNeighbors(index);
    }
}

void Bot::markHorizontalShip(int index) {
    int row = index / 10;

    // Находим начало корабля
    int firstCol = index % 10;
    while (firstCol > 0 && _shots[row * 10 + firstCol - 1]._isOccupied) {
        firstCol--;
    }

    // Находим конец корабля
    int lastCol = index % 10;
    while (lastCol < 9 && _shots[row * 10 + lastCol + 1]._isOccupied) {
        lastCol++;
    }

    // Отмечаем клетки сверху и снизу от каждой палубы
    for (int col = firstCol; col <= lastCol; ++col) {
        int currentIndex = row * 10 + col;
        markVerticalNeighbors(currentIndex);
    }
}

void Bot::markVerticalShip(int index) {
    int col = index % 10;

    // Находим начало корабля
    int firstRow = index / 10;
    while (firstRow > 0 && _shots[(firstRow - 1) * 10 + col]._isOccupied) {
        firstRow--;
    }

    // Находим конец корабля
    int lastRow = index / 10;
    while (lastRow < 9 && _shots[(lastRow + 1) * 10 + col]._isOccupied) {
        lastRow++;
    }

    // Отмечаем клетки слева и справа от каждой палубы
    for (int row = firstRow; row <= lastRow; ++row) {
        int currentIndex = row * 10 + col;
        markHorizontalNeighbors(currentIndex);
    }
}

void Bot::markHorizontalNeighbors(int index) {
    int row = index / 10;
    int col = index % 10;

    // Отмечаем клетку слева
    if (col > 0) {
        int leftIndex = row * 10 + (col - 1);
        _shots[leftIndex]._isShoted = true;
    }

    // Отмечаем клетку справа
    if (col < 9) {
        int rightIndex = row * 10 + (col + 1);
        _shots[rightIndex]._isShoted = true;
    }
}

void Bot::markVerticalNeighbors(int index) {
    int row = index / 10;
    int col = index % 10;

    // Отмечаем клетку сверху
    if (row > 0) {
        int upIndex = (row - 1) * 10 + col;
        _shots[upIndex]._isShoted = true;
    }

    // Отмечаем клетку снизу
    if (row < 9) {
        int downIndex = (row + 1) * 10 + col;
        _shots[downIndex]._isShoted = true;
    }
}

bool Bot::isValidIndex(int index) const {
    return index >= 0 && index < _shots.size();
}

bool Bot::isWithinBounds(int row, int col) const {
    return row >= 0 && row < 10 && col >= 0 && col < 10;
}

bool Bot::isAllIndexesTrue() const {
    return std::all_of(_shots.begin(), _shots.end(),
                       [](const Cell& cell) { return cell._isShoted; });
}

std::set<int> Bot::getMarkedCellsIndexes() const {
    std::set<int> indexes;
    for (int i = 0; i < _shots.size(); ++i) {
        if (_shots[i]._isShoted) {
            indexes.insert(i);
        }
    }
    return indexes;
}
