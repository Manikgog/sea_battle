#ifndef MODEL_ADAPTER_HPP
#define MODEL_ADAPTER_HPP

#include <QObject>
#include <QQmlEngine>
#include <qqmlintegration.h>
#include "model.hpp"
#include "bot.hpp"

class ModelAdapter : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString gameStatus READ getGameStatus NOTIFY gameStatusChanged)

  public:
    explicit ModelAdapter(QObject *parent = nullptr);

    Q_INVOKABLE bool getBotCellIsShoted(int index);
    Q_INVOKABLE bool getBotCellIsOccupied(int index);

    Q_INVOKABLE bool getPlayerCellIsOccupied(int index);
    Q_INVOKABLE bool getPlayerCellIsShoted(int index);

    Q_INVOKABLE void shot(int index);
    Q_INVOKABLE void botMove();
    Q_INVOKABLE void newGame();
    Q_INVOKABLE QString getGameStatus();

  signals:
    void gameStatusChanged();
    void gameWon();

  private:
    void checkWinCondition();

    Model   _model;               // поле бота
    Bot     _bot;                 // бот, который содержит поле игрока
    bool _gameWon;
};

#endif // MODEL_ADAPTER_HPP
