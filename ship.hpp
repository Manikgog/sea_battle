#ifndef SHIP_HPP
#define SHIP_HPP

#include <vector>
#include "cell.hpp"

struct Ship {

    Ship(std::vector<Cell*> cells)
        : _cells(cells) {
    }

    bool isDestroyed() const {
        for(const Cell* cell : _cells) {
            if(!cell->_isShoted) {
                return false;
            }
        }
        return true;
    }

    const std::vector<Cell*> getCells() {
        return _cells;
    }

    int getCellsAmount() const {
        return _cells.size();
    }

private:
    const std::vector<Cell*> _cells;
};

#endif // SHIP_HPP
