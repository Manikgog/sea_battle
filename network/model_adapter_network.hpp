#ifndef MODEL_ADAPTER_NETWORK_HPP
#define MODEL_ADAPTER_NETWORK_HPP

#include <QObject>
#include <QQmlEngine>
#include "../core/model.hpp"
#include "server.hpp"
#include "client.hpp"

class ModelAdapterNetwork : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString botGameStatus READ getBotGameStatus NOTIFY gameStatusChanged)
    Q_PROPERTY(QString playerGameStatus READ getPlayerGameStatus NOTIFY gameStatusChanged)
    Q_PROPERTY(QString gameStatusText READ getGameStatusText NOTIFY gameStatusChanged)
    Q_PROPERTY(bool isServer READ isServer NOTIFY gameModeChanged)
    Q_PROPERTY(bool isClient READ isClient NOTIFY gameModeChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)

public:
    explicit ModelAdapterNetwork(QObject *parent = nullptr);
    ~ModelAdapterNetwork();

    // Методы для работы с полем
    Q_INVOKABLE bool getBotCellIsShoted(int index);
    Q_INVOKABLE bool getBotCellIsOccupied(int index);
    Q_INVOKABLE bool getPlayerCellIsOccupied(int index);
    Q_INVOKABLE bool getPlayerCellIsShoted(int index);

    // Игровые методы
    Q_INVOKABLE void shot(int index);
    Q_INVOKABLE void newGame();
    Q_INVOKABLE QString getBotGameStatus();
    Q_INVOKABLE QString getPlayerGameStatus();
    Q_INVOKABLE QString getGameStatusText();

    // Сетевые методы
    Q_INVOKABLE bool startServer(int port = 12345);
    Q_INVOKABLE bool connectToServer(const QString& address, int port = 12345);
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void setPlayerName(const QString& name);
    Q_INVOKABLE QString getPlayerName() const;
    Q_INVOKABLE bool isServer() const { return _isServer; }
    Q_INVOKABLE bool isClient() const { return _isClient; }
    Q_INVOKABLE bool isConnected() const { return _isConnected; }
    Q_INVOKABLE bool isMyTurn() const { return _isMyTurn; }
    Q_INVOKABLE bool isGameStarted() const { return _gameStarted; }

signals:
    void gameStatusChanged();
    void gameWon();
    void gameOver();
    void gameModeChanged();
    void connectionStatusChanged();
    void opponentReady();
    void turnChanged(bool myTurn);
    void gameMessage(const QString& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onOpponentReady();
    void onOpponentMove(const QJsonObject& move);
    void onGameEnded(const QString& winner);

private:
    void checkWinCondition();
    void processOpponentMove(int index);
    void sendMoveToOpponent(int index, bool hit, bool destroyed);
    void endGame(const QString& winner);
    void updateStatus();

    Model   _model;               // поле игрока (то, что видит противник)
    Model   _playerModel;         // поле противника (то, что видит игрок)
    bool    _gameWon = false;
    bool    _gameOver = false;

    // Сетевые переменные
    bool _isServer = false;
    bool _isClient = false;
    bool _isConnected = false;
    bool _isMyTurn = false;
    bool _opponentReady = false;
    bool _gameStarted = false;
    QString _playerName = "Игрок";

    Server* _server = nullptr;
    Client* _client = nullptr;
};

#endif // MODEL_ADAPTER_NETWORK_HPP
