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





void NetworkManager::startServer(int port) {
    if (_isConnected) {
        disconnectFromHost();
    }

    if (_server) {
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
        qDebug() << "Local address:" << localAddress();
        emit serverModeChanged();
    } else {
        setConnectionStatus("Ошибка запуска сервера", false);
        emit errorOccurred("Не удалось запустить сервер на порту " + QString::number(port));
    }
}





void NetworkManager::connectToHost(const QString &address, int port) {
    if (_isConnected) {
        disconnectFromHost();
    }

    if (_clientSocket) {
        delete _clientSocket;
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
    if (_server) {
        _server->close();
        _server->deleteLater();
        _server = nullptr;
    }

    if (_clientSocket) {
        _clientSocket->close();
        _clientSocket->deleteLater();
        _clientSocket = nullptr;
    }

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
            // Отправляем обновление имени всем
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

           // Отправляем приветственное сообщение
    sendSystemMessage("Пользователь подключился к чату");
    broadcastUserList();
}





void NetworkManager::onConnected() {
    if (_clientSocket) {
        setConnectionStatus("Подключен к серверу", true);

               // Отправляем информацию о пользователе
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
        _clientSocket->deleteLater();
        _clientSocket = nullptr;
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
}





void NetworkManager::onServerError() {
    if (_server) {
        QString errorMsg = "Ошибка сервера";
        setConnectionStatus(errorMsg, false);
        emit errorOccurred(errorMsg);
        qDebug() << "Server error:" << errorMsg;
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

                   // Если мы сервер, пересылаем сообщение всем клиентам
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

           // Если мы клиент, отправляем на сервер
    if (!_isServer && _clientSocket && _clientSocket != exclude) {
        _clientSocket->sendTextMessage(QString::fromUtf8(data));
    }
}





void NetworkManager::sendToClient(QWebSocket *client, const QJsonObject &json) {
    QJsonDocument doc(json);
    client->sendTextMessage(QString::fromUtf8(doc.toJson()));
}
