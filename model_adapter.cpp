#include <QDebug>
#include <iostream>
#include <QTimer>
#include <QJsonArray>
#include "model_adapter.hpp"

ModelAdapter::ModelAdapter(QObject *parent)
    : QObject(parent)
      , _gameWon(false)
      , _gameOver(false)
      , _playerFieldBlocked(true)
      , _turnStatus("Ожидание подключения...")
      , _gameStarted(false)
      , _isMyTurn(false)
{
    _playerField.automaticShipsPlacing();
    _enemyField.automaticShipsPlacing();
    updateTurnStatus();
}

bool ModelAdapter::getPlayerCellIsOccupied(int index) {
    if (index < 0 || index >= static_cast<int>(_playerField.getPlayingField().size())) {
        return false;
    }
    return _playerField.getPlayingField()[index]._isOccupied;
}

bool ModelAdapter::getPlayerCellIsShoted(int index) {
    if (index < 0 || index >= static_cast<int>(_playerField.getPlayingField().size())) {
        return false;
    }
    return _playerField.getPlayingField()[index]._isShoted;
}

bool ModelAdapter::getEnemyCellIsOccupied(int index) {
    if (index < 0 || index >= static_cast<int>(_enemyField.getPlayingField().size())) {
        return false;
    }
    return _enemyField.getPlayingField()[index]._isOccupied;
}

bool ModelAdapter::getEnemyCellIsShoted(int index) {
    if (index < 0 || index >= static_cast<int>(_enemyField.getPlayingField().size())) {
        return false;
    }
    return _enemyField.getPlayingField()[index]._isShoted;
}

std::optional<ShotResult> ModelAdapter::setShot(const QString &indexStr)
{
    bool ok;
    int index = indexStr.toInt(&ok);
    if (!ok) {
        qDebug() << __FUNCTION__ << indexStr << " не число.";
        return {};
    }

    if (index < 0 || index >= static_cast<int>(_playerField.getPlayingField().size())) {
        qDebug() << __FUNCTION__ << index << " лежит за пределами игрового поля.";
        return {};
    }

    auto& field = const_cast<std::vector<Cell>&>(_playerField.getPlayingField());
    Cell& cell = field[index];

    if (cell._isShoted) {
        return {ShotResult{Result::Miss, index}};
    }

    qDebug() << __FUNCTION__ << "Противник стреляет в клетку:" << index;

    int destroyedShipsBefore = _playerField.getDestroyedShipsAmount();
    cell._isShoted = true;

    ShotResult result;
    result._index = index;

    if (cell._isOccupied) {
        qDebug() << __FUNCTION__ << "Попадание!";
        result._result = Result::Wounded;

        // Проверяем, уничтожен ли корабль
        int destroyedShipsAfter = _playerField.getDestroyedShipsAmount();
        if (destroyedShipsAfter > destroyedShipsBefore) {
            result._result = Result::Destroyed;
        }
        // при попадании ход остаётся у противника
        _isMyTurn = false;
        _playerFieldBlocked = true;
    } else {
        qDebug() << __FUNCTION__ << "Промах!";
        result._result = Result::Miss;
        // при промахе противника по моему полю ход переходит ко мне
        _isMyTurn = true;
        _playerFieldBlocked = false;
    }

    checkWinCondition();
    emit gameStatusChanged();
    updateTurnStatus();

    return result;
}

void ModelAdapter::setResult(const QString &resultStr, const QString &indexStr)
{
    bool ok;
    int res = resultStr.toInt(&ok);
    if (!ok) {
        qDebug() << __FUNCTION__ << "Неверный результат:" << resultStr;
        return;
    }

    int index = indexStr.toInt(&ok);
    if (!ok) {
        qDebug() << __FUNCTION__ << "Неверный индекс:" << indexStr;
        return;
    }

    auto& field = const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());
    if (index < 0 || index >= static_cast<int>(field.size())) {
        return;
    }

    Cell& cell = field[index];
    cell._isShoted = true;

    if (res == Result::Miss) {
        // при нашем промахе по противнику - ход переходит к противнику
        _isMyTurn = false;
        _playerFieldBlocked = true;
        qDebug() << "Промах по противнику";
    } else if (res == Result::Wounded) {
        // при попадании по противнику - ход остаётся у нас
        _isMyTurn = true;
        _playerFieldBlocked = false;
        cell._isOccupied = true;
        qDebug() << "Попадание по противнику (ранен)";
    } else if (res == Result::Destroyed) {
        // при уничтожении корабля противника - ход остаётся у нас
        _isMyTurn = true;
        _playerFieldBlocked = false;
        cell._isOccupied = true;
        qDebug() << "Попадание по противнику (убит)";
    }

    checkWinCondition();
    emit gameStatusChanged();
    emit playerFieldUpdated();
    updateTurnStatus();
}








