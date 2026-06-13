#include "bot.hpp"
#include "utils.hpp"


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
    auto search_result = horizontalSearch();
    if(search_result.has_value()) {
        return search_result.value();
    }
    search_result = verticalSearch();
    if(search_result.has_value()) {
        return search_result.value();
    }


    int index = getRandomNumber(0, _shots.size()-1);
    while(_shots[index]._isShoted == true) {
        index = getRandomNumber(0, _shots.size()-1);
        if(isAllIndexesTrue()) {
            return -1;
        }
    }
    _shots[index]._isShoted = true;
    return index;
}


/**
 * @brief Bot::getMarkedCellsNumber - метод для получения количества отмеченных клеточек, по которым уже не нужно стрелять
 * @return
 */
std::set<int> Bot::getMarkedCellsIndexes() {
    std::set<int> indexes;
    for(int i = 0; i < _shots.size(); ++i) {
        if(_shots[i]._isShoted) {
            indexes.insert(i);
        }
    }
    return indexes;
}

/**
     * @brief setHit метод для отметки поражения палубы корабля
     * @param index индекс в векторе _shots
     */
void Bot::setHit(int index) {
    _shots[index]._isShoted = true;
    _shots[index]._isOccupied = true;
    if(index%10 != 0) { // если не крайняя левая клетка
        int upper_diagonal_left_index = index - 11;
        if(upper_diagonal_left_index >= 0) {
            _shots[upper_diagonal_left_index]._isShoted = true;
        }

        int lower_diagonal_left_index = index + 9;
        if(lower_diagonal_left_index < _shots.size()) {
            _shots[lower_diagonal_left_index]._isShoted = true;
        }
    }

    if(index%10 != 9) { // если не крайняя правая клетка
        int upper_diagonal_right_index = index - 9;
        if(upper_diagonal_right_index >= 0) {
            _shots[upper_diagonal_right_index]._isShoted = true;
        }

        int lower_diagonal_right_index = index + 11;
        if(lower_diagonal_right_index < _shots.size()) {
            _shots[lower_diagonal_right_index]._isShoted = true;
        }
    }
}

std::optional<int> Bot::horizontalSearch() {
    for(int i = 0; i < _shots.size(); ++i) {
        if(_shots[i]._isShoted == true
            && _shots[i]._isOccupied == true) {
            int column = i%10;
            // перебор клеток вправо от обнаруженной клетки
            for(int j = column + 1; j < column + 5 && j < _model.getColumns(); ++j) {
                int index = (i/10) * 10 + j;
                if(index >= _shots.size()) {
                    return {};
                }
                if(_shots[index]._isShoted == false) {
                    _shots[index]._isShoted = true;
                    return {index};
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == true) {
                    continue;
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == false) {
                    break;
                }
            }
            // перебор клеток влево от обнаруженной ячейки
            for(int j = column - 1; j >= 0; --j) {
                int index = (i/10) * 10 + j;
                if(index >= _shots.size() || index < 0) {
                    return {};
                }
                if(_shots[index]._isShoted == false) {
                    _shots[index]._isShoted = true;
                    return {index};
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == true) {
                    continue;
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == false) {
                    break;
                }
            }
        }
    }

    return {};
}


std::optional<int> Bot::verticalSearch() {
    for(int i = 0; i < _shots.size(); ++i) {
        if(_shots[i]._isShoted == true
            && _shots[i]._isOccupied == true) {
            int row = i/10;
            int column = i%10;
            // перебор клеток вниз от обнаруженной клетки
            for(int j = row + 1; j < row + 4 && j < _model.getRows(); ++j) {
                int index = j * 10 + column;
                if(index >= _shots.size()) {
                    return {};
                }
                if(_shots[index]._isShoted == false) {
                    _shots[index]._isShoted = true;
                    return {index};
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == true) {
                    continue;
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == false) {
                    break;
                }
            }
            for(int j = row - 1; j >= 0; --j) {
                int index = j * 10 + column;
                if(index >= _shots.size() || index < 0) {
                    return {};
                }
                if(_shots[index]._isShoted == false) {
                    _shots[index]._isShoted = true;
                    return {index};
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == true) {
                    continue;
                }
                if(_shots[index]._isShoted == true
                    && _shots[index]._isOccupied == false) {
                    break;
                }
            }
        }

    }
    return {};
}


/**
 * @brief isAllIndexesTrue проверка на случай если все клетки уже обстреляны
 * @return
 */
bool Bot::isAllIndexesTrue() {
    return std::all_of(_shots.begin(), _shots.end(), [](Cell b) { return b._isShoted; });
}
