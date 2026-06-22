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
    Cell& cell = enemyField[index];

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



/**
 * @brief ModelAdapter::setShot - метод для отметки нанесения удара от противника и возвращения результата удара
 * @param index
 * @return
 */
std::optional<ShotResult> ModelAdapter::setShot(const QString &index)
{
    bool ok = true;
    int index_int = index.toInt(&ok);
    if(!ok) {
        qDebug() << __FUNCTION__ << index << " не число.";
        return {};
    }
    if(index_int < 0 || index_int >= _playerField.getPlayingField().size()) {
        qDebug() << __FUNCTION__ << index_int << " лежит за пределами игрового поля.";
        return {};
    }
    auto& field = const_cast<std::vector<Cell>&>(_playerField.getPlayingField());
    Cell& cell = field[index_int];

    if (!cell._isShoted) {
        qDebug() << __FUNCTION__  << "Player shoots at enemy cell:" << index;
        int destroyed_ships_before = _playerField.getDestroyedShipsAmount();
        cell._isShoted = true;
        bool isMiss = true;
        if (cell._isOccupied) {
            isMiss = false;
            qDebug() << __FUNCTION__ << "HIT!";
        }

        checkWinCondition();
        emit gameStatusChanged();

        if (!_gameWon && isMiss) {
            _playerFieldBlocked = true;
            _turnStatus = "Ваш ход (промах!)";
            emit turnStatusChanged();
            emit gameStatusChanged();
            return {ShotResult{Result::Miss, index_int}};
        } else if (!_gameWon && !isMiss) {
            _turnStatus = "Ход противника...";
            emit turnStatusChanged();
            emit gameStatusChanged();
            int destroyed_ships_after = _playerField.getDestroyedShipsAmount();
            if(destroyed_ships_before < destroyed_ships_after) {
                return {ShotResult{Result::Destroyed, index_int}};
            }
            return {ShotResult{Result::Wounded, index_int}};
        }
    }
    return {ShotResult{Result::Miss, index_int}};
}




/**
 * @brief ModelAdapter::setResult метод для фиксации результата стрельбы по противнику
 * @param result - статус попадания (мимо, убит, ранен)
 * @param index - индекс ячейки
 */
void ModelAdapter::setResult(const QString &result, const QString &index) {
    bool ok = false;
    int res = result.toUInt(&ok);
    if(!ok) {
        return;
    }
    int index_int = index.toInt(&ok);
    if(!ok) {
        return;
    }
    auto& field = const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());
    Cell& cell = field[index_int];
    if(res == Result::Miss) {
        _turnStatus = "Ход противника (промах)";
        cell._isShoted = true;
    } else if(res == Result::Wounded) {
        _turnStatus = "Ваш ход (ранен)";
        cell._isShoted = true;
        cell._isOccupied = true;
    } else if(res == Result::Destroyed) {
        _turnStatus = "Ваш ход (убит)";
        cell._isShoted = true;
        cell._isOccupied = true;
    }
    emit playerFieldUpdated();
    emit gameStatusChanged();
    emit turnStatusChanged();
    qDebug() << __FUNCTION__;
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

QString ModelAdapter::getEnemyGameStatus() {
    int shipsDestroyed = 0;
    int totalShips = _enemyField.getShips().size();

    for (const Ship& ship : _enemyField.getShips()) {
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
