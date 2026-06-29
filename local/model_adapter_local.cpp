#include <QDebug>
#include <QTimer>

#include "model_adapter_local.hpp"



ModelAdapterLocal::ModelAdapterLocal(QObject *parent)
    : QObject(parent)
    , _gameWon(false)
    , _gameOver(false)
    , _isPlayerTurn(false)
    , _isGameStarted(false) {
    _model.automaticShipsPlacing();
    _bot._model.automaticShipsPlacing();
    updateTurnStatus();
}





bool ModelAdapterLocal::getBotCellIsShoted(int index) {
    //qDebug() << __FUNCTION__ << "index =" << index;
    if (index < 0 || index >= _model.getPlayingField().size()) {
        return false;
    }
    return _model.getPlayingField()[index]._isShoted;
}




bool ModelAdapterLocal::getBotCellIsOccupied(int index) {
    if (index < 0 || index >= _model.getPlayingField().size()) {
        return false;
    }
    //qDebug() << __FUNCTION__ << "index =" << index;
    return _model.getPlayingField()[index]._isOccupied;
}





bool ModelAdapterLocal::getPlayerCellIsOccupied(int index)
{
    if (index < 0 || index >= _bot._model.getPlayingField().size()) {
        return false;
    }
    //qDebug() << __FUNCTION__ << "index =" << index;
    return _bot._model.getPlayingField()[index]._isOccupied;
}





bool ModelAdapterLocal::getPlayerCellIsShoted(int index)
{
    if (index < 0 || index >= _bot._model.getPlayingField().size()) {
        return false;
    }
    //qDebug() << __FUNCTION__ << "index =" << index;
    return _bot._model.getPlayingField()[index]._isShoted;
}




/**
 * @brief ModelAdapterLocal::shot метод вызываемый при клике мышки игроком по полю бота
 * @param index
 */
void ModelAdapterLocal::shot(int index) {
    if (_gameWon || _gameOver || _playerFieldBlocked) {
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
            _playerFieldBlocked = true;
            _turnStatus = "Ход бота...";
            _isPlayerTurn = false;
            emit turnStatusChanged();
            emit gameStatusChanged();
            // Используем QTimer для небольшой задержки перед ходом бота
            QTimer::singleShot(1000, this, [this]() {
                botMove();
                _playerFieldBlocked = false;
                _turnStatus = "Ваш ход";
                emit turnStatusChanged();
                emit gameStatusChanged(); // Разблокируем UI
            });
        } else if(!_gameWon && !is_miss) {
            // Если попал, ход остается за игроком
            _turnStatus = "Ваш ход (попадание!)";
            _isPlayerTurn = true;
            emit turnStatusChanged();
            emit gameStatusChanged();
        }
    }
}




void ModelAdapterLocal::botMove() {
    if (_gameWon || _gameOver) {
        _playerFieldBlocked = false;
        _turnStatus = "Ход бота ...";
        _isPlayerTurn = false;
        emit turnStatusChanged();
        emit gameStatusChanged();
        return;
    }

    // Бот выбирает случайную клетку для выстрела
    int index = _bot.shoot();
    qDebug() << "Bot shoots at player cell:" << index;

    auto& playerField = const_cast<std::vector<Cell>&>(_bot._model.getPlayingField());
    if (index >= 0 && index < static_cast<int>(playerField.size())) {
        auto& cell = playerField[index];
        if (!cell._isShoted) {
            int destroyed_ships_before = _bot._model.getDestroyedShipsAmount();
            cell._isShoted = true;

            if (cell._isOccupied) {
                int destroyed_ships_after = _bot._model.getDestroyedShipsAmount();

                if(destroyed_ships_after > destroyed_ships_before) {
                    _bot.setHit(index, true);
                } else {
                    _bot.setHit(index);
                }
                qDebug() << "Bot HIT at player cell:" << index;

                checkWinCondition();

                // Если бот попал и игра не окончена, делаем еще один ход с задержкой
                if (!_gameWon && !_gameOver) {
                    _turnStatus = "Ход бота (попадание!)";
                    _isPlayerTurn = false;
                    emit turnStatusChanged();
                    emit gameStatusChanged();
                    QTimer::singleShot(500, this, [this]() {
                        botMove();
                    });
                } else {
                    _playerFieldBlocked = false;
                    _turnStatus = "Игра окончена";
                    _isPlayerTurn = false;
                    emit turnStatusChanged();
                    emit gameStatusChanged();
                }
            } else {
                qDebug() << "Bot MISS at player cell:" << index;
                checkWinCondition();
                if (!_gameWon && !_gameOver) {
                    // После промаха ход переходит к игроку
                    _playerFieldBlocked = false;
                    _turnStatus = "Ваш ход";
                    _isPlayerTurn = true;
                    emit turnStatusChanged();
                    emit gameStatusChanged();
                } else {
                    // Если игра окончена, обновляем статус
                    _playerFieldBlocked = false;
                    if (_gameWon) {
                        _turnStatus = "🏆 ПОБЕДА! 🏆";
                        _isPlayerTurn = false;
                    } else if (_gameOver) {
                        _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";
                        _isPlayerTurn = false;
                    }
                    emit turnStatusChanged();
                    emit gameStatusChanged();
                }
            }
        }
    }
}




