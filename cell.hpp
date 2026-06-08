#ifndef CELL_HPP
#define CELL_HPP

#include <array>
#include <QString>

static inline const std::array<QString, 10> y_arr = {"а", "б", "в", "г", "д", "е", "ж", "з", "и", "к"};


struct Point {
    QString _y = y_arr[0];
    int     _x = 0;
};

struct Cell {
    Point   _position;
    bool    _isShoted   = false;
    bool    _isOccupied = false;
    bool    _isAllowed = true;
};

#endif // CELL_HPP
