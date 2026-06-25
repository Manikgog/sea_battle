#ifndef BOT_HPP
#define BOT_HPP

#include <QDebug>
#include <set>
#include "../core/model.hpp"

class Bot {

public:

    Bot();

    void reset();
    int shoot();
    std::set<int> getMarkedCellsIndexes();

    /**
     * @brief setHit метод для отметки поражения палубы корабля
     * @param index индекс в векторе _shots
     */
    void setHit(int index, bool is_desroyed = false);
    std::optional<int> horizontalSearch();
    std::optional<int> verticalSearch();

    Model _model;               // класс с картой кораблей игрока, то есть по которому будет стрелять компьютер (бот)

private:
    /**
     * @brief isAllIndexesTrue проверка на случай если все клетки уже обстреляны
     * @return
     */
    bool isAllIndexesTrue();

    std::vector<Cell> _shots;   // вектор для учёта произведенных ударов
};

#endif // BOT_HPP
