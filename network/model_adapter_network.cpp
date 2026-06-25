#include "model_adapter_network.hpp"
#include "server.hpp"
#include "client.hpp"
#include <QDebug>
#include <QTimer>

ModelAdapterNetwork::ModelAdapterNetwork(QObject *parent)
    : QObject(parent) {
    _model.automaticShipsPlacing();
    _playerModel.automaticShipsPlacing();
    updateStatus();
}

// Добавьте определение деструктора
ModelAdapterNetwork::~ModelAdapterNetwork() {
    delete _server;
    delete _client;
}

bool ModelAdapterNetwork::getBotCellIsShoted(int index) {
    if (index < 0 || index >= static_cast<int>(_model.getPlayingField().size())) {
        return false;
    }
    return _model.getPlayingField()[index]._isShoted;
}

bool ModelAdapterNetwork::getBotCellIsOccupied(int index) {
    if (index < 0 || index >= static_cast<int>(_model.getPlayingField().size())) {
        return false;
    }
    return _model.getPlayingField()[index]._isOccupied;
}

bool ModelAdapterNetwork::getPlayerCellIsOccupied(int index) {
    if (index < 0 || index >= static_cast<int>(_playerModel.getPlayingField().size())) {
        return false;
    }
    return _playerModel.getPlayingField()[index]._isOccupied;
}

bool ModelAdapterNetwork::getPlayerCellIsShoted(int index) {
    if (index < 0 || index >= static_cast<int>(_playerModel.getPlayingField().size())) {
        return false;
    }
    return _playerModel.getPlayingField()[index]._isShoted;
}

void ModelAdapterNetwork::shot(int index) {
    if (_gameWon || _gameOver || !_gameStarted) {
        return;
    }

    if (!_isMyTurn) {
        emit gameMessage("Сейчас ход противника");
        return;
    }

    if (index < 0 || index >= static_cast<int>(_playerModel.getPlayingField().size())) {
        return;
    }

    auto& field = const_cast<std::vector<Cell>&>(_playerModel.getPlayingField());
    auto& cell = field[index];

    if (cell._isShoted) {
        return;
    }

    qDebug() << "Player shoots at:" << index;

    cell._isShoted = true;
    bool isHit = cell._isOccupied;
    bool isDestroyed = false;

    if (isHit) {
        for (const Ship& ship : _playerModel.getShips()) {
            if (ship.isDestroyed()) {
                isDestroyed = true;
                qDebug() << "Enemy ship destroyed!";
            }
        }
    }

    sendMoveToOpponent(index, isHit, isDestroyed);
    checkWinCondition();
    updateStatus();

    if (_gameWon || _gameOver) {
        return;
    }

    if (!isHit) {
        _isMyTurn = false;
        emit turnChanged(false);
        emit gameMessage("Промах! Ход переходит к противнику");
    } else {
        emit gameMessage("Попадание! Ваш ход продолжается");
    }

    updateStatus();
}

void ModelAdapterNetwork::newGame() {
    _model.reset();
    _model.automaticShipsPlacing();
    _playerModel.reset();
    _playerModel.automaticShipsPlacing();
    _gameWon = false;
    _gameOver = false;
    _gameStarted = false;
    _isMyTurn = false;
    _opponentReady = false;

    if (_isServer) {
        _isMyTurn = true;
        _gameStarted = true;
        emit turnChanged(true);
        emit gameMessage("Новая игра начата! Ваш ход.");

        QJsonObject startMsg;
        startMsg["type"] = "startGame";
        _server->sendMove(startMsg);
    }

    updateStatus();
    emit gameStatusChanged();
    qDebug() << "New game started";
}

bool ModelAdapterNetwork::startServer(int port) {
    if (_isConnected) {
        disconnectFromServer();
    }

    _server = new Server(this);

    connect(_server, &Server::playerConnected,
            this, &ModelAdapterNetwork::onConnected);
    connect(_server, &Server::playerDisconnected,
            this, &ModelAdapterNetwork::onDisconnected);
    connect(_server, &Server::clientReady,
            this, &ModelAdapterNetwork::onOpponentReady);
    connect(_server, &Server::moveReceived,
            this, [this](const QJsonObject& move) {
                onOpponentMove(move);
            });
    connect(_server, &Server::error,
            this, [this](const QString& error) {
                emit gameMessage("Ошибка сервера: " + error);
            });

    if (_server->start(port)) {
        _isServer = true;
        _isClient = false;
        _isConnected = false;
        _isMyTurn = false;
        _gameStarted = false;
        emit gameModeChanged();
        emit gameMessage("Сервер запущен на порту " + QString::number(port) +
                         ". Ожидание подключения противника...");
        updateStatus();
        return true;
    }

    return false;
}

