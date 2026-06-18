#include "network_manager.h"
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
      , m_userName("Абонент")
{
    setConnectionStatus("Отключен", false);
}

void NetworkManager::setIsServer(bool isServer)
{
    if (m_isServer != isServer) {
        if (m_isConnected) {
            disconnectFromHost();
        }
        m_isServer = isServer;
        emit serverModeChanged();
    }
}

QString NetworkManager::localAddress() const
{
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

void NetworkManager::startServer(int port)
{
    if (m_isConnected) {
        disconnectFromHost();
    }

    if (m_server) {
        delete m_server;
        m_server = nullptr;
    }

    m_port = port;
    m_server = new QWebSocketServer("Messenger Server",
                                    QWebSocketServer::NonSecureMode,
                                    this);

    connect(m_server, &QWebSocketServer::newConnection,
            this, &NetworkManager::onNewConnection);
    connect(m_server, &QWebSocketServer::serverError,
            this, &NetworkManager::onServerError);

    if (m_server->listen(QHostAddress::Any, port)) {
        setConnectionStatus("Сервер запущен на порту " + QString::number(port), true);
        qDebug() << "Server started on port" << port;
        qDebug() << "Local address:" << localAddress();
        emit serverModeChanged();
    } else {
        setConnectionStatus("Ошибка запуска сервера", false);
        emit errorOccurred("Не удалось запустить сервер на порту " + QString::number(port));
    }
}

void NetworkManager::connectToHost(const QString &address, int port)
{
    if (m_isConnected) {
        disconnectFromHost();
    }

    if (m_clientSocket) {
        delete m_clientSocket;
        m_clientSocket = nullptr;
    }

    m_port = port;
    m_clientSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(m_clientSocket, &QWebSocket::connected,
            this, &NetworkManager::onConnected);
    connect(m_clientSocket, &QWebSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(m_clientSocket, &QWebSocket::textMessageReceived,
            this, &NetworkManager::onTextMessageReceived);
    connect(m_clientSocket, &QWebSocket::errorOccurred,
            this, &NetworkManager::onSocketError);

    setConnectionStatus("Подключение к " + address + ":" + QString::number(port) + "...", false);
    m_clientSocket->open(QUrl("ws://" + address + ":" + QString::number(port)));
}

void NetworkManager::disconnectFromHost()
{
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

    if (m_clientSocket) {
        m_clientSocket->close();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }

    for (QWebSocket *client : m_clients) {
        client->close();
        client->deleteLater();
    }
    m_clients.clear();
    m_clientNames.clear();

    setConnectionStatus("Отключен", false);
}

void NetworkManager::sendMessage(const QString &sender, const QString &text)
{
    QJsonObject json;
    json["type"] = "message";
    json["sender"] = sender;
    json["text"] = text;
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    if (m_isServer) {
        sendToAll(json);
    } else if (m_clientSocket && m_isConnected) {
        sendToClient(m_clientSocket, json);
    }
}

void NetworkManager::setUserName(const QString &name)
{
    if (m_userName != name) {
        m_userName = name;
        if (m_isConnected) {
            // Отправляем обновление имени всем
            QJsonObject json;
            json["type"] = "user_update";
            json["userName"] = name;
            if (m_isServer) {
                sendToAll(json);
            } else if (m_clientSocket) {
                sendToClient(m_clientSocket, json);
            }
        }
    }
}

void NetworkManager::onNewConnection()
{
    QWebSocket *client = m_server->nextPendingConnection();
    if (!client) {
        return;
    }

    connect(client, &QWebSocket::textMessageReceived,
            this, &NetworkManager::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(client, &QWebSocket::errorOccurred,
            this, &NetworkManager::onSocketError);

    m_clients.append(client);
    m_clientNames[client] = "Гость";

    qDebug() << "New client connected. Total clients:" << m_clients.size();

           // Отправляем приветственное сообщение
    sendSystemMessage("Пользователь подключился к чату");
    broadcastUserList();
}

void NetworkManager::onConnected()
{
    if (m_clientSocket) {
        setConnectionStatus("Подключен к серверу", true);

               // Отправляем информацию о пользователе
        QJsonObject json;
        json["type"] = "join";
        json["userName"] = m_userName;
        sendToClient(m_clientSocket, json);

        qDebug() << "Connected to server as" << m_userName;
    }
}

void NetworkManager::onDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket) {
        QString userName = m_clientNames.value(socket, "Гость");
        m_clients.removeAll(socket);
        m_clientNames.remove(socket);
        socket->deleteLater();

        if (m_isServer) {
            sendSystemMessage(userName + " покинул чат");
            broadcastUserList();
            qDebug() << "Client disconnected. Remaining clients:" << m_clients.size();
        }
    } else if (m_clientSocket) {
        setConnectionStatus("Отключен от сервера", false);
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
}

void NetworkManager::onTextMessageReceived(const QString &message)
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull()) {
        qDebug() << "Invalid JSON received:" << message;
        return;
    }

    QJsonObject json = doc.object();
    processJsonMessage(json, socket);
}

void NetworkManager::onSocketError(QAbstractSocket::SocketError error)
{
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

void NetworkManager::onServerError()
{
    if (m_server) {
        QString errorMsg = "Ошибка сервера";
        setConnectionStatus(errorMsg, false);
        emit errorOccurred(errorMsg);
        qDebug() << "Server error:" << errorMsg;
    }
}

void NetworkManager::setConnectionStatus(const QString &status, bool connected)
{
    if (m_connectionStatus != status || m_isConnected != connected) {
        m_connectionStatus = status;
        m_isConnected = connected;
        emit connectionStatusChanged();
    }
}

void NetworkManager::broadcastUserList()
{
    QStringList userList;
    for (const QString &name : m_clientNames.values()) {
        if (!name.isEmpty()) {
            userList.append(name);
        }
    }

    QJsonObject json;
    json["type"] = "user_list";
    json["users"] = QJsonArray::fromStringList(userList);

    sendToAll(json);
}

void NetworkManager::sendSystemMessage(const QString &message)
{
    QJsonObject json;
    json["type"] = "system";
    json["text"] = message;
    json["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    sendToAll(json);
}

void NetworkManager::processJsonMessage(const QJsonObject &json, QWebSocket *sender)
{
    QString type = json["type"].toString();

    if (type == "join") {
        QString userName = json["userName"].toString();
        if (userName.isEmpty()) {
            userName = "Гость";
        }

        if (m_isServer && sender) {
            m_clientNames[sender] = userName;
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
            if (m_isServer) {
                sendToAll(json, sender);
            }
        }
    } else if (type == "user_update") {
        QString newName = json["userName"].toString();
        if (m_isServer && sender && !newName.isEmpty()) {
            QString oldName = m_clientNames.value(sender);
            m_clientNames[sender] = newName;
            sendSystemMessage(oldName + " теперь известен как " + newName);
            broadcastUserList();
        }
    }
}

void NetworkManager::sendToAll(const QJsonObject &json, QWebSocket *exclude)
{
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    for (QWebSocket *client : m_clients) {
        if (client != exclude) {
            client->sendTextMessage(QString::fromUtf8(data));
        }
    }

           // Если мы клиент, отправляем на сервер
    if (!m_isServer && m_clientSocket && m_clientSocket != exclude) {
        m_clientSocket->sendTextMessage(QString::fromUtf8(data));
    }
}

void NetworkManager::sendToClient(QWebSocket *client, const QJsonObject &json)
{
    QJsonDocument doc(json);
    client->sendTextMessage(QString::fromUtf8(doc.toJson()));
}
