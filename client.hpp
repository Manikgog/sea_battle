#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <QObject>
#include <memory>
#include "network_manager.hpp"

class Client : public QObject {
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);

    bool connectToServer(const QString& address, int port = 12345);
    void disconnect();
    bool isConnected() const;

    void sendReady(const QString& playerName);
    void sendMove(const QJsonObject& move);

signals:
    void connected();
    void disconnected();
    void gameStarted();
    void moveReceived(const QJsonObject& move);
    void gameEnded(const QString& winner);
    void error(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onMessageReceived(const QJsonObject& message);

private:
    std::unique_ptr<NetworkManager> _networkManager;
};

#endif // CLIENT_HPP
