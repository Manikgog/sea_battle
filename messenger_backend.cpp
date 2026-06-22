#include "messenger_backend.hpp"
#include <QDebug>
#include <QTimer>

MessengerBackend::MessengerBackend(QObject *parent)
    : QObject(parent)
      , _currentUser("Абонент")
      , _networkManager(new NetworkManager(this))
      , _gameModel(new ModelAdapter(this))
{
    connect(_networkManager, &NetworkManager::messageReceived,
            this, &MessengerBackend::onMessageReceived);
    connect(_networkManager, &NetworkManager::connectionStatusChanged,
            this, &MessengerBackend::isConnectedChanged);

           // Добавляем системное сообщение
    receiveMessage("Система", "Добро пожаловать в мессенджер! Настройте подключение.");
}

QString MessengerBackend::currentUser() const
{
    return _currentUser;
}

void MessengerBackend::setCurrentUser(const QString &user)
{
    if (_currentUser != user) {
        _currentUser = user;
        emit currentUserChanged();
        _networkManager->setUserName(user);
    }
}

QStringList MessengerBackend::messages() const
{
    QStringList result;
    for (const Message &msg : _messages) {
        result.append(msg.text);
    }
    return result;
}

QStringList MessengerBackend::messageSenders() const
{
    QStringList result;
    for (const Message &msg : _messages) {
        result.append(msg.sender);
    }
    return result;
}

QStringList MessengerBackend::messageTimes() const
{
    QStringList result;
    for (const Message &msg : _messages) {
        result.append(msg.timestamp.toString("hh:mm:ss"));
    }
    return result;
}


void MessengerBackend::sendMessage(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }

    if (!_networkManager->isConnected()) {
        receiveMessage("Система", "Нет подключения к сети. Сообщение не отправлено.");
        return;
    }

    Message msg;
    msg.sender = _currentUser;
    msg.text = text.trimmed();
    msg.timestamp = QDateTime::currentDateTime();

    _messages.append(msg);
    emit messagesChanged();

    // Отправляем по сети
    _networkManager->sendMessage(_currentUser, msg.text);

    qDebug() << "Отправлено сообщение от" << _currentUser << ":" << text;
}



/**
 * @brief MessengerBackend::shotMessage метод для отправки сообщения с индексом клетки
 * @param index
 */
void MessengerBackend::shotMessage(const QString &index)
{
    if (index.trimmed().isEmpty()) {
        return;
    }

    if (!_networkManager->isConnected()) {
        receiveMessage("Система", "Нет подключения к сети. Сообщение не отправлено.");
        return;
    }

    Message msg;
    msg.sender = _currentUser;
    msg.text = "shot " + index.trimmed();
    msg.timestamp = QDateTime::currentDateTime();

    //m_messages.append(msg);
    //emit messagesChanged();

    // Отправляем по сети
    _networkManager->sendMessage(_currentUser, msg.text);

    qDebug() << "Выстрел от" << _currentUser << ":" << msg.text;
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

    QStringList message_parts = msg.text.split(" ");
    int counter = 1;
    for(const QString& s : message_parts) {
        qDebug() << "part" << counter++ << s;
    }
    if(!message_parts.empty()) {
        if(message_parts[0] == "shot" && message_parts.size() == 2) {
            qDebug() << message_parts[0] << message_parts[1];
            std::optional<ShotResult> result_optional = _gameModel->setShot(message_parts[1]);
            Result result = Result::Miss;
            if(result_optional.has_value()) {
                result = result_optional.value()._result;
                int index = result_optional.value()._index;
                sendShotResultMessage(result, index);
            } else {
                return;
            }
            return;
        }
        if(message_parts[0] == "result" && message_parts.size() == 3) {
            qDebug() << message_parts[0] << "result =>" << message_parts[1] << "index =>" << message_parts[2];
            _gameModel->setResult(message_parts[1], message_parts[2]);
            return;
        }
    }

    if (text == "player_ready") {
        // Клиент готов - просто показываем в чате
        // QML обработает это через onNewMessageReceived
    } else if (text == "game_start") {
        // Сервер начал игру - QML обработает это
    }

    _messages.append(msg);
    emit messagesChanged();
    emit newMessageReceived(sender, text);

    qDebug() << "Получено сообщение от" << sender << ":" << text;
}

void MessengerBackend::onMessageReceived(const QString &sender, const QString &text)
{
    receiveMessage(sender, text);
}

void MessengerBackend::sendShotResultMessage(Result shot_result, int index)
{
    QString message_text = "result " + QString::number(shot_result) + " " + QString::number(index);
    sendMessage(message_text);
}
