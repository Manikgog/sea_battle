#ifndef BOT_HPP
#define BOT_HPP

#include <QDebug>
#include <set>
#include <optional>
#include <functional>
#include "../core/model.hpp"

class Bot {
public:
    Bot();
    void reset();
    int shoot();
    void setHit(int index, bool isDestroyed = false);
    std::set<int> getMarkedCellsIndexes() const;

    Model _model;

private:
    // Основные методы
    int randomShot();
    std::optional<int> horizontalSearch();
    std::optional<int> verticalSearch();
    std::optional<int> searchInDirection(int startIndex, int step, int maxSteps, bool isHorizontal);

    // Методы для обработки попаданий
    void markShot(int index);
    void markDiagonalCells(int index);
    void markSurroundingCells(int index);
    void markHorizontalShip(int index);
    void markVerticalShip(int index);
    void markHorizontalNeighbors(int index);
    void markVerticalNeighbors(int index);

    // Вспомогательные методы
    bool isAllIndexesTrue() const;
    bool isValidIndex(int index) const;
    bool isWithinBounds(int row, int col) const;

    std::vector<Cell> _shots;
};

#endif // BOT_HPP
