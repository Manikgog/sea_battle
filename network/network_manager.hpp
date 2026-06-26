#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QAbstractSocket>
#include <QHash>
#include <QJsonArray>
#include <QDateTime>

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool isServer READ isServer WRITE setIsServer NOTIFY serverModeChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString localAddress READ localAddress NOTIFY serverModeChanged)
    Q_PROPERTY(int port READ port NOTIFY serverModeChanged)

  public:
    explicit NetworkManager(QObject *parent = nullptr);

    bool isConnected() const { return _isConnected; }
    bool isServer() const { return _isServer; }
    void setIsServer(bool isServer);
    QString connectionStatus() const { return _connectionStatus; }
    QString localAddress() const;
    int port() const { return _port; }

  public slots:
    void startServer(int port = 8080);
    void connectToHost(const QString &address, int port = 8080);
    void disconnectFromHost();
    void sendMessage(const QString &sender, const QString &text);
    void setUserName(const QString &name);

  signals:
    void connectionStatusChanged();
    void serverModeChanged();
    void messageReceived(const QString &sender, const QString &text);
    void userConnected(const QString &userName);
    void userDisconnected(const QString &userName);
    void errorOccurred(const QString &error);

  private slots:
    void onNewConnection();
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onSocketError(QAbstractSocket::SocketError error);
    void onServerError(); // Изменено: без параметров

  private:
    void setConnectionStatus(const QString &status, bool connected);
    void broadcastUserList();
    void sendSystemMessage(const QString &message);
    void processJsonMessage(const QJsonObject &json, QWebSocket *sender);
    void sendToAll(const QJsonObject &json, QWebSocket *exclude = nullptr);
    void sendToClient(QWebSocket *client, const QJsonObject &json);

    QWebSocketServer                *_server = nullptr;
    QWebSocket                      *_clientSocket = nullptr;
    QList<QWebSocket *>             _clients;
    QString                         _userName;
    QString                         _connectionStatus;
    bool                            _isConnected = false;
    bool                            _isServer = false;
    int                             _port = 8080;
    QHash<QWebSocket *, QString>    _clientNames;
};

#endif // NETWORKMANAGER_H
