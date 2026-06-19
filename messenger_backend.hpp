#ifndef MESSENGERBACKEND_H
#define MESSENGERBACKEND_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>
#include "network_manager.hpp"
#include "model_adapter.hpp"

struct Message {
    QString sender;
    QString text;
    QDateTime timestamp;
};

class MessengerBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentUser READ currentUser WRITE setCurrentUser NOTIFY currentUserChanged)
    Q_PROPERTY(QStringList messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(QStringList messageSenders READ messageSenders NOTIFY messagesChanged)
    Q_PROPERTY(QStringList messageTimes READ messageTimes NOTIFY messagesChanged)
    Q_PROPERTY(NetworkManager* networkManager READ networkManager CONSTANT)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(ModelAdapter* gameModel READ gameModel CONSTANT)

  public:
    explicit MessengerBackend(QObject *parent = nullptr);

    QString currentUser() const;
    void setCurrentUser(const QString &user);

    QStringList messages() const;
    QStringList messageSenders() const;
    QStringList messageTimes() const;
    NetworkManager* networkManager() const { return m_networkManager; }
    bool isConnected() const { return m_networkManager->isConnected(); }
    Q_INVOKABLE ModelAdapter* gameModel() const { return m_gameModel; }

  public slots:
    void sendMessage(const QString &text);
    void receiveMessage(const QString &sender, const QString &text);

  signals:
    void currentUserChanged();
    void messagesChanged();
    void newMessageReceived(const QString &sender, const QString &text);
    void isConnectedChanged();

  private slots:
    void onMessageReceived(const QString &sender, const QString &text);

  private:
    QString         m_currentUser;
    QList<Message>  m_messages;
    NetworkManager  *m_networkManager;
    ModelAdapter    *m_gameModel;
};

#endif // MESSENGERBACKEND_H
