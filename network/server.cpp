#include "server.hpp"

Server::Server(QObject *parent)
    : QObject(parent) {
    _networkManager = std::make_unique<NetworkManager>(this);

    connect(_networkManager.get(), &NetworkManager::connected,
            this, &Server::onConnected);
    connect(_networkManager.get(), &NetworkManager::disconnected,
            this, &Server::onDisconnected);
    connect(_networkManager.get(), &NetworkManager::messageReceived,
            this, &Server::onMessageReceived);
    connect(_networkManager.get(), &NetworkManager::playerReady,
            this, [this](const QString& name) {
                _playerName = name;
                emit clientReady();
            });
    connect(_networkManager.get(), &NetworkManager::errorOccurred,
            this, &Server::error);
}

bool Server::start(int port) {
    return _networkManager->startServer(port);
}

void Server::stop() {
    _networkManager->disconnect();
}

bool Server::isRunning() const {
    return _networkManager->isConnected();
}

void Server::sendMove(const QJsonObject& move) {
    QJsonObject message = move;
    message["type"] = "move";
    _networkManager->sendMessage(message);
}

void Server::sendGameEnded(const QString& winner) {
    QJsonObject message;
    message["type"] = "gameEnded";
    message["winner"] = winner;
    _networkManager->sendMessage(message);
}

void Server::onConnected() {
    // Отправляем сообщение о старте игры
    QJsonObject startMsg;
    startMsg["type"] = "startGame";
    _networkManager->sendMessage(startMsg);

    emit playerConnected(_playerName);
}

void Server::onDisconnected() {
    emit playerDisconnected();
}

void Server::onMessageReceived(const QJsonObject& message) {
    QString type = message["type"].toString();

    if (type == "move") {
        emit moveReceived(message);
    }
}
