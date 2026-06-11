#include <QDebug>
#include <iostream>
#include <QTimer>

#include "model_adapter.hpp"



ModelAdapter::ModelAdapter(QObject *parent)
    : QObject(parent)
      , _gameWon(false) {
    _model.automaticShipsPlacing();
    _bot._model.automaticShipsPlacing();
}

bool ModelAdapter::getBotCellIsShoted(int index) {
    //qDebug() << __FUNCTION__ << "index =" << index;
    if (index < 0 || index >= _model.getPlayingField().size()) {
        return false;
    }
    return _model.getPlayingField()[index]._isShoted;
}

bool ModelAdapter::getBotCellIsOccupied(int index) {
    if (index < 0 || index >= _model.getPlayingField().size()) {
        return false;
    }
    //qDebug() << __FUNCTION__ << "index =" << index;
    return _model.getPlayingField()[index]._isOccupied;
}

bool ModelAdapter::getPlayerCellIsOccupied(int index)
{
    if (index < 0 || index >= _bot._model.getPlayingField().size()) {
        return false;
    }
    //qDebug() << __FUNCTION__ << "index =" << index;
    return _bot._model.getPlayingField()[index]._isOccupied;
}

bool ModelAdapter::getPlayerCellIsShoted(int index)
{
    if (index < 0 || index >= _bot._model.getPlayingField().size()) {
        return false;
    }
    //qDebug() << __FUNCTION__ << "index =" << index;
    return _bot._model.getPlayingField()[index]._isShoted;
}

void ModelAdapter::shot(int index) {
    if (_gameWon) {
        return;
    }

    if (index < 0 || index >= static_cast<int>(_model.getPlayingField().size())) {
        return;
    }

    auto& botField = const_cast<std::vector<Cell>&>(_model.getPlayingField());
    auto& cell = botField[index];

    if (!cell._isShoted) {
        qDebug() << "Player shoots at bot cell:" << index;

        cell._isShoted = true;
        bool is_miss = true;
        if(cell._isOccupied) {
            is_miss = false;
            // Проверка на уничтожение корабля у бота
            for (const Ship& ship : _model.getShips()) {
                if (ship.isDestroyed()) {
                    qDebug() << "Bot ship destroyed!";
                }
            }
        }

        checkWinCondition();
        emit gameStatusChanged();

        // Если игра не окончена, бот делает ответный ход
        if (!_gameWon && is_miss) {
            // Используем QTimer для небольшой задержки перед ходом бота
            QTimer::singleShot(500, this, &ModelAdapter::botMove);
        }
    }
}




void ModelAdapter::botMove() {
    if (_gameWon) {
        return;
    }

    // Бот выбирает случайную клетку для выстрела
    int index = _bot.shoot();
    qDebug() << "Bot shoots at player cell:" << index;

    auto& playerField = const_cast<std::vector<Cell>&>(_bot._model.getPlayingField());
    if (index >= 0 && index < static_cast<int>(playerField.size())) {
        auto& cell = playerField[index];
        if (!cell._isShoted) {
            cell._isShoted = true;

            if (cell._isOccupied) {
                _bot.setHit(index);
                qDebug() << "Bot HIT at player cell:" << index;
                // Если бот попал, он может сделать еще один ход
                emit gameStatusChanged();
                botMove();
            } else {
                qDebug() << "Bot MISS at player cell:" << index;
                emit gameStatusChanged();
            }
        }


    }
}




void ModelAdapter::newGame() {
    _model.reset();
    _model.automaticShipsPlacing();
    _bot._model.reset();
    _bot._model.automaticShipsPlacing();
    _gameWon = false;
    qDebug() << __FUNCTION__;
    std::cout << "Поле бота:" << std::endl;
    for(int i = 0; i < _model.getPlayingField().size(); ++i) {
        if(i%10 == 0) {
            std::cout << std::endl;
        }
        if(_model.getPlayingField()[i]._isOccupied) {
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

    std::cout << "Поле игрока:" << std::endl;
    for(int i = 0; i < _bot._model.getPlayingField().size(); ++i) {
        if(i%10 == 0) {
            std::cout << std::endl;
        }
        if(_bot._model.getPlayingField()[i]._isOccupied) {
            std::cout << "+ ";
        } else if(_bot._model.getPlayingField()[i]._isAllowed == false) {
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
