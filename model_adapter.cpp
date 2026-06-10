#include <QDebug>
#include <iostream>

#include "model_adapter.hpp"


ModelAdapter::ModelAdapter(QObject *parent)
    : QObject(parent)
    , _gameWon(false) {
    _model.automaticShipsPlacing();
}

bool ModelAdapter::getCellIsShoted(int index) {
    //qDebug() << __FUNCTION__;
    if (index < 0 || index >= _model.getPlayingField().size()) {
        return false;
    }
    return _model.getPlayingField()[index]._isShoted;
}

bool ModelAdapter::getCellIsOccupied(int index) {
    if (index < 0 || index >= _model.getPlayingField().size()) {
        return false;
    }
    return _model.getPlayingField()[index]._isOccupied;
}

void ModelAdapter::shot(int index) {
    if (_gameWon) {
        return;
    }

    if (index < 0 || index >= _model.getPlayingField().size()) {
        return;
    }

    auto& field = const_cast<std::vector<Cell>&>(_model.getPlayingField());
    auto& cell = field[index];
    if (!cell._isShoted) {
        //qDebug() << __FUNCTION__ << index;
        cell._isShoted = true;

        // Проверка на уничтожение корабля
        for (const Ship& ship : _model.getShips()) {
            if (ship.isDestroyed()) {
                qDebug() << "Корабль уничтожен!";
            }
        }

        checkWinCondition();
        emit gameStatusChanged();
    }
}

void ModelAdapter::newGame() {
    _model = Model();
    _model.automaticShipsPlacing();
    //qDebug() << __FUNCTION__ << _model.getShips().size();
    _gameWon = false;

    int count_cells = 0;
    for(int i = 0; i < _model.getPlayingField().size(); ++i) {
        if(i%10 == 0) {
            std::cout << std::endl;
        }
        if(_model.getPlayingField()[i]._isOccupied) {
            count_cells++;
            std::cout << "+ ";
        } else if(_model.getPlayingField()[i]._isAllowed == false) {
            std::cout << "  ";
        }
        else{
            std::cout << "  ";
        }

    }
    std::cout << std::endl;
    std::cout << std::endl;
    emit gameStatusChanged();
}

QString ModelAdapter::getGameStatus() {
    if (_gameWon) {
        return "ПОБЕДА!";
    }

    int shipsDestroyed = 0;
    int totalShips = _model.getShips().size();

    for (const Ship& ship : _model.getShips()) {
        if (ship.isDestroyed()) {
            shipsDestroyed++;
        }
    }

    return QString("Корабли уничтожены: %1 / %2").arg(shipsDestroyed).arg(totalShips);
}

void ModelAdapter::checkWinCondition() {
    if (_model.isAllShipsIsDestroyed() && !_gameWon) {
        _gameWon = true;
        emit gameWon();
        emit gameStatusChanged();
    }
}