void ModelAdapter::shot(int index) {
    qDebug() << "shot() called: index=" << index
             << "_gameWon=" << _gameWon
             << "_gameOver=" << _gameOver
             << "_gameStarted=" << _gameStarted
             << "_isMyTurn=" << _isMyTurn
             << "_playerFieldBlocked=" << _playerFieldBlocked;

    if (_gameWon || _gameOver) {
        qDebug() << "Игра уже закончена";
        return;
    }

    if (!_gameStarted) {
        qDebug() << "Игра еще не началась";
        return;
    }

    if (!_isMyTurn) {
        qDebug() << "Сейчас не ваш ход!";
        return;
    }

    if (_playerFieldBlocked) {
        qDebug() << "Поле заблокировано";
        return;
    }

    if (index < 0 || index >= static_cast<int>(_enemyField.getPlayingField().size())) {
        return;
    }

    auto& field = const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());
    Cell& cell = field[index];

    if (cell._isShoted) {
        qDebug() << "Клетка уже обстреляна:" << index;
        return;
    }

    qDebug() << "Игрок стреляет в клетку:" << index;

    // Блокируем поле до получения результата
    _playerFieldBlocked = true;
    _isMyTurn = false;
    emit gameStatusChanged();
    updateTurnStatus();

    // Отправляем выстрел через бэкенд
    emit shotRequested(index);
}










void ModelAdapter::shipPlacing() {
    qDebug() << "=== Перерасстановка кораблей игрока ===";

    _playerField.reset();
    _playerField.automaticShipsPlacing();

    _gameWon = false;
    _gameOver = false;
    _playerFieldBlocked = true;
    _gameStarted = false;
    _turnStatus = "Ожидание начала игры...";

    emit playerFieldUpdated();
    emit gameStatusChanged();
    emit turnStatusChanged();
    emit updateGameButtonState();
}







void ModelAdapter::newGame() {
    _playerField.reset();
    _playerField.automaticShipsPlacing();
    _enemyField.reset();
    _enemyField.automaticShipsPlacing();

    _gameWon = false;
    _gameOver = false;
    _playerFieldBlocked = true;
    _gameStarted = false;
    _turnStatus = "Ожидание начала игры...";

    emit turnStatusChanged();
    emit gameStatusChanged();
    emit playerFieldUpdated();
}










void ModelAdapter::startGame() {
    _gameStarted = true;
    _isMyTurn = true;
    _playerFieldBlocked = false;
    _turnStatus = "Ваш ход!";

    qDebug() << "=== startGame() ===";
    qDebug() << "_gameStarted =" << _gameStarted;
    qDebug() << "_isMyTurn =" << _isMyTurn;
    qDebug() << "_playerFieldBlocked =" << _playerFieldBlocked;

    emit turnStatusChanged();
    emit gameStatusChanged();
    emit playerFieldUpdated();
    emit updateGameButtonState();
}








QString ModelAdapter::getTurnStatus() {
    if (_gameWon) {
        return "🏆 ПОБЕДА! 🏆";
    } else if (_gameOver) {
        return "💀 ВЫ ПРОИГРАЛИ! 💀";
    } else if (!_gameStarted) {
        return "Ожидание начала игры...";
    } else if (_isMyTurn) {
        return "Ваш ход!";
    } else {
        return "Ожидание хода противника...";
    }
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
        _playerFieldBlocked = true;
        _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";
        emit turnStatusChanged();
        emit gameStatusChanged();
        emit gameOver();
        return;
    }

    if (playerWon && !_gameWon) {
        _gameWon = true;
        _playerFieldBlocked = true;
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
    } else if (!_gameStarted) {
        _turnStatus = "Ожидание начала игры...";
    } else if (_playerFieldBlocked) {
        _turnStatus = "Ожидание хода противника...";
    } else {
        _turnStatus = "Ваш ход!";
    }

    emit turnStatusChanged();
}
