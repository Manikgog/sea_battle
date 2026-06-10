#ifndef BOT_HPP
#define BOT_HPP

#include "utils.hpp"
#include "model.hpp"

class Bot {

  public:

    Bot() {
        Model model;
        int size = model.getPlayingField().size();
        _shots = std::vector<bool>(size, false);
    }

    int shoot() {
        int index = getRandomNumber(0, _shots.size());
        while(_shots[index] == true) {
            index = getRandomNumber(0, _shots.size());
        }
        _shots[index] = true;
        return index;
    }

    Model _model;               // класс с картой кораблей игрока, то есть по которому будет стрелять компьютер (бот)

  private:

    std::vector<bool> _shots;   // вектор bool, если по точке уже производился удар, то значение равно true, не производился - false

};

#endif // BOT_HPP
