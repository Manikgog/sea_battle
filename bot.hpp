#ifndef BOT_HPP
#define BOT_HPP

#include "utils.hpp"
#include "model.hpp"

class Bot {

  public:

    Bot() {
        _model = Model();
        int size = _model.getPlayingField().size();
        _shots = std::vector<Cell>(size, Cell());
    }

    int shoot() {
        if(horizontalSearch().has_value()) {
            return horizontalSearch().value();
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

    void setHit(int index) {
        _shots[index]._isShoted = true;
        _shots[index]._isOccupied = true;
    }

    std::optional<int> horizontalSearch() {
        for(int i = 0; i < _shots.size(); ++i) {
            if(_shots[i]._isShoted == true
                && _shots[i]._isOccupied == true) {
                int column = i%10;
                // перебор клеток вправо от обнаруженной клетки
                for(int j = column + 1; j < column + 5 && j < _model.getColumns(); ++j) {
                    int index = i/10 + j;
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
                    int index = i/10 + j;
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

    Model _model;               // класс с картой кораблей игрока, то есть по которому будет стрелять компьютер (бот)

  private:
    /**
     * @brief isAllIndexesTrue проверка на случай если все клетки уже обстреляны
     * @return
     */
    bool isAllIndexesTrue() {
        return std::all_of(_shots.begin(), _shots.end(), [](Cell b) { return b._isShoted; });
    }

    std::vector<Cell> _shots;   // вектор bool, если по точке уже производился удар, то значение равно true, не производился - false

};

#endif // BOT_HPP
