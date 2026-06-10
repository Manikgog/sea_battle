#ifndef BOT_HPP
#define BOT_HPP

#include "utils.hpp"

class Bot {

  public:
    Bot(int first_index, int last_index) {
        for(int i = first_index; i <= last_index; ++i) {
            _shots.push_back(false);
        }
    }

    int shot(int first_index, int last_index) {
        int index = getRandomNumber(first_index, last_index);
        while(_shots[index] == true) {
            index = getRandomNumber(first_index, last_index);
        }
        _shots[index] = true;
        return index;
    }

  private:
    std::vector<bool> _shots;   // вектор bool, если по точке уже производился удар, то значение равно true, не производился - false
};

#endif // BOT_HPP
