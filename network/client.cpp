#include "client.hpp"

Client::Client(QObject *parent)
    : QObject(parent) {
    _networkManager = std::make_unique<NetworkManager>(this);

    connect(_networkManager.get(), &NetworkManager::connected,
            this, &Client::onConnected);
    connect(_networkManager.get(), &NetworkManager::disconnected,
            this, &Client::onDisconnected);
    connect(_networkManager.get(), &NetworkManager::messageReceived,
            this, &Client::onMessageReceived);
    connect(_networkManager.get(), &NetworkManager::gameStarted,
            this, &Client::gameStarted);
    connect(_networkManager.get(), &NetworkManager::enemyMove,
            this, &Client::moveReceived);
    connect(_networkManager.get(), &NetworkManager::gameEnded,
            this, &Client::gameEnded);
    connect(_networkManager.get(), &NetworkManager::errorOccurred,
            this, &Client::error);
}

bool Client::connectToServer(const QString& address, int port) {
    return _networkManager->connectToServer(address, port);
}

void Client::disconnect() {
    _networkManager->disconnect();
}

bool Client::isConnected() const {
    return _networkManager->isConnected();
}

void Client::sendReady(const QString& playerName) {
    QJsonObject message;
    message["type"] = "ready";
    message["playerName"] = playerName;
    _networkManager->sendMessage(message);
}

void Client::sendMove(const QJsonObject& move) {
    QJsonObject message = move;
    message["type"] = "move";
    _networkManager->sendMessage(message);
}

void Client::onConnected() {
    emit connected();
}

void Client::onDisconnected() {
    emit disconnected();
}

void Client::onMessageReceived(const QJsonObject& message) {
    QString type = message["type"].toString();

    if (type == "startGame") {
        emit gameStarted();
    } else if (type == "move") {
        emit moveReceived(message);
    } else if (type == "gameEnded") {
        QString winner = message["winner"].toString();
        emit gameEnded(winner);
    }
}
