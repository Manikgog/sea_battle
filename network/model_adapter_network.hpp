#ifndef MODEL_ADAPTER_NETWORK_HPP
#define MODEL_ADAPTER_NETWORK_HPP

#include <QObject>
#include <QQmlEngine>
#include <qqmlintegration.h>
#include <optional>
#include "../core/model.hpp"
#include "../core/cell.hpp"

class ModelAdapterNetwork : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool gameStarted READ isGameStarted NOTIFY gameStatusChanged)
    Q_PROPERTY(bool myTurn READ isMyTurn NOTIFY gameStatusChanged)
    Q_PROPERTY(bool gameOver READ isGameOver NOTIFY gameStatusChanged)
    Q_PROPERTY(bool playerFieldBlocked READ isPlayerFieldBlocked NOTIFY gameStatusChanged)

  public:
    explicit ModelAdapterNetwork(QObject *parent = nullptr);

           // Методы для поля игрока
    Q_INVOKABLE bool getPlayerCellIsOccupied(int index);
    Q_INVOKABLE bool getPlayerCellIsShoted(int index);

           // Методы для поля противника
    Q_INVOKABLE bool getEnemyCellIsOccupied(int index);
    Q_INVOKABLE bool getEnemyCellIsShoted(int index);

    Q_INVOKABLE void shot(int index);
    Q_INVOKABLE void newGame();
    Q_INVOKABLE QString getPlayerGameStatus();
    Q_INVOKABLE QString getEnemyGameStatus();
    Q_INVOKABLE bool isPlayerFieldBlocked() const {
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
    Q_INVOKABLE QString getTurnStatus();
    Q_INVOKABLE void setTurnStatus(const QString& status) {
        _turnStatus = status;
        emit turnStatusChanged();
    }
    Q_INVOKABLE void shipPlacing();
    Q_INVOKABLE void startGame();
    Q_INVOKABLE bool isGameStarted() const {
        qDebug() << "isGameStarted() called, returning:" << _gameStarted;
        return _gameStarted;
    }
    Q_INVOKABLE void setGameStarted(bool isStarted) {_gameStarted = isStarted;};
    Q_INVOKABLE bool isGameOver() const {
        return _gameWon || _gameOver;
    }

           // Для сетевой игры
    std::optional<ShotResult> setShot(const QString& index);
    void setResult(const QString& result, const QString& index);
    Q_INVOKABLE void setMyTurn(bool isMyTurn) { _isMyTurn = isMyTurn; }
    Q_INVOKABLE bool isMyTurn() const {
        return _isMyTurn;
    }
    Q_INVOKABLE void setPlayerFieldBlocked(bool blocked) {
        _playerFieldBlocked = blocked;
        updateTurnStatus();
        emit gameStatusChanged();
    }

    Q_INVOKABLE std::vector<Cell>& getEnemyFieldRef() {
        return const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());
    }

    Q_INVOKABLE void updateGameStatus() {
        emit gameStatusChanged();
    }



    Q_INVOKABLE void resetGame() {
        _playerField.reset();
        _playerField.automaticShipsPlacing();
        _enemyField.reset();
        _enemyField.automaticShipsPlacing();

        _gameWon = false;
        _gameOver = false;
        _playerFieldBlocked = true;
        _gameStarted = false;
        _isMyTurn = false;
        _turnStatus = "Ожидание начала игры...";

        emit turnStatusChanged();
        emit gameStatusChanged();
        emit playerFieldUpdated();
        emit updateGameButtonState();

        qDebug() << "=== Игра сброшена ===";
    }


    Q_INVOKABLE void gameWon() {
        _gameWon = true;
        _gameOver = true;
        _playerFieldBlocked = true;
        _turnStatus = "🏆 ПОБЕДА! 🏆";
        emit turnStatusChanged();
        emit gameStatusChanged();
        emit gameWonSignal();
    }

    void increaseEnemyShipsDestroyed() {
        _enemyShipsDestroyed++;
    }

    void setEnemyShipsDestroyed(int amountDestroyedShips) {
        _enemyShipsDestroyed = amountDestroyedShips;
    }


  signals:
    void gameStatusChanged();
    void gameWonSignal();
    void gameOver();
    void turnStatusChanged();
    void playerFieldUpdated();
    void updateGameButtonState();
    void shotRequested(int index);

  private:
    void checkWinCondition();
    void updateTurnStatus();

    Model   _playerField;
    Model   _enemyField;
    bool    _gameWon = false;
    bool    _gameOver = false;
    bool    _playerFieldBlocked = true;
    bool    _gameStarted = false;
    QString _turnStatus;
    bool    _isMyTurn = false;
    int     _enemyShipsDestroyed = 0;
};

#endif // MODEL_ADAPTER_NETWORK_HPP
