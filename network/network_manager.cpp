#include "network_manager.hpp"
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , _userName("Абонент") {
    setConnectionStatus("Отключен", false);
}

void NetworkManager::setIsServer(bool isServer) {
    if (_isServer != isServer) {
        if (_isConnected) {
            disconnectFromHost();
        }
        _isServer = isServer;
        emit serverModeChanged();
    }
}

QString NetworkManager::localAddress() const {
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : addresses) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol &&
            addr != QHostAddress::LocalHost &&
            addr != QHostAddress::LocalHostIPv6) {
            return addr.toString();
        }
    }
    return "127.0.0.1";
}

bool NetworkManager::isValidPort(int port) {
    return port > 0 && port <= 65535;
}

bool NetworkManager::isValidAddress(const QString &address) {
    if (address.isEmpty()) {
        return false;
    }
    QString trimmed = address.trimmed();
    static QRegularExpression ipv4Regex(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
        );
    return ipv4Regex.match(trimmed).hasMatch();
}

void NetworkManager::startServer(int port) {
    if (!isValidPort(port)) {
        QString errorMsg = "Неверный порт: " + QString::number(port) + ". Порт должен быть в диапазоне 1-65535.";
        setConnectionStatus("Ошибка: " + errorMsg, false);
        emit errorOccurred(errorMsg);
        qDebug() << errorMsg;
        return;
    }

    if (_isConnected) {
        disconnectFromHost();
    }

    // Очищаем предыдущий сервер
    if (_server) {
        _server->close();
        delete _server;
        _server = nullptr;
    }

    _port = port;
    _server = new QWebSocketServer("Messenger Server",
                                   QWebSocketServer::NonSecureMode,
                                   this);

    connect(_server, &QWebSocketServer::newConnection,
            this, &NetworkManager::onNewConnection);
    connect(_server, &QWebSocketServer::serverError,
            this, &NetworkManager::onServerError);

    if (_server->listen(QHostAddress::Any, port)) {
        setConnectionStatus("Сервер запущен на порту " + QString::number(port), true);
        qDebug() << "Server started on port" << port;
        emit serverModeChanged();
    } else {
        setConnectionStatus("Ошибка запуска сервера", false);
        emit errorOccurred("Не удалось запустить сервер на порту " + QString::number(port));
        qDebug() << "Failed to start server on port" << port;
    }
}

void NetworkManager::connectToHost(const QString &address, int port) {
    qDebug() << __FUNCTION__ << "Connecting to" << address << ":" << port;

    if (!isValidAddress(address)) {
        QString errorMsg = "Неверный IP-адрес: " + address + ". Ожидается формат xxx.xxx.xxx.xxx";
        setConnectionStatus("Ошибка: " + errorMsg, false);
        emit errorOccurred(errorMsg);
        qDebug() << errorMsg;
        return;
    }

    if (!isValidPort(port)) {
        QString errorMsg = "Неверный порт: " + QString::number(port) + ". Порт должен быть в диапазоне 1-65535.";
        setConnectionStatus("Ошибка: " + errorMsg, false);
        emit errorOccurred(errorMsg);
        qDebug() << errorMsg;
        return;
    }

    // Отключаемся от текущего соединения если оно есть
    if (_isConnected) {
        disconnectFromHost();
    }

    // Безопасно удаляем старый сокет
    if (_clientSocket) {
        // Отключаем все сигналы от старого сокета
        disconnect(_clientSocket, nullptr, this, nullptr);
        _clientSocket->close();
        _clientSocket->deleteLater();
        _clientSocket = nullptr;
    }

    _port = port;
    _clientSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(_clientSocket, &QWebSocket::connected,
            this, &NetworkManager::onConnected);
    connect(_clientSocket, &QWebSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(_clientSocket, &QWebSocket::textMessageReceived,
            this, &NetworkManager::onTextMessageReceived);
    connect(_clientSocket, &QWebSocket::errorOccurred,
            this, &NetworkManager::onSocketError);

    setConnectionStatus("Подключение к " + address + ":" + QString::number(port) + "...", false);
    _clientSocket->open(QUrl("ws://" + address + ":" + QString::number(port)));
}

void NetworkManager::disconnectFromHost() {
    // Останавливаем сервер если он запущен
    if (_server) {
        _server->close();
        delete _server;
        _server = nullptr;
    }

    // Отключаем и удаляем клиентский сокет
    if (_clientSocket) {
        disconnect(_clientSocket, nullptr, this, nullptr);
        _clientSocket->close();
        _clientSocket->deleteLater();
        _clientSocket = nullptr;
    }

    // Закрываем все клиентские соединения
    for (QWebSocket *client : _clients) {
        client->close();
        client->deleteLater();
    }
    _clients.clear();
    _clientNames.clear();

    setConnectionStatus("Отключен", false);
}

void NetworkManager::sendMessage(const QString &sender, const QString &text) {
    QJsonObject json;
    json["type"] = "message";
    json["sender"] = sender;
    json["text"] = text;
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    if (_isServer) {
        sendToAll(json);
    } else if (_clientSocket && _isConnected) {
        sendToClient(_clientSocket, json);
    }
}

void NetworkManager::setUserName(const QString &name) {
    if (_userName != name) {
        _userName = name;
        if (_isConnected) {
            QJsonObject json;
            json["type"] = "user_update";
            json["userName"] = name;
            if (_isServer) {
                sendToAll(json);
            } else if (_clientSocket) {
                sendToClient(_clientSocket, json);
            }
        }
    }
}

void NetworkManager::onNewConnection() {
    QWebSocket *client = _server->nextPendingConnection();
    if (!client) {
        return;
    }

    connect(client, &QWebSocket::textMessageReceived,
            this, &NetworkManager::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(client, &QWebSocket::errorOccurred,
            this, &NetworkManager::onSocketError);

    _clients.append(client);
    _clientNames[client] = "Гость";

    qDebug() << "New client connected. Total clients:" << _clients.size();
    sendSystemMessage("Пользователь подключился к чату");
    broadcastUserList();
}

void NetworkManager::onConnected() {
    if (_clientSocket) {
        setConnectionStatus("Подключен к серверу", true);

        QJsonObject json;
        json["type"] = "join";
        json["userName"] = _userName;
        sendToClient(_clientSocket, json);

        qDebug() << "Connected to server as" << _userName;
    }
}

void NetworkManager::onDisconnected() {
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket) {
        QString userName = _clientNames.value(socket, "Гость");
        _clients.removeAll(socket);
        _clientNames.remove(socket);
        socket->deleteLater();

        if (_isServer) {
            sendSystemMessage(userName + " покинул чат");
            broadcastUserList();
            qDebug() << "Client disconnected. Remaining clients:" << _clients.size();
        }
    } else if (_clientSocket) {
        setConnectionStatus("Отключен от сервера", false);
        // Не удаляем _clientSocket здесь, это сделает onSocketError или disconnectFromHost
    }
}

void NetworkManager::onTextMessageReceived(const QString &message) {
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull()) {
        qDebug() << "Invalid JSON received:" << message;
        return;
    }

    QJsonObject json = doc.object();
    processJsonMessage(json, socket);
}

