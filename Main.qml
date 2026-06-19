import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import MessengerApp 1.0

ApplicationWindow {
    id: root
    width: 600
    height: 900  // Увеличил высоту для размещения полей
    visible: true
    title: "Морской бой"

    property var model: backend.gameModel  // Исправлено: используем модель из бэкенда
    property string currentUser: backend.currentUser
    property bool isConnected: backend.isConnected

    // Функция для обновления конкретной клетки поля бота
    function updateBotCell(index) {
        var item = botGridRepeater.itemAt(index);
        if (item) {
            item.color = item.getCellColor(index);
            var textItem = item.children[0];
            if (textItem) {
                textItem.text = item.getCellMark(index);
                textItem.color = item.getMarkColor(index);
            }
        }
    }

    // Функция для обновления конкретной клетки поля игрока
    function updatePlayerCell(index) {
        var item = playerGridRepeater.itemAt(index);
        if (item) {
            item.color = item.getCellColor(index);
            var textItem = item.children[0];
            if (textItem) {
                textItem.text = item.getCellMark(index);
                textItem.color = item.getMarkColor(index);
            }
        }
    }

    // Функция для обновления всех клеток
    function updateAllCells() {
        for (var i = 0; i < 100; i++) {
            updatePlayerCell(i);
        }
    }

    MessengerBackend {
        id: backend
        currentUser: "Абонент"
        onNewMessageReceived: {
            messageListView.positionViewAtEnd()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Заголовок с управлением подключением
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#2196F3"
            radius: 10

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "📱 " + backend.currentUser
                        color: "white"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        Layout.preferredWidth: 12
                        Layout.preferredHeight: 12
                        radius: 6
                        color: backend.isConnected ? "#4CAF50" : "#f44336"

                        Behavior on color {
                            ColorAnimation { duration: 300 }
                        }
                    }

                    Text {
                        text: backend.isConnected ? "Подключен" : "Отключен"
                        color: "white"
                        font.pixelSize: 12
                    }
                }

                // Строка для имени пользователя
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: "Имя:"
                        color: "white"
                        font.pixelSize: 12
                    }

                    TextField {
                        id: userNameField
                        Layout.fillWidth: true
                        Layout.preferredWidth: 100
                        text: backend.currentUser
                        color: "black"
                        background: Rectangle {
                            color: "white"
                            radius: 5
                        }
                        placeholderText: "Введите имя"
                        font.pixelSize: 12
                        onAccepted: {
                            if (text.trim().length > 0) {
                                backend.currentUser = text.trim()
                            }
                        }
                        onTextChanged: {
                            if (text.trim().length > 0) {
                                backend.currentUser = text.trim()
                            }
                        }
                    }

                    Button {
                        text: "Обновить"
                        background: Rectangle {
                            color: "#4CAF50"
                            radius: 5
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        onClicked: {
                            if (userNameField.text.trim().length > 0) {
                                backend.currentUser = userNameField.text.trim()
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                        id: modeCombo
                        Layout.preferredWidth: 120
                        model: ["Клиент", "Сервер"]
                        currentIndex: 0
                        onCurrentIndexChanged: {
                            backend.networkManager.isServer = currentIndex === 1
                        }
                    }

                    TextField {
                        id: addressField
                        Layout.fillWidth: true
                        placeholderText: modeCombo.currentIndex === 0 ? "IP адрес сервера" : "Порт (по умолчанию 8080)"
                        text: modeCombo.currentIndex === 0 ? "127.0.0.1" : "8080"
                    }

                    Button {
                        id: connectButton
                        text: backend.isConnected ? "Отключиться" : "Подключиться"
                        background: Rectangle {
                            color: backend.isConnected ? "#f44336" : "#4CAF50"
                            radius: 5
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        onClicked: {
                            if (backend.isConnected) {
                                backend.networkManager.disconnectFromHost()
                            } else {
                                if (modeCombo.currentIndex === 0) {
                                    backend.networkManager.connectToHost(addressField.text, 8080)
                                } else {
                                    backend.networkManager.startServer(parseInt(addressField.text) || 8080)
                                }
                            }
                        }
                    }
                }

                Text {
                    text: backend.networkManager.connectionStatus
                    color: "white"
                    font.pixelSize: 10
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
        }

        // Поля морского боя
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 10

            // Индикатор чей ход
            Rectangle {
                id: turnIndicatorRect
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 400
                Layout.preferredHeight: 40
                color: "#3498db"
                radius: 10
                border.color: "#2c3e50"
                border.width: 2

                Text {
                    id: turnIndicatorText
                    anchors.centerIn: parent
                    text: root.model ? root.model.getTurnStatus() : "Ожидание игры..."
                    font.pixelSize: 20
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            RowLayout {
                spacing: 40
                Layout.alignment: Qt.AlignHCenter

//////////////////////////////////////////////// Поле игрока \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
                Column {
                    spacing: 10
                    Text {
                        text: "🚢 Ваше поле"
                        font.pixelSize: 18
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        width: 250
                        color: "#2c3e50"
                    }

                    Rectangle {
                        width: 250
                        height: 250
                        color: "lightgray"
                        border.color: "silver"
                        border.width: 2
                        radius: 5

                        Grid {
                            anchors.centerIn: parent
                            rows: 10
                            columns: 10
                            spacing: 2

                            Repeater {
                                id: playerGridRepeater
                                model: 100

                                Rectangle {
                                    id: playerCellRect
                                    width: 20
                                    height: 20
                                    color: getCellColor(index)
                                    border.color: "slategray"
                                    border.width: 1

                                    Text {
                                        id: playerCellText
                                        anchors.centerIn: parent
                                        text: getCellMark(index)
                                        font.pixelSize: 24
                                        font.bold: true
                                        color: getMarkColor(index)
                                    }

                                    function getCellIsShoted(cellIndex) {
                                        if (!root.model) return false;
                                        return root.model.getPlayerCellIsShoted(cellIndex);
                                    }

                                    function getCellIsOccupied(cellIndex) {
                                        if (!root.model) return false;
                                        return root.model.getPlayerCellIsOccupied(cellIndex);
                                    }

                                    function getCellMark(cellIndex) {
                                        if (!root.model) return "";
                                        if (root.model.getPlayerCellIsShoted(cellIndex)) {
                                            if (root.model.getPlayerCellIsOccupied(cellIndex)) {
                                                return "✖";
                                            } else {
                                                return "•";
                                            }
                                        }
                                        return "";
                                    }

                                    function getMarkColor(cellIndex) {
                                        if (!root.model) return "black";
                                        if (root.model.getPlayerCellIsOccupied(cellIndex)) {
                                            return "red";
                                        }
                                        return "black";
                                    }

                                    function getCellColor(cellIndex) {
                                        if (!root.model) return "#ecf0f1";
                                        if (root.model.getPlayerCellIsShoted(cellIndex)) {
                                            if (root.model.getPlayerCellIsOccupied(cellIndex)) {
                                                return "#e74c3c";
                                            }
                                        }
                                        if (root.model.getPlayerCellIsOccupied(cellIndex)) {
                                            return "#5dade2";
                                        }
                                        return "#ecf0f1";
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: 250
                        height: 50
                        color: "#ecf0f1"
                        radius: 8
                        border.color: "#bdc3c7"
                        border.width: 1

                        Text {
                            id: playerStatusText
                            anchors.centerIn: parent
                            text: root.model ? root.model.getPlayerGameStatus() : "Ожидание"
                            font.pixelSize: 16
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            color: "#2c3e50"
                        }
                    }

                    Button {
                        id: shipPlacement
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 40
                        text: "Расставить корабли"

                        background: Rectangle {
                            color: parent.enabled ? "#2196F3" : "#b0bec5"
                            radius: 5
                        }

                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 14
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (root.model) {
                                root.model.shipPlacing();
                                // Обновляем все клетки
                                root.updateAllCells();
                                playerStatusText.text = root.model.getPlayerGameStatus();
                                turnIndicatorText.text = root.model.getTurnStatus();
                            }
                        }
                    }
                }

//////////////////////////////////////// Поле противника \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
                Column {
                    spacing: 10
                    Text {
                        text: "🤖 Поле противника"
                        font.pixelSize: 18
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        width: 250
                        color: "#2c3e50"
                    }

                    Rectangle {
                        width: 250
                        height: 250
                        color: "lightgray"
                        border.color: "silver"
                        border.width: 2
                        radius: 5

                        // Затемнение поля противника во время хода противника
                        Rectangle {
                            anchors.fill: parent
                            color: "black"
                            opacity: root.model && root.model.isPlayerFieldBlocked() ? 0.3 : 0
                            visible: root.model && root.model.isPlayerFieldBlocked()
                            z: 1
                        }

                        Grid {
                            anchors.centerIn: parent
                            rows: 10
                            columns: 10
                            spacing: 2

                            Repeater {
                                id: botGridRepeater
                                model: 100

                                Rectangle {
                                    id: botCellRect
                                    width: 20
                                    height: 20
                                    color: getCellColor(index)
                                    border.color: "slategray"
                                    border.width: 1

                                    Text {
                                        id: botCellText
                                        anchors.centerIn: parent
                                        text: getCellMark(index)
                                        font.pixelSize: 24
                                        font.bold: true
                                        color: getMarkColor(index)
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: root.model && !root.model.isPlayerFieldBlocked()
                                        onClicked: {
                                            if (root.model && !getCellIsShoted(index) && !root.model.isPlayerFieldBlocked()) {
                                                root.model.shot(index);
                                                root.updateBotCell(index);
                                                botStatusText.text = root.model.getBotGameStatus();
                                                playerStatusText.text = root.model.getPlayerGameStatus();
                                                turnIndicatorText.text = root.model.getTurnStatus();
                                            }
                                        }
                                    }

                                    function getEnemyCellIsShoted(cellIndex) {
                                        if (!root.model) return false;
                                        return root.model.getBotCellIsShoted(cellIndex);
                                    }

                                    function getEnemyCellIsOccupied(cellIndex) {
                                        if (!root.model) return false;
                                        return root.model.getBotCellIsOccupied(cellIndex);
                                    }

                                    function getEnemyCellMark(cellIndex) {
                                        if (!root.model) return "";
                                        if (root.model.getEnemyCellIsShoted(cellIndex)) {
                                            if (root.model.getEnemyCellIsOccupied(cellIndex)) {
                                                return "✖";
                                            } else {
                                                return "•";
                                            }
                                        }
                                        return "";
                                    }

                                    function getEnemyMarkColor(cellIndex) {
                                        if (!root.model) return "black";
                                        if (root.model.getEnemyCellIsOccupied(cellIndex)) {
                                            return "red";
                                        }
                                        return "black";
                                    }

                                    function getEnemyCellColor(cellIndex) {
                                        if (!root.model) return "#ecf0f1";
                                        if (root.model.getEnemyCellIsShoted(cellIndex)) {
                                            if (root.model.getEnemyCellIsOccupied(cellIndex)) {
                                                return "#e74c3c";
                                            }
                                        }
                                        return "#ecf0f1";
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: 250
                        height: 50
                        color: "#ecf0f1"
                        radius: 8
                        border.color: "#bdc3c7"
                        border.width: 1

                        Text {
                            id: botStatusText
                            anchors.centerIn: parent
                            text: root.model ? root.model.getBotGameStatus() : "Ожидание"
                            font.pixelSize: 16
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            color: "#2c3e50"
                        }
                    }
                }
            }
        }

        // Список сообщений (уменьшенная высота)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#f5f5f5"
            radius: 10
            border.color: "#e0e0e0"
            border.width: 1

            ListView {
                id: messageListView
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                clip: true
                model: backend.messages

                delegate: MessegeDelegate {
                    messageText: modelData || ""
                    sender: backend.messageSenders[index] || ""
                    time: backend.messageTimes[index] || ""
                    isOwnMessage: sender === backend.currentUser
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                Component.onCompleted: {
                    positionViewAtEnd()
                }

                onCountChanged: {
                    positionViewAtEnd()
                }
            }
        }

        // Поле ввода и кнопка отправки
        Rectangle {
            id: inputContainer
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "white"
            radius: 10
            border.color: "#e0e0e0"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#f8f9fa"
                    radius: 5
                    border.color: "#dee2e6"
                    border.width: 1

                    TextArea {
                        id: messageInput
                        anchors.fill: parent
                        anchors.margins: 8
                        placeholderText: backend.isConnected ? "Введите сообщение..." : "Подключитесь к сети для отправки сообщений"
                        wrapMode: TextArea.Wrap
                        font.pixelSize: 14
                        enabled: backend.isConnected

                        Keys.onEnterPressed: {
                            if (event.modifiers & Qt.ShiftModifier) {
                                event.accepted = false
                            } else {
                                event.accepted = true
                                sendButton.clicked()
                            }
                        }

                        onTextChanged: {
                            if (text && text.length > 0) {
                                inputContainer.Layout.preferredHeight = Math.min(120, 40 + contentHeight)
                            } else {
                                inputContainer.Layout.preferredHeight = 80
                            }
                        }
                    }
                }

                ColumnLayout {
                    spacing: 5

                    Button {
                        id: sendButton
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 40
                        text: "Отправить"
                        enabled: backend.isConnected && messageInput.text && messageInput.text.trim().length > 0

                        background: Rectangle {
                            color: parent.enabled ? "#2196F3" : "#b0bec5"
                            radius: 5
                        }

                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 14
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (messageInput.text) {
                                backend.sendMessage(messageInput.text)
                                messageInput.text = ""
                                messageInput.forceActiveFocus()
                            }
                        }
                    }

                    Label {
                        text: "Shift+Enter\nдля переноса"
                        font.pixelSize: 10
                        color: "#757575"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        // Статусная строка
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: "#e3f2fd"
            radius: 5

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    text: "Сообщений: " + (backend.messages ? backend.messages.length : 0)
                    font.pixelSize: 12
                    color: "#1565c0"
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: backend.isConnected ? "🟢 Онлайн" : "🔴 Офлайн"
                    font.pixelSize: 12
                    color: backend.isConnected ? "#4CAF50" : "#f44336"
                }
            }
        }
        Connections {
                target: root.model

                function onPlayerFieldUpdated() {
                    root.updateAllCells();
                }

                function onGameWon() {
                    botStatusText.text = "🏆 ПОБЕДА! 🏆\nВсе корабли противника уничтожены!"
                    botStatusText.color = "#e74c3c"
                    gameMessageDialog.title = "Победа!"
                    gameMessageDialog.text = "🎉 ПОЗДРАВЛЯЕМ! 🎉\nВы уничтожили все корабли противника!"
                    gameMessageDialog.open()
                    // Обновляем статус хода
                    turnIndicator.text = root.model.getTurnStatus()
                }

                function onGameOver() {
                    playerStatusText.text = "ВЫ ПРОИГРАЛИ! 💀\nВсе ваши корабли уничтожены!"
                    playerStatusText.color = "black"
                    gameMessageDialog.title = "Игра окончена"
                    gameMessageDialog.text = "😢 Вы проиграли!\nВсе ваши корабли уничтожены."
                    gameMessageDialog.open()
                    // Обновляем статус хода
                    turnIndicator.text = root.model.getTurnStatus()
                }

                function onGameStatusChanged() {
                    botStatusText.text = root.model.getBotGameStatus()
                    botStatusText.color = "#2c3e50"
                    playerStatusText.text = root.model.getPlayerGameStatus()
                    for (var i = 0; i < 100; i++) {
                        root.updatePlayerCell(i);
                    }
                }

                function onTurnStatusChanged() {
                    // Обновляем индикатор хода
                    turnIndicator.text = root.model.getTurnStatus()
                    // Меняем цвет индикатора в зависимости от статуса
                    var status = root.model.getTurnStatus()
                    if (status.includes("ПОБЕДА")) {
                        turnIndicator.color = "#27ae60"
                    } else if (status.includes("ПРОИГРАЛИ")) {
                        turnIndicator.color = "#e74c3c"
                    } else if (status.includes("Ход бота")) {
                        turnIndicator.color = "#f39c12"
                    } else {
                        turnIndicator.color = "white"
                    }
                }
            }
        }

    // Сохраняем ссылку на индикатор для обновления
    property alias turnIndicator: turnIndicatorText

    Component.onCompleted: {
        if (root.model) {
            botStatusText.text = root.model.getBotGameStatus();
            playerStatusText.text = root.model.getPlayerGameStatus();
            turnIndicator.text = root.model.getTurnStatus();
        }
    }

}
