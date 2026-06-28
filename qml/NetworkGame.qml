import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.LocalStorage 2.0

Item {
    id: root
    property var adapter: null
    signal backToMenu()

    property var model: backend.gameModel
    property string currentUser: backend.currentUser
    property bool isConnected: backend.isConnected
    property bool isServer: false
    property bool clientReady: false
    onClientReadyChanged: {
        console.log("clientReady изменился на:", clientReady)
        updateGameButtonState()
    }
    property bool gameStarted: false

    property bool playerReady: false
    onPlayerReadyChanged: {
        console.log("playerReady изменился на:", playerReady)
        updateGameButtonState()
    }

    // Функции для работы с localStorage
    function getStorage() {
        return LocalStorage.openDatabaseSync("SeaBattleSettings", "1.0", "Settings", 1000000)
    }

    function getLastAddress() {
        try {
            var db = getStorage()
            db.transaction(function(tx) {
                tx.executeSql('CREATE TABLE IF NOT EXISTS settings(key TEXT PRIMARY KEY, value TEXT)')
            })
            var result = ""
            db.transaction(function(tx) {
                var rs = tx.executeSql('SELECT value FROM settings WHERE key = "lastAddress"')
                if (rs.rows.length > 0) {
                    result = rs.rows.item(0).value
                }
            })
            return result || "127.0.0.1"
        } catch(e) {
            console.warn("Failed to read lastAddress:", e)
            return "127.0.0.1"
        }
    }

    function getLastPort() {
        try {
            var db = getStorage()
            db.transaction(function(tx) {
                tx.executeSql('CREATE TABLE IF NOT EXISTS settings(key TEXT PRIMARY KEY, value TEXT)')
            })
            var result = ""
            db.transaction(function(tx) {
                var rs = tx.executeSql('SELECT value FROM settings WHERE key = "lastPort"')
                if (rs.rows.length > 0) {
                    result = rs.rows.item(0).value
                }
            })
            return result || "8080"
        } catch(e) {
            console.warn("Failed to read lastPort:", e)
            return "8080"
        }
    }

    function saveLastAddress(address) {
        try {
            var db = getStorage()
            db.transaction(function(tx) {
                tx.executeSql('CREATE TABLE IF NOT EXISTS settings(key TEXT PRIMARY KEY, value TEXT)')
                tx.executeSql('INSERT OR REPLACE INTO settings(key, value) VALUES("lastAddress", ?)', [address])
            })
        } catch(e) {
            console.warn("Failed to save lastAddress:", e)
        }
    }

    function saveLastPort(port) {
        try {
            var db = getStorage()
            db.transaction(function(tx) {
                tx.executeSql('CREATE TABLE IF NOT EXISTS settings(key TEXT PRIMARY KEY, value TEXT)')
                tx.executeSql('INSERT OR REPLACE INTO settings(key, value) VALUES("lastPort", ?)', [port])
            })
        } catch(e) {
            console.warn("Failed to save lastPort:", e)
        }
    }

    // Загружаем сохраненные настройки при инициализации
    property string lastAddress: getLastAddress()
    property int lastPort: parseInt(getLastPort())

    function updateGameButtonState() {
        if (isServer) {
            gameButton.text = "Начать игру"
            gameButton.enabled = isConnected && clientReady && !gameStarted && !playerReady
            console.log("Сервер: isConnected=" + isConnected +
                                   ", clientReady=" + clientReady +
                                   ", gameStarted=" + gameStarted +
                                   ", playerReady=" + playerReady +
                                   ", enabled=" + (isConnected && clientReady && !gameStarted && !playerReady))
            gameButton.forceActiveFocus()
        } else {
            gameButton.text = "Готов"
            gameButton.enabled = isConnected && !playerReady && !gameStarted
            console.log("Клиент: isConnected=" + isConnected +
                                   ", playerReady=" + playerReady +
                                   ", gameStarted=" + gameStarted +
                                   ", enabled=" + (isConnected && !playerReady && !gameStarted))
        }
    }


    // Функция сброса состояния игры
    function resetGameState() {
        clientReady = false
        gameStarted = false
        playerReady = false
        updateGameButtonState()
        shipPlacementButton.enabled = true
        updateAllCells()
        if (model) {
            playerStatusText.text = model.getPlayerGameStatus()
            turnIndicatorText.text = model.getTurnStatus()
        }
    }

    // Функция для обновления конкретной клетки поля бота
    function updateEnemyCell(index) {
        var item = enemyGridRepeater.itemAt(index);
        if (item) {
            item.color = item.getCellColor(index);
            var textItem = item.children[0];
            if (textItem) {
                textItem.text = item.getCellMark(index);
                textItem.color = item.getMarkColor(index);
            }
        }
    }

    function clearEnemyCell(index) {
        var item = enemyGridRepeater.itemAt(index);
        if (item) {
            var textItem = item.children[0];
            if (textItem) {
                textItem.text = "";
                textItem.color = "#ecf0f1";
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
            updateEnemyCell(i);
        }
    }

    function clearAllCells() {
        for (var i = 0; i < 100; i++) {
            clearEnemyCell(i)
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
            Layout.preferredHeight: 160
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
                        id: connectionStatus
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

                // Строка с настройками подключения
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                        id: modeCombo
                        Layout.preferredWidth: 100
                        model: ["Клиент", "Сервер"]
                        currentIndex: 0
                        onCurrentIndexChanged: {
                            root.isServer = currentIndex === 1
                            backend.networkManager.isServer = currentIndex === 1
                            root.resetGameState()
                            shipPlacementButton.enabled = true
                            if (currentIndex === 0) {
                                addressField.placeholderText = "IP адрес сервера"
                                portField.placeholderText = "Порт (8080)"
                            } else {
                                addressField.placeholderText = "IP адрес (0.0.0.0)"
                                portField.placeholderText = "Порт (8080)"
                            }
                        }
                    }

                    TextField {
                        id: addressField
                        Layout.fillWidth: true
                        Layout.preferredWidth: 120
                        placeholderText: modeCombo.currentIndex === 0 ? "IP адрес сервера" : "IP адрес (0.0.0.0)"
                        text: root.lastAddress
                        color: "black"
                        background: Rectangle {
                            color: "white"
                            radius: 5
                        }
                        font.pixelSize: 12
                        onTextChanged: {
                            if (text.trim().length > 0) {
                                root.lastAddress = text.trim()
                                saveLastAddress(text.trim())
                            }
                        }
                    }

                    TextField {
                        id: portField
                        Layout.preferredWidth: 65
                        placeholderText: "Порт"
                        text: String(root.lastPort)
                        color: "black"
                        background: Rectangle {
                            color: "white"
                            radius: 5
                        }
                        font.pixelSize: 12
                        validator: IntValidator {
                            bottom: 1
                            top: 65535
                        }
                        onTextChanged: {
                            var port = parseInt(text)
                            if (port > 0 && port <= 65535) {
                                root.lastPort = port
                                saveLastPort(text)
                            }
                        }
                    }

                    Button {
                        id: connectButton
                        text: backend.isConnected ? "Отключиться" : "Подключиться"
                        Layout.preferredWidth: 110
                        background: Rectangle {
                            color: backend.isConnected ? "#f44336" : "#4CAF50"
                            radius: 5
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 11
                            font.bold: true
                        }
                        onClicked: {
                            if (backend.isConnected) {
                                backend.networkManager.disconnectFromHost()
                                root.resetGameState()
                                shipPlacementButton.enabled = true
                            } else {
                                var port = parseInt(portField.text) || 8080

                                if(!backend.networkManager.isValidPort(port)) {
                                    connectionStatus.text = "Недопустимый порт"
                                    return
                                }

                                if (modeCombo.currentIndex === 0) {
                                    // Режим клиента
                                    var address = addressField.text.trim()
                                    if(!backend.networkManager.isValidAddress(address)) {
                                        connectionStatus.text = "Недопустимый адрес"
                                        return
                                    }
                                    if (address.length === 0) {
                                        address = "127.0.0.1"
                                    }
                                    backend.networkManager.connectToHost(address, port)
                                } else {
                                    // Режим сервера
                                    backend.networkManager.startServer(port)
                                }
                                root.resetGameState()
                                connectionStatus.text = "Подключён"
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

                // Поле игрока
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
                        id: shipPlacementButton
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 40
                        text: "Расставить корабли"
                        enabled: !root.gameStarted

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
                                root.updateAllCells();
                                playerStatusText.text = root.model.getPlayerGameStatus();
                                turnIndicatorText.text = root.model.getTurnStatus();
                                root.updateGameButtonState()
                            }
                        }
                    }
                }

                // Поле противника
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

                        Grid {
                            anchors.centerIn: parent
                            rows: 10
                            columns: 10
                            spacing: 2

                            Repeater {
                                id: enemyGridRepeater
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
                                        enabled: true
                                        onClicked: {
                                            console.log("Клик по клетке поля противника:", index)
                                            if (root.model && root.model.isGameStarted() && root.model.isMyTurn()) {
                                                root.model.shot(index)
                                            }
                                        }
                                    }

                                    function getCellIsShoted(cellIndex) {
                                        if (!root.model) return false;
                                        return root.model.getEnemyCellIsShoted(cellIndex);
                                    }

                                    function getCellIsOccupied(cellIndex) {
                                        if (!root.model) return false;
                                        return root.model.getEnemyCellIsOccupied(cellIndex);
                                    }

                                    function getCellMark(cellIndex) {
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

                                    function getMarkColor(cellIndex) {
                                        if (!root.model) return "black";
                                        if (root.model.getEnemyCellIsOccupied(cellIndex)) {
                                            return "red";
                                        }
                                        return "black";
                                    }

                                    function getCellColor(cellIndex) {
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
                            id: enemyStatusText
                            anchors.centerIn: parent
                            text: root.model ? root.model.getEnemyGameStatus() : "Ожидание"
                            font.pixelSize: 16
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            color: "#2c3e50"
                        }
                    }

                    Row {
                        spacing: 10
                        Layout.alignment: Qt.AlignHCenter

                        Button {
                            id: gameButton
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 40
                            text: root.isServer ? "Начать игру" : "Готов"
                            enabled: {
                                if (root.isServer) {
                                    var result = root.isConnected && root.clientReady && !root.gameStarted && !root.playerReady
                                    console.log("gameButton.enabled вычисление (сервер):", result)
                                    return result
                                } else {
                                    var result = root.isConnected && !root.playerReady && !root.gameStarted
                                    console.log("gameButton.enabled вычисление (клиент):", result)
                                    return result
                                }
                            }

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
                                if (root.isServer) {
                                    if (root.model) {
                                        console.log("СЕРВЕР: Нажата кнопка Начать игру")
                                        root.model.setMyTurn(true)
                                        root.model.startGame()
                                        root.gameStarted = true
                                        root.playerReady = true
                                        shipPlacementButton.enabled = false
                                        backend.sendMessage("game_start")
                                        backend.receiveMessage("Система", "Игра началась! Ваш ход.")
                                        playerStatusText.text = root.model.getPlayerGameStatus()
                                        turnIndicatorText.text = root.model.getTurnStatus()
                                        root.updateGameButtonState()
                                        console.log("Сервер: isMyTurn=" + root.model.isMyTurn())
                                        console.log("Сервер: isPlayerFieldBlocked=" + root.model.isPlayerFieldBlocked())
                                        clearAllCells()
                                    }
                                } else {
                                    if (root.model) {
                                        console.log("КЛИЕНТ: Нажата кнопка Готов")
                                        root.playerReady = true
                                        backend.sendMessage("player_ready")
                                        backend.receiveMessage("Система", "Вы готовы к игре. Ожидайте начала.")
                                        playerStatusText.text = root.model.getPlayerGameStatus()
                                        turnIndicatorText.text = root.model.getTurnStatus()
                                        shipPlacementButton.enabled = false
                                        root.updateGameButtonState()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Список сообщений
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
                model: backend.messageObjects

                delegate: Item {
                    width: ListView.view.width
                    height: messageDelegate.height

                    MessegeDelegate {
                        id: messageDelegate
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        messageData: {
                            if (!model || !modelData) return {sender: "", text: "", time: ""};
                            return modelData;
                        }
                        isOwnMessage: {
                            if (!messageData || !messageData.sender) return false;
                            return messageData.sender === backend.currentUser && messageData.sender !== "Система";
                        }
                    }
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
    }

    // Обработка сообщений от других игроков
    Connections {
        target: backend
        function onNewMessageReceived(sender, text) {
            console.log("onNewMessageReceived: sender=" + sender + ", text=" + text)

            if (text === "player_ready") {
                console.log("СЕРВЕР: Получен player_ready от клиента")
                root.clientReady = true
                root.updateGameButtonState()
                backend.receiveMessage("Система", "Противник готов к игре!")
                console.log("Получен player_ready, clientReady=" + root.clientReady)
            } else if (text === "game_start") {
                console.log("КЛИЕНТ: Получен game_start от сервера")
                if (root.model) {
                    root.gameStarted = true
                    root.playerReady = true
                    root.model.setMyTurn(false)
                    root.model.setPlayerFieldBlocked(true)
                    root.model.updateGameStatus()
                    root.updateAllCells()
                    shipPlacementButton.enabled = false
                    playerStatusText.text = root.model.getPlayerGameStatus()
                    turnIndicatorText.text = root.model.getTurnStatus()
                    backend.receiveMessage("Система", "Игра началась! Ход противника.")
                    root.updateGameButtonState()
                    clearAllCells()
                    console.log("Клиент: gameStarted=" + root.gameStarted +
                                       ", isMyTurn=" + root.model.isMyTurn())
                }
            } else if (text === "game_over") {
                if (root.model) {
                    root.gameStarted = false
                    root.playerReady = false
                    root.clientReady = false
                    root.model.setMyTurn(false)
                    root.model.setPlayerFieldBlocked(true)
                    root.model.updateGameStatus()
                    root.updateAllCells()
                    shipPlacementButton.enabled = true
                    playerStatusText.text = root.model.getPlayerGameStatus()
                    turnIndicatorText.text = root.model.getTurnStatus()
                    backend.receiveMessage("Система", "Вы выиграли!")
                    root.updateGameButtonState()
                }
            }
        }

        function onUpdateGameButtonState() {
            console.log("Получен сигнал updateGameButtonState")
            root.updateGameButtonState()
        }
    }

    // Обработка сигналов от модели
    Connections {
        target: root.model

        function onPlayerFieldUpdated() {
            root.updateAllCells();
        }

        function onGameWonSignal() {
            gameStarted = false
            playerReady = false
            console.log("Получен сигнал onGameWonSignal gameStarted ==>", gameStarted)
            enemyStatusText.text = "🏆 ПОБЕДА! 🏆\nВсе корабли противника уничтожены!"
            enemyStatusText.color = "#e74c3c"
            turnIndicatorText.text = root.model.getTurnStatus()
            root.updateAllCells()
            shipPlacementButton.enabled = true
            root.updateGameButtonState()
            // gameMessageDialog.title = "Победа!"
            // gameMessageDialog.text = "🎉 ПОЗДРАВЛЯЕМ! 🎉\nВы уничтожили все корабли противника!"
            // gameMessageDialog.open()
        }

        function onGameOverSignal() {
            gameStarted = false
            playerReady = false
            console.log("Получен сигнал onGameOverSignal gameStarted ==>", gameStarted)
            playerStatusText.text = "ВЫ ПРОИГРАЛИ! 💀\nВсе ваши корабли уничтожены!"
            playerStatusText.color = "black"
            turnIndicatorText.text = root.model.getTurnStatus()
            root.updateAllCells()
            shipPlacementButton.enabled = true
            root.updateGameButtonState()
            // gameMessageDialog.title = "Игра окончена"
            // gameMessageDialog.text = "😢 Вы проиграли!\nВсе ваши корабли уничтожены."
            // gameMessageDialog.open()
        }

        function onGameStatusChanged() {
            enemyStatusText.text = root.model.getEnemyGameStatus()
            enemyStatusText.color = "#2c3e50"
            playerStatusText.text = root.model.getPlayerGameStatus()
            turnIndicatorText.text = root.model.getTurnStatus()
            root.updateAllCells()
            root.updateGameButtonState()
        }

        function onTurnStatusChanged() {
            turnIndicatorText.text = root.model.getTurnStatus()
            var status = root.model.getTurnStatus()
            if (status.includes("ПОБЕДА")) {
                turnIndicatorRect.color = "#27ae60"
            } else if (status.includes("ПРОИГРАЛИ")) {
                turnIndicatorRect.color = "#e74c3c"
            } else if (status.includes("Ожидание хода противника")
                       || status.includes("Ход противника")) {
                turnIndicatorRect.color = "#f39c12"
            } else if(status.includes("Ваш ход")) {
                turnIndicatorRect.color = "#2ecc71"
            } else {
                turnIndicatorRect.color = "#3498db"
            }
            root.updateAllCells()
        }

        function onShotRequested(index) {
            console.log("Отправка выстрела через backend, индекс:", index)
            backend.shotMessage(String(index))
        }

        function onUpdateGameButtonState() {
            console.log("Получен сигнал updateGameButtonState от модели")
            root.updateGameButtonState()
        }
    }



    Component.onCompleted: {
        if (root.model) {
            enemyStatusText.text = root.model.getEnemyGameStatus();
            playerStatusText.text = root.model.getPlayerGameStatus();
            turnIndicatorText.text = root.model.getTurnStatus();
        }

        root.updateGameButtonState()
        shipPlacementButton.enabled = true
        root.resetGameState()
    }

    // Диалог для уведомлений
    Dialog {
        id: gameMessageDialog
        modal: true
        anchors.centerIn: parent
        width: 300
        height: 200

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                id: dialogTitle
                text: gameMessageDialog.title
                font.pixelSize: 18
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            Text {
                id: dialogText
                text: gameMessageDialog.text
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Button {
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                onClicked: gameMessageDialog.close()
            }
        }
    }
}
