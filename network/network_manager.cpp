#include "network_manager.hpp"
#include <QDebug>
#include <QHostAddress>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent) {
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::startServer(int port) {
    if (_role != Role::None) {
        disconnect();
    }

    _server = std::make_unique<QTcpServer>(this);
    connect(_server.get(), &QTcpServer::newConnection,
            this, &NetworkManager::onNewConnection);

    if (!_server->listen(QHostAddress::Any, port)) {
        emit errorOccurred("Не удалось запустить сервер на порту " + QString::number(port));
        return false;
    }

    _role = Role::Server;
    qDebug() << "Сервер запущен на порту" << port;
    return true;
}

bool NetworkManager::connectToServer(const QString& address, int port) {
    if (_role != Role::None) {
        disconnect();
    }

    _socket = std::make_unique<QTcpSocket>(this);
    connect(_socket.get(), &QTcpSocket::connected,
            this, &NetworkManager::onConnected);
    connect(_socket.get(), &QTcpSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead,
            this, &NetworkManager::onReadyRead);
    connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError error) {
                emit errorOccurred("Ошибка сокета: " + QString::number(error));
            });

    _socket->connectToHost(address, port);
    _role = Role::Client;
    return true;
}

void NetworkManager::disconnect() {
    _isConnected = false;
    _role = Role::None;

    if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
        _socket->disconnectFromHost();
    }
    _socket.reset();

    if (_server && _server->isListening()) {
        _server->close();
    }
    _server.reset();
}

void NetworkManager::sendMessage(const QJsonObject& message) {
    if (!_isConnected) {
        qDebug() << "Не удалось отправить сообщение: соединение не установлено";
        return;
    }
    sendJsonMessage(message);
}

bool NetworkManager::isConnected() const {
    return _isConnected;
}

void NetworkManager::onReadyRead() {
    if (!_socket) return;

    _buffer.append(_socket->readAll());
    processData();
}

void NetworkManager::onConnected() {
    _isConnected = true;
    emit connected();
    qDebug() << "Подключено к серверу";
}

void NetworkManager::onDisconnected() {
    _isConnected = false;
    emit disconnected();
    qDebug() << "Соединение разорвано";
}

void NetworkManager::onNewConnection() {
    if (!_server) return;

    QTcpSocket* clientSocket = _server->nextPendingConnection();
    if (!clientSocket) return;

    if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
        clientSocket->disconnectFromHost();
        return;
    }

    _socket = std::unique_ptr<QTcpSocket>(clientSocket);
    connect(_socket.get(), &QTcpSocket::readyRead,
            this, &NetworkManager::onReadyRead);
    connect(_socket.get(), &QTcpSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError error) {
                emit errorOccurred("Ошибка сокета: " + QString::number(error));
            });

    _isConnected = true;
    emit connected();
    qDebug() << "Клиент подключился к серверу";
}

void NetworkManager::sendJsonMessage(const QJsonObject& message) {
    if (!_socket || _socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Сокет не подключен";
        return;
    }

    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    _socket->write(data);
    _socket->flush();
}

void NetworkManager::processData() {
    while (true) {
        int endPos = _buffer.indexOf('\n');
        if (endPos == -1) break;

        QByteArray messageData = _buffer.left(endPos);
        _buffer.remove(0, endPos + 1);

        QJsonDocument doc = QJsonDocument::fromJson(messageData);
        if (!doc.isNull()) {
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();

            if (type == "ready") {
                _playerName = obj["playerName"].toString();
                emit playerReady(_playerName);
            } else if (type == "startGame") {
                emit gameStarted();
            } else if (type == "move") {
                emit enemyMove(obj);
            } else if (type == "gameEnded") {
                emit gameEnded(obj["winner"].toString());
            } else {
                emit messageReceived(obj);
            }
        }
    }
}
