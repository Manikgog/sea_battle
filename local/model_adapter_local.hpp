#ifndef MODEL_ADAPTER_LOCAL_HPP
#define MODEL_ADAPTER_LOCAL_HPP

#include <QObject>
#include <QQmlEngine>
#include "../core/model.hpp"
#include "bot.hpp"

class ModelAdapterLocal : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString botGameStatus READ getBotGameStatus NOTIFY gameStatusChanged)
    Q_PROPERTY(QString playerGameStatus READ getPlayerGameStatus NOTIFY gameStatusChanged)

public:
    explicit ModelAdapterLocal(QObject *parent = nullptr);

    Q_INVOKABLE bool getBotCellIsShoted(int index);
    Q_INVOKABLE bool getBotCellIsOccupied(int index);

    Q_INVOKABLE bool getPlayerCellIsOccupied(int index);
    Q_INVOKABLE bool getPlayerCellIsShoted(int index);

    Q_INVOKABLE void shot(int index);
    Q_INVOKABLE void botMove();
    Q_INVOKABLE void newGame();
    Q_INVOKABLE QString getBotGameStatus();
    Q_INVOKABLE QString getPlayerGameStatus();
    Q_INVOKABLE bool isPlayerFieldBlocked() {
        return _playerFieldBlocked;
    }
    Q_INVOKABLE QString getTurnStatus();

signals:
    void gameStatusChanged();
    void gameWon();
    void gameOver();
    void turnStatusChanged();

private:
    void checkWinCondition();
    void updateTurnStatus();

    Model   _model;               // поле бота
    Bot     _bot;                 // бот, который содержит поле игрока
    bool    _gameWon = false;
    bool    _gameOver = false;
    bool    _playerFieldBlocked = false;    // признак блокировки поля игрока
    QString _turnStatus = "Ваш ход";        // текущий статус хода
};

#endif // MODEL_ADAPTER_LOCAL_HPP
