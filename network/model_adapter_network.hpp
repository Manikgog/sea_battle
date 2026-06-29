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
    Q_INVOKABLE bool isPlayerFieldBlocked() const;
    Q_INVOKABLE QString getTurnStatus() const;
    Q_INVOKABLE void setTurnStatus(const QString& status);
    Q_INVOKABLE void shipPlacing();
    Q_INVOKABLE void startGame();
    Q_INVOKABLE bool isGameStarted() const;
    Q_INVOKABLE void setGameStarted(bool isStarted) {
        _gameStarted = isStarted;
    }
    Q_INVOKABLE bool isGameOver() const {
        return _gameWon || _gameOver;
    }

    std::optional<ShotResult> setShot(const QString& index);
    void setResult(const QString& result, const QString& index);
    Q_INVOKABLE void setMyTurn(bool isMyTurn) {
        _isMyTurn = isMyTurn;
    }
    Q_INVOKABLE bool isMyTurn() const {
        return _isMyTurn;
    }
    Q_INVOKABLE void setPlayerFieldBlocked(bool blocked);

    Q_INVOKABLE std::vector<Cell>& getEnemyFieldRef() {
        return const_cast<std::vector<Cell>&>(_enemyField.getPlayingField());
    }

    Q_INVOKABLE void updateGameStatus() {
        emit gameStatusChanged();
    }
    Q_INVOKABLE void gameWon();
    Q_INVOKABLE void gameOver();

    void increaseEnemyShipsDestroyed() {
        _enemyShipsDestroyed++;
    }

    void setEnemyShipsDestroyed(int amountDestroyedShips) {
        _enemyShipsDestroyed = amountDestroyedShips;
    }

    bool isGameStarted() {
        return _gameStarted;
    }


  signals:
    void gameStatusChanged();
    void gameWonSignal();
    void gameOverSignal();
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
