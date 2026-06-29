#include <QDebug>
#include <QTimer>
#include <QJsonArray>
#include "model_adapter_network.hpp"

ModelAdapterNetwork::ModelAdapterNetwork(QObject *parent)
    : QObject(parent)
      , _gameWon(false)
      , _gameOver(false)
      , _playerFieldBlocked(true)
      , _turnStatus("Ожидание подключения...")
      , _gameStarted(false)
      , _isMyTurn(false)
{
    _playerField.automaticShipsPlacing();
    updateTurnStatus();
}

bool ModelAdapterNetwork::getPlayerCellIsOccupied(int index) {
    if (index < 0 || index >= static_cast<int>(_playerField.getPlayingField().size())) {
        return false;
    }
    return _playerField.getPlayingField()[index]._isOccupied;
}

bool ModelAdapterNetwork::getPlayerCellIsShoted(int index) {
    if (index < 0 || index >= static_cast<int>(_playerField.getPlayingField().size())) {
        return false;
    }
    return _playerField.getPlayingField()[index]._isShoted;
}

bool ModelAdapterNetwork::getEnemyCellIsOccupied(int index) {
    if (index < 0 || index >= static_cast<int>(_enemyField.getPlayingField().size())) {
        return false;
    }
    if(_enemyField.getPlayingField()[index]._isOccupied) {
        qDebug() << __FUNCTION__ << index << _enemyField.getPlayingField()[index]._isOccupied;
    }
    return _enemyField.getPlayingField()[index]._isOccupied;
}

bool ModelAdapterNetwork::getEnemyCellIsShoted(int index) {
    if (index < 0 || index >= static_cast<int>(_enemyField.getPlayingField().size())) {
        return false;
    }
    if(_enemyField.getPlayingField()[index]._isShoted) {
        qDebug() << __FUNCTION__ << index << _enemyField.getPlayingField()[index]._isShoted;
    }
    return _enemyField.getPlayingField()[index]._isShoted;
}

std::optional<ShotResult> ModelAdapterNetwork::setShot(const QString &indexStr)
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

    int destroyedShipsBefore = _playerField.getDestroyedShipsAmount();
    cell._isShoted = true;

    ShotResult result;
    result._index = index;

    if (cell._isOccupied) {
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








void ModelAdapterNetwork::setResult(const QString &resultStr, const QString &indexStr)
{
    bool ok;
    int res = resultStr.toInt(&ok);
    if (!ok) {
        qDebug() << __FUNCTION__ << "Результат не является числом:" << resultStr;
        return;
    }

    int index = indexStr.toInt(&ok);
    if (!ok) {
        qDebug() << __FUNCTION__ << "Индекс не является числом:" << indexStr;
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
    } else if (res == Result::Wounded) {
        // при попадании по противнику - ход остаётся у нас
        _isMyTurn = true;
        _playerFieldBlocked = false;
        cell._isOccupied = true;
    } else if (res == Result::Destroyed) {
        // при уничтожении корабля противника - ход остаётся у нас
        _isMyTurn = true;
        _playerFieldBlocked = false;
        cell._isOccupied = true;
    }

    checkWinCondition();
    emit gameStatusChanged();
    //emit playerFieldUpdated();
    updateTurnStatus();
}





Q_INVOKABLE void ModelAdapterNetwork::setPlayerFieldBlocked(bool blocked) {
    _playerFieldBlocked = blocked;
    updateTurnStatus();
    emit gameStatusChanged();
}





Q_INVOKABLE void ModelAdapterNetwork::gameWon() {
    _gameWon = true;
    _gameOver = true;
    _playerFieldBlocked = true;
    _gameStarted = false;
    _turnStatus = "🏆 ПОБЕДА! 🏆";

    _enemyField.reset();
    _enemyField.automaticShipsPlacing();
    _playerField.reset();
    _playerField.automaticShipsPlacing();
    _enemyShipsDestroyed = 0;

    emit gameWonSignal();
    emit turnStatusChanged();
    emit gameStatusChanged();
    emit updateGameButtonState();
}






Q_INVOKABLE void ModelAdapterNetwork::gameOver() {
    _gameWon = false;
    _gameOver = true;
    _playerFieldBlocked = true;
    _gameStarted = false;
    _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";

    _enemyField.reset();
    _enemyField.automaticShipsPlacing();
    _playerField.reset();
    _playerField.automaticShipsPlacing();
    _enemyShipsDestroyed = 0;

    emit gameOverSignal();
    emit turnStatusChanged();
    emit gameStatusChanged();
    emit updateGameButtonState();
}






void ModelAdapterNetwork::shot(int index) {
    if (_gameWon || _gameOver) {
        return;
    }

    if (!_gameStarted) {
        return;
    }

    if (!_isMyTurn) {
        return;
    }

    if (_playerFieldBlocked) {
        return;
    }

    if (index < 0 || index >= static_cast<int>(_enemyField.getPlayingField().size())) {
        return;
    }

    auto& field = const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());
    Cell& cell = field[index];

    if (cell._isShoted) {
        return;
    }

    // Блокируем поле до получения результата
    _playerFieldBlocked = true;
    _isMyTurn = false;
    emit gameStatusChanged();
    updateTurnStatus();

    // Отправляем выстрел через бэкенд
    emit shotRequested(index);
}










void ModelAdapterNetwork::shipPlacing() {
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







void ModelAdapterNetwork::newGame() {
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










void ModelAdapterNetwork::startGame() {
    _gameStarted = true;
    _gameWon = false;
    _gameOver = false;
    _isMyTurn = true;
    _playerFieldBlocked = false;
    emit turnStatusChanged();
    emit gameStatusChanged();
    emit playerFieldUpdated();
    emit updateGameButtonState();
}




Q_INVOKABLE bool ModelAdapterNetwork::isGameStarted() const {
    return _gameStarted;
}




QString ModelAdapterNetwork::getTurnStatus() const {
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





Q_INVOKABLE void ModelAdapterNetwork::setTurnStatus(const QString& status) {
    _turnStatus = status;
    emit turnStatusChanged();
}







QString ModelAdapterNetwork::getPlayerGameStatus() {
    int shipsDestroyed = 0;
    int totalShips = _playerField.getShips().size();
    for (const Ship& ship : _playerField.getShips()) {
        if (ship.isDestroyed()) {
            shipsDestroyed++;
        }
    }
    return QString("Корабли уничтожены: %1 / %2").arg(shipsDestroyed).arg(totalShips);
}






QString ModelAdapterNetwork::getEnemyGameStatus() {
    int totalShips = _enemyField.getShips().size();
    return QString("Корабли уничтожены: %1 / %2").arg(_enemyShipsDestroyed).arg(totalShips);
}





Q_INVOKABLE bool ModelAdapterNetwork::isPlayerFieldBlocked() const {
    // Если игра не началась - поле заблокировано
    if (!_gameStarted) {
        return true;
    }
    // Если игра закончена - поле заблокировано
    if (_gameWon || _gameOver) {
        return true;
    }
    // Если не наш ход - поле заблокировано
    return !_isMyTurn;
}






void ModelAdapterNetwork::checkWinCondition() {
    bool playerLost = _playerField.isAllShipsIsDestroyed();

    if (playerLost && !_gameOver) {
        _gameOver = true;
        _gameWon = false;
        _playerFieldBlocked = true;
        _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";
        emit turnStatusChanged();
        emit gameStatusChanged();
        emit gameOver();
        return;
    }
}







void ModelAdapterNetwork::updateTurnStatus() {
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