bool ModelAdapterNetwork::connectToServer(const QString& address, int port) {
    if (_isConnected) {
        disconnectFromServer();
    }

    _client = new Client(this);

    connect(_client, &Client::connected,
            this, &ModelAdapterNetwork::onConnected);
    connect(_client, &Client::disconnected,
            this, &ModelAdapterNetwork::onDisconnected);
    connect(_client, &Client::gameStarted,
            this, [this]() {
                _gameStarted = true;
                _isMyTurn = false;
                emit turnChanged(false);
                emit gameMessage("Игра началась! Ход противника");
                updateStatus();
            });
    connect(_client, &Client::moveReceived,
            this, &ModelAdapterNetwork::onOpponentMove);
    connect(_client, &Client::gameEnded,
            this, &ModelAdapterNetwork::onGameEnded);
    connect(_client, &Client::error,
            this, [this](const QString& error) {
                emit gameMessage("Ошибка клиента: " + error);
            });

    if (_client->connectToServer(address, port)) {
        _isClient = true;
        _isServer = false;
        _isConnected = false;
        _isMyTurn = false;
        _gameStarted = false;
        emit gameModeChanged();
        emit gameMessage("Подключение к серверу " + address + ":" +
                         QString::number(port) + "...");
        updateStatus();
        return true;
    }

    return false;
}

void ModelAdapterNetwork::disconnectFromServer() {
    if (_server) {
        _server->stop();
        delete _server;
        _server = nullptr;
    }
    if (_client) {
        _client->disconnect();
        delete _client;
        _client = nullptr;
    }
    _isServer = false;
    _isClient = false;
    _isConnected = false;
    _isMyTurn = false;
    _gameStarted = false;
    _opponentReady = false;
    emit gameModeChanged();
    emit connectionStatusChanged();
    emit gameMessage("Отключено от сервера");
    updateStatus();
}

void ModelAdapterNetwork::setPlayerName(const QString& name) {
    _playerName = name;
    if (_client) {
        _client->sendReady(name);
    }
}

QString ModelAdapterNetwork::getPlayerName() const {
    return _playerName;
}

void ModelAdapterNetwork::onConnected() {
    _isConnected = true;
    emit connectionStatusChanged();

    if (_isServer) {
        emit gameMessage("Противник подключился! Ожидание готовности...");
    } else {
        // Клиент отправляет готовность после подключения
        if (_client) {
            _client->sendReady(_playerName);
        }
        emit gameMessage("Подключено к серверу! Ожидание начала игры...");
    }
    updateStatus();
}

void ModelAdapterNetwork::onDisconnected() {
    _isConnected = false;
    _gameStarted = false;
    emit connectionStatusChanged();
    emit gameMessage("Соединение разорвано");

    if (_isServer) {
        delete _server;
        _server = nullptr;
    } else {
        delete _client;
        _client = nullptr;
    }
    _isServer = false;
    _isClient = false;
    emit gameModeChanged();
    updateStatus();
}

void ModelAdapterNetwork::onOpponentReady() {
    _opponentReady = true;
    emit opponentReady();

    if (_isServer) {
        _isMyTurn = true;
        _gameStarted = true;
        emit turnChanged(true);
        emit gameMessage("Противник готов! Вы начинаете первый ход.");

        QJsonObject startMsg;
        startMsg["type"] = "startGame";
        _server->sendMove(startMsg);
        updateStatus();
    }
}

void ModelAdapterNetwork::onOpponentMove(const QJsonObject& move) {
    if (!_gameStarted) {
        return;
    }

    int index = move["index"].toInt();
    bool hit = move["hit"].toBool();
    bool destroyed = move["destroyed"].toBool();

    processOpponentMove(index);

    if (hit) {
        emit gameMessage("Противник попал в вашу клетку!");
        // Ход остаётся у противника
    } else {
        emit gameMessage("Противник промахнулся. Ваш ход!");
        _isMyTurn = true;
        emit turnChanged(true);
    }

    checkWinCondition();
    updateStatus();
}

