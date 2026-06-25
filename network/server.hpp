#ifndef SERVER_HPP
#define SERVER_HPP

#include <QObject>
#include <memory>
#include "network_manager.hpp"

class Server : public QObject {
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);

    bool start(int port = 12345);
    void stop();
    bool isRunning() const;

    void sendMove(const QJsonObject& move);
    void sendGameEnded(const QString& winner);

signals:
    void playerConnected(const QString& playerName);
    void playerDisconnected();
    void clientReady();
    void moveReceived(const QJsonObject& move);
    void error(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onMessageReceived(const QJsonObject& message);

private:
    std::unique_ptr<NetworkManager> _networkManager;
    QString _playerName;
};

#endif // SERVER_HPP
