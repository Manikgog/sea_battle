#ifndef MODEL_ADAPTER_HPP
#define MODEL_ADAPTER_HPP

#include <QObject>
#include <QQmlEngine>
#include <qqmlintegration.h>
#include "model.hpp"

class ModelAdapter : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool gameStarted READ isGameStarted NOTIFY gameStatusChanged)
    Q_PROPERTY(bool myTurn READ isMyTurn NOTIFY gameStatusChanged)
    Q_PROPERTY(bool gameOver READ isGameOver NOTIFY gameStatusChanged)
    Q_PROPERTY(bool playerFieldBlocked READ isPlayerFieldBlocked NOTIFY gameStatusChanged)

  public:
    explicit ModelAdapter(QObject *parent = nullptr);

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

  signals:
    void gameStatusChanged();
    void gameWon();
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
};

#endif // MODEL_ADAPTER_HPP
