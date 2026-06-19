#ifndef MODEL_ADAPTER_HPP
#define MODEL_ADAPTER_HPP

#include <QObject>
#include <QQmlEngine>
#include <qqmlintegration.h>
#include "model.hpp"

class ModelAdapter : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    //Q_PROPERTY(QString enemyGameStatus READ getEnemyGameStatus NOTIFY gameStatusChanged)
    Q_PROPERTY(QString playerGameStatus READ getPlayerGameStatus NOTIFY gameStatusChanged)

  public:
    explicit ModelAdapter(QObject *parent = nullptr);

           // Методы для поля игрока (левое поле)
    Q_INVOKABLE bool getPlayerCellIsOccupied(int index);
    Q_INVOKABLE bool getPlayerCellIsShoted(int index);

    // Методы для поля противника (правое поле)
    Q_INVOKABLE bool getEnemyCellIsOccupied(int index);
    Q_INVOKABLE bool getEnemyCellIsShoted(int index);

    Q_INVOKABLE void shot(int index);
    Q_INVOKABLE void newGame();
    Q_INVOKABLE QString getPlayerGameStatus();
    Q_INVOKABLE bool isPlayerFieldBlocked() const {
        return _playerFieldBlocked;
    }
    Q_INVOKABLE QString getTurnStatus();
    Q_INVOKABLE void shipPlacing();

    // Для сетевой игры
    Q_INVOKABLE void setEnemyField(const QJsonArray &fieldData);

  signals:
    void gameStatusChanged();
    void gameWon();
    void gameOver();
    void turnStatusChanged();
    void playerFieldUpdated();

  private:
    void checkWinCondition();
    void updateTurnStatus();

    Model   _playerField;   // Поле игрока (с кораблями игрока)
    Model   _enemyField;    // Поле противника (куда стреляет игрок)
    bool    _gameWon;
    bool    _gameOver;
    bool    _playerFieldBlocked = false;
    QString _turnStatus = "Ваш ход";
};

#endif // MODEL_ADAPTER_HPP
