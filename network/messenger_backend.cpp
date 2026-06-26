#include "messenger_backend.hpp"
#include <QDebug>
#include <QTimer>

MessengerBackend::MessengerBackend(QObject *parent)
    : QObject(parent)
      , _currentUser("Абонент")
      , _networkManager(new NetworkManager(this))
      , _gameModel(new ModelAdapterNetwork(this))
{
    connect(_networkManager, &NetworkManager::messageReceived,
            this, &MessengerBackend::onMessageReceived);
    connect(_networkManager, &NetworkManager::connectionStatusChanged,
            this, &MessengerBackend::isConnectedChanged);

    receiveMessage("Система", "Добро пожаловать в игру Морской бой! Настройте подключение.");
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

    if (msg.text != "player_ready" && msg.text != "game_start" && !msg.text.startsWith("shot ") && !msg.text.startsWith("result ")) {
        _messages.append(msg);
        emit messagesChanged();
    }

    _networkManager->sendMessage(_currentUser, msg.text);
    qDebug() << "Отправлено сообщение от" << _currentUser << ":" << text;
}









void MessengerBackend::shotMessage(const QString &index)
{
    if (index.trimmed().isEmpty()) {
        return;
    }

    if (!_networkManager->isConnected()) {
        receiveMessage("Система", "Нет подключения к сети. Сообщение не отправлено.");
        return;
    }

    QString msgText = "shot " + index.trimmed();
    _networkManager->sendMessage(_currentUser, msgText);
    qDebug() << "Выстрел от" << _currentUser << ":" << msgText;
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

    QStringList messageParts = msg.text.split(" ");

           // Обработка выстрела
    if (!messageParts.empty() && messageParts[0] == "shot" && messageParts.size() == 2) {
        qDebug() << "Получен выстрел от" << sender << "индекс:" << messageParts[1];

        bool ok;
        int index = messageParts[1].toInt(&ok);
        if (ok) {
            // Применяем выстрел к полю игрока
            std::optional<ShotResult> result = _gameModel->setShot(messageParts[1]);
            if (result.has_value()) {
                // Отправляем результат обратно
                sendShotResultMessage(result.value()._result, result.value()._index);
                if(_gameModel->isGameOver()) {
                    sendMessage("game_over");
                    _gameModel->resetGame();
                    updateGameButtonState();
                }
            }
        }
        return;
    }

           // Обработка результата выстрела
    if (!messageParts.empty() && messageParts[0] == "result" && messageParts.size() == 3) {
        qDebug() << "Получен результат:" << messageParts[1] << "индекс:" << messageParts[2];
        _gameModel->setResult(messageParts[1], messageParts[2]);
        int res = messageParts[1].toInt();
        if (res == Result::Miss) {
            // Промах - ход переходит к противнику
            _gameModel->setMyTurn(false);
            _gameModel->setPlayerFieldBlocked(true);
        } else if(res == Result::Destroyed) {
            _gameModel->increaseEnemyShipsDestroyed();
            _gameModel->setMyTurn(true);
            _gameModel->setPlayerFieldBlocked(false);
        } else if(res == Result::Wounded) {
            // Попадание - ход остается у нас
            _gameModel->setMyTurn(true);
            _gameModel->setPlayerFieldBlocked(false);
        }
        _gameModel->updateGameStatus();
        return;
    }

           // Обработка команд
    if (text == "player_ready") {
        qDebug() << "Получен player_ready от" << sender;
        emit newMessageReceived(sender, text);
        emit updateGameButtonState();
        _messages.append(msg);
        emit messagesChanged();
        return;
    } else if (text == "game_start") {
        _gameModel->setMyTurn(false);
        _gameModel->setGameStarted(true);
        emit newMessageReceived(sender, text);
        emit updateGameButtonState();
        _messages.append(msg);
        emit messagesChanged();
        _gameModel->setEnemyShipsDestroyed(0);
        return;
    } else if(text == "game_over") {
        _gameModel->gameWon();
        _gameModel->setEnemyShipsDestroyed(0);
        return;
    }

           // Обычное сообщение
    _messages.append(msg);
    emit messagesChanged();
    emit newMessageReceived(sender, text);
    qDebug() << "Получено сообщение от" << sender << ":" << text;
}








void MessengerBackend::onMessageReceived(const QString &sender, const QString &text)
{
    receiveMessage(sender, text);
}








void MessengerBackend::sendShotResultMessage(Result shotResult, int index)
{
    QString messageText = "result " + QString::number(shotResult) + " " + QString::number(index);
    if (_networkManager->isConnected()) {
        _networkManager->sendMessage(_currentUser, messageText);
        qDebug() << "Отправлен результат:" << messageText;
    }
}
