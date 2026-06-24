#ifndef MODEL_ADAPTER_HPP
#define MODEL_ADAPTER_HPP

#include <QObject>
#include <QQmlEngine>
#include <qqmlintegration.h>
#include "model.hpp"

class ModelAdapter : public QObject {
    Q_OBJECT
    QML_ELEMENT

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
        return _playerFieldBlocked || _gameWon || _gameOver;
    }
    Q_INVOKABLE QString getTurnStatus();
    Q_INVOKABLE void setTurnStatus(const QString& status) {
        _turnStatus = status;
        emit turnStatusChanged();
    }
    Q_INVOKABLE void shipPlacing();
    Q_INVOKABLE void startGame();
    Q_INVOKABLE bool isGameStarted() const {
        return _gameStarted;
    }
    Q_INVOKABLE bool isGameOver() const {
        return _gameWon || _gameOver;
    }

           // Для сетевой игры
    std::optional<ShotResult> setShot(const QString& index);
    void setResult(const QString& result, const QString& index);

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
};

#endif // MODEL_ADAPTER_HPP
