#include <QDebug>
#include <iostream>
#include <QTimer>
#include <QJsonArray>
#include "model_adapter.hpp"

ModelAdapter::ModelAdapter(QObject *parent)
    : QObject(parent)
      , _gameWon(false)
      , _gameOver(false)
{
    // Расставляем корабли на поле игрока
    _playerField.automaticShipsPlacing();

    // Расставляем корабли на поле противника (для одиночной игры)
    _enemyField.automaticShipsPlacing();

    updateTurnStatus();
}

// === Методы для поля игрока (левое поле) ===

bool ModelAdapter::getPlayerCellIsOccupied(int index) {
    if (index < 0 || index >= _playerField.getPlayingField().size()) {
        return false;
    }
    return _playerField.getPlayingField()[index]._isOccupied;
}

bool ModelAdapter::getPlayerCellIsShoted(int index) {
    if (index < 0 || index >= _playerField.getPlayingField().size()) {
        return false;
    }
    return _playerField.getPlayingField()[index]._isShoted;
}

// === Методы для поля противника (правое поле) ===

bool ModelAdapter::getEnemyCellIsOccupied(int index) {
    if (index < 0 || index >= _enemyField.getPlayingField().size()) {
        return false;
    }
    return _enemyField.getPlayingField()[index]._isOccupied;
}

bool ModelAdapter::getEnemyCellIsShoted(int index) {
    if (index < 0 || index >= _enemyField.getPlayingField().size()) {
        return false;
    }
    return _enemyField.getPlayingField()[index]._isShoted;
}

void ModelAdapter::shot(int index) {
    if (_gameWon || _gameOver || _playerFieldBlocked) {
        return;
    }

    if (index < 0 || index >= static_cast<int>(_enemyField.getPlayingField().size())) {
        return;
    }

    auto& enemyField = const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());
    auto& cell = enemyField[index];

    if (!cell._isShoted) {
        qDebug() << "Player shoots at enemy cell:" << index;

        cell._isShoted = true;
        bool isMiss = true;
        if (cell._isOccupied) {
            isMiss = false;
            qDebug() << "HIT!";
        }

        checkWinCondition();
        emit gameStatusChanged();

        if (!_gameWon && isMiss) {
            _playerFieldBlocked = true;
            _turnStatus = "Ход бота...";
            emit turnStatusChanged();
            emit gameStatusChanged();

        } else if (!_gameWon && !isMiss) {
            _turnStatus = "Ваш ход (попадание!)";
            emit turnStatusChanged();
            emit gameStatusChanged();
        }
    }
}



void ModelAdapter::shipPlacing() {
    qDebug() << "=== Перерасстановка кораблей игрока ===";

    // Сбрасываем поле игрока
    _playerField.reset();

    // Расставляем корабли на поле игрока заново
    bool playerPlaced = _playerField.automaticShipsPlacing();

    qDebug() << "Корабли игрока расставлены:" << playerPlaced;
    qDebug() << "Количество кораблей:" << _playerField.getShips().size();

    // Сбрасываем состояние игры
    _gameWon = false;
    _gameOver = false;
    _playerFieldBlocked = false;
    _turnStatus = "Ваш ход";

    emit playerFieldUpdated();
    emit gameStatusChanged();
    emit turnStatusChanged();
}

void ModelAdapter::setEnemyField(const QJsonArray &fieldData) {
    // Для сетевой игры: получаем поле противника по сети
    // auto& enemyField = const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());

    // for (int i = 0; i < fieldData.size() && i < enemyField.size(); ++i) {
    //     //QJsonObject cellData = fieldData[i].toObject();
    //     //enemyField[i]._isOccupied = cellData["isOccupied"].toBool();
    //     // _isShoted не копируем - это состояние выстрелов игрока
    // }

    emit gameStatusChanged();
}

void ModelAdapter::newGame() {
    _playerField.reset();
    _playerField.automaticShipsPlacing();
    _enemyField.reset();
    _enemyField.automaticShipsPlacing();

    _gameWon = false;
    _gameOver = false;
    _playerFieldBlocked = false;
    _turnStatus = "Ваш ход";

    emit turnStatusChanged();
    emit gameStatusChanged();
    emit playerFieldUpdated();
}

QString ModelAdapter::getTurnStatus() {
    return _turnStatus;
}


QString ModelAdapter::getPlayerGameStatus() {
    int shipsDestroyed = 0;
    int totalShips = _playerField.getShips().size();

    for (const Ship& ship : _playerField.getShips()) {
        if (ship.isDestroyed()) {
            shipsDestroyed++;
        }
    }

    return QString("Корабли уничтожены: %1 / %2").arg(shipsDestroyed).arg(totalShips);
}

void ModelAdapter::checkWinCondition() {
    bool playerLost = _playerField.isAllShipsIsDestroyed();
    bool playerWon = _enemyField.isAllShipsIsDestroyed();

    qDebug() << "checkWinCondition: playerLost =" << playerLost << "playerWon =" << playerWon;

    if (playerLost && !_gameOver) {
        _gameOver = true;
        _playerFieldBlocked = false;
        _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";
        emit turnStatusChanged();
        emit gameStatusChanged();
        emit gameOver();
        return;
    }

    if (playerWon && !_gameWon) {
        _gameWon = true;
        _playerFieldBlocked = false;
        _turnStatus = "🏆 ПОБЕДА! 🏆";
        emit turnStatusChanged();
        emit gameWon();
        emit gameStatusChanged();
    }
}

void ModelAdapter::updateTurnStatus() {
    if (_gameWon) {
        _turnStatus = "🏆 ПОБЕДА! 🏆";
    } else if (_gameOver) {
        _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";
    } else if (_playerFieldBlocked) {
        _turnStatus = "Ход бота...";
    } else {
        _turnStatus = "Ваш ход";
    }
    emit turnStatusChanged();
}