void ModelAdapterLocal::newGame() {
    _model.reset();
    _model.automaticShipsPlacing();
    _bot.reset();
    _bot._model.automaticShipsPlacing();
    _gameWon = false;
    _gameOver = false;
    _playerFieldBlocked = false;
    _turnStatus = "Ваш ход";
    _isPlayerTurn = true;
    emit turnStatusChanged();
    emit gameStatusChanged();
}




QString ModelAdapterLocal::getTurnStatus() {
    return _turnStatus;
}




bool ModelAdapterLocal::isPlayerTurn() const
{
    return _isPlayerTurn;
}




QString ModelAdapterLocal::getBotGameStatus() {

    int shipsDestroyed = 0;
    int totalShips = _model.getShips().size();

    for (const Ship& ship : _model.getShips()) {
        if (ship.isDestroyed()) {
            shipsDestroyed++;
        }
    }

    return QString("Корабли уничтожены: %1 / %2").arg(shipsDestroyed).arg(totalShips);
}



QString ModelAdapterLocal::getPlayerGameStatus() {

    int shipsDestroyed = 0;
    int totalShips = _bot._model.getShips().size();

    for (const Ship& ship : _bot._model.getShips()) {
        if (ship.isDestroyed()) {
            shipsDestroyed++;
        }
    }

    return QString("Корабли уничтожены: %1 / %2").arg(shipsDestroyed).arg(totalShips);
}




void ModelAdapterLocal::checkWinCondition() {
    bool playerLost = _bot._model.isAllShipsIsDestroyed();  // Все корабли игрока уничтожены?
    bool playerWon = _model.isAllShipsIsDestroyed();         // Все корабли бота уничтожены?

    qDebug() << "checkWinCondition: playerLost =" << playerLost << "playerWon =" << playerWon;

    if (playerLost && !_gameOver) {
        _gameOver = true;
        _playerFieldBlocked = false;
        qDebug() << "BOT WINS! Game Over!";
        _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";
        _isPlayerTurn = false;
        emit turnStatusChanged();
        emit gameStatusChanged();
        emit gameOver();
        return;
    }

    if (playerWon && !_gameWon) {
        _gameWon = true;
        qDebug() << "PLAYER WINS!";
        _playerFieldBlocked = false;
        _turnStatus = "🏆 ПОБЕДА! 🏆";
        _isPlayerTurn = false;
        emit turnStatusChanged();
        qDebug() << "PLAYER WINS!";
        emit gameWon();
        emit gameStatusChanged();
    }
}



void ModelAdapterLocal::updateTurnStatus() {
    if(_isGameStarted) {
        if (_gameWon) {
            _turnStatus = "🏆 ПОБЕДА! 🏆";
            _isPlayerTurn = false;
            _isGameStarted = false;
        } else if (_gameOver) {
            _turnStatus = "💀 ВЫ ПРОИГРАЛИ! 💀";
            _isPlayerTurn = false;
            _isGameStarted = false;
        } else  if (!_isPlayerTurn) {
            _turnStatus = "Ход бота...";
            _isPlayerTurn = false;
        } else if (_isPlayerTurn){
            _turnStatus = "Ваш ход";
            _isPlayerTurn = false;
        }
    } else {
        _turnStatus = "Ожидание начала игры ...";
    }
    emit turnStatusChanged();
}
