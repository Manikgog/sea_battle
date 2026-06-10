#ifndef MODEL_ADAPTER_HPP
#define MODEL_ADAPTER_HPP

#include <QObject>
#include <QQmlEngine>
#include "model.hpp"

class ModelAdapter : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString gameStatus READ getGameStatus NOTIFY gameStatusChanged)

  public:
    explicit ModelAdapter(QObject *parent = nullptr);

    Q_INVOKABLE bool getCellIsShoted(int index);
    Q_INVOKABLE bool getCellIsOccupied(int index);
    Q_INVOKABLE void shot(int index);
    Q_INVOKABLE void newGame();
    Q_INVOKABLE QString getGameStatus();

  signals:
    void gameStatusChanged();
    void gameWon();

  private:
    void checkWinCondition();

    Model _model;
    bool _gameWon;
};

#endif // MODEL_ADAPTER_HPP