void ModelAdapterNetwork::processOpponentMove(int index) {
    auto& field = const_cast<std::vector<Cell>&>(_model.getPlayingField());
    if (index >= 0 && index < static_cast<int>(field.size())) {
        auto& cell = field[index];
        if (!cell._isShoted) {
            cell._isShoted = true;

            if (cell._isOccupied) {
                for (const Ship& ship : _model.getShips()) {
                    if (ship.isDestroyed()) {
                        // Отмечаем все клетки вокруг уничтоженного корабля
                        // Здесь можно добавить логику отметки
                        break;
                    }
                }
            }
        }
    }
}

void ModelAdapterNetwork::sendMoveToOpponent(int index, bool hit, bool destroyed) {
    QJsonObject move;
    move["index"] = index;
    move["hit"] = hit;
    move["destroyed"] = destroyed;

    if (_server) {
        _server->sendMove(move);
    } else if (_client) {
        _client->sendMove(move);
    }
}

void ModelAdapterNetwork::onGameEnded(const QString& winner) {
    _gameOver = true;
    _gameStarted = false;
    emit gameMessage("Игра окончена! Победитель: " + winner);

    if (winner == _playerName) {
        emit gameWon();
    } else {
        emit gameOver();
    }
    updateStatus();
}

void ModelAdapterNetwork::endGame(const QString& winner) {
    if (_server) {
        _server->sendGameEnded(winner);
    }
    onGameEnded(winner);
}

QString ModelAdapterNetwork::getBotGameStatus() {
    if (!_gameStarted) {
        return "Ожидание начала игры...";
    }
    if (_gameWon) return "🏆 ПОБЕДА!";
    if (_gameOver) return "💀 ПОРАЖЕНИЕ";

    int shipsDestroyed = 0;
    int totalShips = _playerModel.getShips().size();

    for (const Ship& ship : _playerModel.getShips()) {
        if (ship.isDestroyed()) {
            shipsDestroyed++;
        }
    }

    return QString("Корабли уничтожены: %1 / %2").arg(shipsDestroyed).arg(totalShips);
}

QString ModelAdapterNetwork::getPlayerGameStatus() {
    if (!_gameStarted) {
        return "Ожидание начала игры...";
    }
    if (_gameWon) return "🏆 ПОБЕДА!";
    if (_gameOver) return "💀 ПОРАЖЕНИЕ";

    int shipsDestroyed = 0;
    int totalShips = _model.getShips().size();

    for (const Ship& ship : _model.getShips()) {
        if (ship.isDestroyed()) {
            shipsDestroyed++;
        }
    }

    return QString("Корабли уничтожены: %1 / %2").arg(shipsDestroyed).arg(totalShips);
}

QString ModelAdapterNetwork::getGameStatusText() {
    if (_isServer) {
        return "Сервер" + QString(_isMyTurn ? " (Ваш ход)" : " (Ход противника)");
    } else if (_isClient) {
        return "Клиент" + QString(_isMyTurn ? " (Ваш ход)" : " (Ход противника)");
    }
    return "Не подключен";
}

void ModelAdapterNetwork::updateStatus() {
    emit gameStatusChanged();
}

void ModelAdapterNetwork::checkWinCondition() {
    bool playerLost = _model.isAllShipsIsDestroyed();
    bool playerWon = _playerModel.isAllShipsIsDestroyed();

    qDebug() << "checkWinCondition: playerLost =" << playerLost << "playerWon =" << playerWon;

    if (playerLost && !_gameOver) {
        _gameOver = true;
        _gameStarted = false;
        qDebug() << "PLAYER LOST!";
        if (_isServer) {
            endGame("Противник");
        } else if (_isClient) {
            endGame("Противник");
        }
        return;
    }

    if (playerWon && !_gameWon) {
        _gameWon = true;
        _gameStarted = false;
        qDebug() << "PLAYER WON!";
        if (_isServer || _isClient) {
            endGame(_playerName);
        }
        return;
    }
}