void NetworkManager::onSocketError(QAbstractSocket::SocketError error) {
    QString errorMsg;
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        errorMsg = "Соединение отклонено";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMsg = "Удаленный хост закрыл соединение";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMsg = "Хост не найден";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorMsg = "Таймаут соединения";
        break;
    default:
        errorMsg = "Ошибка сокета: " + QString::number(error);
        break;
    }

    setConnectionStatus("Ошибка: " + errorMsg, false);
    emit errorOccurred(errorMsg);
    qDebug() << "Socket error:" << errorMsg;

    // Безопасно очищаем сокет при ошибке
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket) {
        // Если это клиентский сокет
        if (socket == _clientSocket) {
            disconnect(_clientSocket, nullptr, this, nullptr);
            _clientSocket->close();
            _clientSocket->deleteLater();
            _clientSocket = nullptr;
        } else {
            // Если это клиент на сервере
            _clients.removeAll(socket);
            _clientNames.remove(socket);
            socket->deleteLater();
        }
    }
}

void NetworkManager::onServerError() {
    if (_server) {
        QString errorMsg = "Ошибка сервера";
        setConnectionStatus(errorMsg, false);
        emit errorOccurred(errorMsg);
        qDebug() << "Server error:" << errorMsg;

        // Безопасно очищаем сервер при ошибке
        _server->close();
        delete _server;
        _server = nullptr;
    }
}

void NetworkManager::setConnectionStatus(const QString &status, bool connected) {
    if (_connectionStatus != status || _isConnected != connected) {
        _connectionStatus = status;
        _isConnected = connected;
        emit connectionStatusChanged();
    }
}

void NetworkManager::broadcastUserList() {
    QStringList userList;
    for (const QString &name : _clientNames.values()) {
        if (!name.isEmpty()) {
            userList.append(name);
        }
    }

    QJsonObject json;
    json["type"] = "user_list";
    json["users"] = QJsonArray::fromStringList(userList);

    sendToAll(json);
}

void NetworkManager::sendSystemMessage(const QString &message) {
    QJsonObject json;
    json["type"] = "system";
    json["text"] = message;
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    sendToAll(json);
}

void NetworkManager::processJsonMessage(const QJsonObject &json, QWebSocket *sender) {
    QString type = json["type"].toString();

    if (type == "join") {
        QString userName = json["userName"].toString();
        if (userName.isEmpty()) {
            userName = "Гость";
        }

        if (_isServer && sender) {
            _clientNames[sender] = userName;
            sendSystemMessage(userName + " присоединился к чату");
            broadcastUserList();
            qDebug() << "User joined:" << userName;
        }
    } else if (type == "message") {
        QString senderName = json["sender"].toString();
        QString text = json["text"].toString();

        if (!senderName.isEmpty() && !text.isEmpty()) {
            emit messageReceived(senderName, text);

            if (_isServer) {
                sendToAll(json, sender);
            }
        }
    } else if (type == "user_update") {
        QString newName = json["userName"].toString();
        if (_isServer && sender && !newName.isEmpty()) {
            QString oldName = _clientNames.value(sender);
            _clientNames[sender] = newName;
            sendSystemMessage(oldName + " теперь известен как " + newName);
            broadcastUserList();
        }
    }
}

void NetworkManager::sendToAll(const QJsonObject &json, QWebSocket *exclude) {
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    for (QWebSocket *client : _clients) {
        if (client != exclude) {
            client->sendTextMessage(QString::fromUtf8(data));
        }
    }

    if (!_isServer && _clientSocket && _clientSocket != exclude) {
        _clientSocket->sendTextMessage(QString::fromUtf8(data));
    }
}

void NetworkManager::sendToClient(QWebSocket *client, const QJsonObject &json) {
    if (!client) {
        return;
    }
    QJsonDocument doc(json);
    client->sendTextMessage(QString::fromUtf8(doc.toJson()));
}
