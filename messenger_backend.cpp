#include "messenger_backend.h"
#include <QDebug>
#include <QTimer>

MessengerBackend::MessengerBackend(QObject *parent)
    : QObject(parent)
      , m_currentUser("Абонент")
      , m_networkManager(new NetworkManager(this))
{
    connect(m_networkManager, &NetworkManager::messageReceived,
            this, &MessengerBackend::onMessageReceived);
    connect(m_networkManager, &NetworkManager::connectionStatusChanged,
            this, &MessengerBackend::isConnectedChanged);

           // Добавляем системное сообщение
    receiveMessage("Система", "Добро пожаловать в мессенджер! Настройте подключение.");
}

QString MessengerBackend::currentUser() const
{
    return m_currentUser;
}

void MessengerBackend::setCurrentUser(const QString &user)
{
    if (m_currentUser != user) {
        m_currentUser = user;
        emit currentUserChanged();
        m_networkManager->setUserName(user);
    }
}

QStringList MessengerBackend::messages() const
{
    QStringList result;
    for (const Message &msg : m_messages) {
        result.append(msg.text);
    }
    return result;
}

QStringList MessengerBackend::messageSenders() const
{
    QStringList result;
    for (const Message &msg : m_messages) {
        result.append(msg.sender);
    }
    return result;
}

QStringList MessengerBackend::messageTimes() const
{
    QStringList result;
    for (const Message &msg : m_messages) {
        result.append(msg.timestamp.toString("hh:mm:ss"));
    }
    return result;
}

void MessengerBackend::sendMessage(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }

    if (!m_networkManager->isConnected()) {
        receiveMessage("Система", "Нет подключения к сети. Сообщение не отправлено.");
        return;
    }

    Message msg;
    msg.sender = m_currentUser;
    msg.text = text.trimmed();
    msg.timestamp = QDateTime::currentDateTime();

    m_messages.append(msg);
    emit messagesChanged();

           // Отправляем по сети
    m_networkManager->sendMessage(m_currentUser, msg.text);

    qDebug() << "Отправлено сообщение от" << m_currentUser << ":" << text;
}

void MessengerBackend::receiveMessage(const QString &sender, const QString &text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }

    Message msg;
    msg.sender = sender;
    msg.text = text.trimmed();
    msg.timestamp = QDateTime::currentDateTime();

    m_messages.append(msg);
    emit messagesChanged();
    emit newMessageReceived(sender, text);

    qDebug() << "Получено сообщение от" << sender << ":" << text;
}

void MessengerBackend::onMessageReceived(const QString &sender, const QString &text)
{
    receiveMessage(sender, text);
}
