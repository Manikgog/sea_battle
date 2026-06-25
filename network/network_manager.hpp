#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

class NetworkManager : public QObject {
    Q_OBJECT

public:
    enum class Role {
        Server,
        Client,
        None
    };

    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    bool startServer(int port = 12345);
    bool connectToServer(const QString& address, int port = 12345);
    void disconnect();

    void sendMessage(const QJsonObject& message);
    bool isConnected() const;
    Role getRole() const { return _role; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void messageReceived(const QJsonObject& message);
    void playerReady(const QString& playerName);
    void gameStarted();
    void enemyMove(const QJsonObject& move);
    void gameEnded(const QString& winner);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onNewConnection();

private:
    void sendJsonMessage(const QJsonObject& message);
    void processData();

    Role _role = Role::None;
    std::unique_ptr<QTcpServer> _server;
    std::unique_ptr<QTcpSocket> _socket;
    QByteArray _buffer;
    bool _isConnected = false;
    QString _playerName;
};

#endif // NETWORK_MANAGER_HPP
