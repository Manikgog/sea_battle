#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include "ship.hpp"

enum Side {
    left
    , right
    , up
    , down
};


class Test;


class Model {

  public:
    Model();
    bool isAllShipsIsDestroyed() const;
    bool automaticShipsPlacing();
    void reset();

    Point getPoint(int number);
    const std::vector<Cell>& getPlayingField() const;

    const std::vector<Ship>& getShips() const;
    int getDestroyedShipsAmount() const;
    int getRows() {
        return _rows;
    }
    int getColumns() {
        return _columns;
    }

  private:
    friend class Test;
    bool automaticPlacingShip(int cells);
    bool isCellFree(int row, int column);
    bool placingLeft(int row, int column, int cells);
    bool placingRight(int row, int column, int cells);
    bool placingUp(int row, int column, int cells);
    bool placingDown(int row, int column, int cells);
    void markNotAllowedPlacingPoint(int column, int row, Side side, int cells);
    void addShip(int row, int column, Side side, int cells);

    int _rows = 10;
    int _columns = 10;
    std::vector<Cell> _playingField;
    std::vector<Ship> _ships;
    std::unordered_map<int, int> _amountShipsByCells;   // словарь где ключ - количество палуб, значение - количество кораблей;
};

#endif // MODEL_HPP
