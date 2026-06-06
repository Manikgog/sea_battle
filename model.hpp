#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include "ship.hpp"


class Model {

public:
    Model();
    bool isAllShipsIsDestroyed();
    bool automaticShipsPlacing();

    Point getPoint(int number);
private:

    int getRandomNumber(int start_number, int end_number);
    bool automaticPlacingShip(int cells);

    int _rows = 10;
    int _columns = 10;
    std::vector<Cell> _playingField;
    std::vector<Ship> _ships;
    std::unordered_map<int, int> _amountShipsByCells;   // словарь где ключ - количество палуб, значение - количество кораблей;
};

#endif // MODEL_HPP
