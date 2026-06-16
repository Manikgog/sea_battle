import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    id: root
    width: 800
    height: 650
    visible: true
    title: qsTr("Морской бой - Сетевая игра")

    property var model: ModelAdapter

    MessageDialog {
        id: gameMessageDialog
        title: "Игра окончена"
        buttons: MessageDialog.Ok
        onAccepted: {
            // Можно добавить действие при закрытии диалога
        }
    }

    MessageDialog {
        id: networkMessageDialog
        title: "Сетевая игра"
        buttons: MessageDialog.Ok
        onAccepted: {}
    }

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

    function updateAllCells() {
        for (var i = 0; i < 100; i++) {
            updateBotCell(i);
            updatePlayerCell(i);
        }
    }

    function showNetworkMessage(text) {
        networkMessageDialog.text = text;
        networkMessageDialog.open();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Верхняя панель с информацией о сетевой игре
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "#34495e"
            radius: 5
            visible: model.isConnected || model.isServer || model.isClient

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 20

                Text {
                    text: model.getGameStatusText()
                    color: "white"
                    font.pixelSize: 14
                    font.bold: true
                }

                Text {
                    text: model.isGameStarted ?
                          (model.isMyTurn ? "▶ Ваш ход" : "⏳ Ход противника") :
                          "⏳ Ожидание..."
                    color: model.isMyTurn ? "#2ecc71" : "#f1c40f"
                    font.pixelSize: 14
                    font.bold: true
                    visible: model.isConnected
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Отключиться"
                    onClicked: {
                        model.disconnectFromServer()
                        updateAllCells()
                        statusMessageText.text = "Отключено"
                    }
                    background: Rectangle {
                        color: "#e74c3c"
                        radius: 5
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 12
                    }
                }
            }
        }

        RowLayout {
            spacing: 20
            Layout.alignment: Qt.AlignHCenter

            // Поле игрока (свои корабли)
            Column {
                spacing: 5
                Text {
                    text: "🚢 Ваше поле"
                    font.pixelSize: 16
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
                                    } else {
                                        return "";
                                    }
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
                    height: 40
                    color: "#ecf0f1"
                    radius: 8
                    border.color: "#bdc3c7"
                    border.width: 1

                    Text {
                        id: playerStatusText
                        anchors.centerIn: parent
                        text: root.model ? root.model.getPlayerGameStatus() : ""
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        color: "#2c3e50"
                    }
                }
            }

            // Поле противника
            Column {
                spacing: 5
                Text {
                    text: "👤 Поле противника"
                    font.pixelSize: 16
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
                                    visible: !model.isConnected
                                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (root.model && !getCellIsShoted(index)) {
                                            root.model.shot(index);
                                            root.updateBotCell(index);
                                            botStatusText.text = root.model.getBotGameStatus();
                                            playerStatusText.text = root.model.getPlayerGameStatus();
                                        }
                                    }
                                }

                                function getCellIsShoted(cellIndex) {
                                    if (!root.model) return false;
                                    return root.model.getBotCellIsShoted(cellIndex);
                                }

                                function getCellIsOccupied(cellIndex) {
                                    if (!root.model) return false;
                                    return root.model.getBotCellIsOccupied(cellIndex);
                                }

                                function getCellMark(cellIndex) {
                                    if (!root.model) return "";
                                    if (root.model.getBotCellIsShoted(cellIndex)) {
                                        if (root.model.getBotCellIsOccupied(cellIndex)) {
                                            return "✖";
                                        } else {
                                            return "•";
                                        }
                                    }
                                    return "";
                                }

                                function getMarkColor(cellIndex) {
                                    if (!root.model) return "black";
                                    if (root.model.getBotCellIsOccupied(cellIndex)) {
                                        return "red";
                                    } else {
                                        return "";
                                    }
                                }

                                function getCellColor(cellIndex) {
                                    if (!root.model) return "#ecf0f1";
                                    if (root.model.getBotCellIsShoted(cellIndex)) {
                                        if (root.model.getBotCellIsOccupied(cellIndex)) {
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
                    height: 40
                    color: "#ecf0f1"
                    radius: 8
                    border.color: "#bdc3c7"
                    border.width: 1

                    Text {
                        id: botStatusText
                        anchors.centerIn: parent
                        text: root.model ? root.model.getBotGameStatus() : ""
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        color: "#2c3e50"
                    }
                }
            }
        }

        // Панель управления сетевой игрой
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#ecf0f1"
            radius: 8
            border.color: "#bdc3c7"
            border.width: 1
            visible: !model.isConnected

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20

                    ColumnLayout {
                        spacing: 5
                        Layout.fillWidth: true

                        Text {
                            text: "Сервер (создать игру)"
                            font.bold: true
                            font.pixelSize: 14
                        }

                        RowLayout {
                            spacing: 5

                            TextField {
                                id: serverPortField
                                text: "12345"
                                placeholderText: "Порт"
                                Layout.preferredWidth: 100
                                inputMethodHints: Qt.ImhDigitsOnly
                            }

                            Button {
                                text: "Создать игру"
                                Layout.preferredWidth: 120
                                onClicked: {
                                    if (model.startServer(parseInt(serverPortField.text))) {
                                        updateAllCells()
                                        statusMessageText.text = "Сервер запущен, ожидание подключения..."
                                    }
                                }
                                background: Rectangle {
                                    color: "#27ae60"
                                    radius: 8
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        spacing: 5
                        Layout.fillWidth: true

                        Text {
                            text: "Клиент (подключиться)"
                            font.bold: true
                            font.pixelSize: 14
                        }

                        RowLayout {
                            spacing: 5

                            TextField {
                                id: clientAddressField
                                text: "localhost"
                                placeholderText: "Адрес"
                                Layout.preferredWidth: 120
                            }

                            TextField {
                                id: clientPortField
                                text: "12345"
                                placeholderText: "Порт"
                                Layout.preferredWidth: 80
                                inputMethodHints: Qt.ImhDigitsOnly
                            }

                            TextField {
                                id: playerNameField
                                text: "Игрок"
                                placeholderText: "Имя"
                                Layout.preferredWidth: 100
                            }

                            Button {
                                text: "Подключиться"
                                Layout.preferredWidth: 120
                                onClicked: {
                                    model.setPlayerName(playerNameField.text)
                                    if (model.connectToServer(clientAddressField.text,
                                                            parseInt(clientPortField.text))) {
                                        updateAllCells()
                                        statusMessageText.text = "Подключение к серверу..."
                                    }
                                }
                                background: Rectangle {
                                    color: "#2980b9"
                                    radius: 8
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "#f8f9fa"
                    radius: 4
                    border.color: "#dee2e6"
                    border.width: 1

                    Text {
                        id: statusMessageText
                        anchors.fill: parent
                        anchors.margins: 2
                        text: "Выберите режим: создайте игру или подключитесь к существующей"
                        font.pixelSize: 12
                        color: "#6c757d"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            Button {
                text: "🔄 Новая игра"
                font.pixelSize: 14
                enabled: model.isConnected
                onClicked: {
                    if (model.isConnected) {
                        model.newGame()
                        updateAllCells()
                        botStatusText.text = model.getBotGameStatus()
                        playerStatusText.text = model.getPlayerGameStatus()
                    }
                }
                background: Rectangle {
                    color: enabled ? (parent.pressed ? "#2980b9" : "#3498db") : "#95a5a6"
                    radius: 8
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: parent.font.pixelSize
                }
            }
        }
    }

    Connections {
        target: root.model
        function onGameWon() {
            botStatusText.text = "🏆 ПОБЕДА! 🏆\nВсе корабли противника уничтожены!"
            botStatusText.color = "#e74c3c"
            gameMessageDialog.title = "Победа!"
            gameMessageDialog.text = "🎉 ПОЗДРАВЛЯЕМ! 🎉\nВы уничтожили все корабли противника!"
            gameMessageDialog.open()
        }

        function onGameOver() {
            playerStatusText.text = "ВЫ ПРОИГРАЛИ! 💀\nВсе ваши корабли уничтожены!"
            playerStatusText.color = "black"
            gameMessageDialog.title = "Игра окончена"
            gameMessageDialog.text = "😢 Вы проиграли!\nВсе ваши корабли уничтожены."
            gameMessageDialog.open()
        }

        function onGameStatusChanged() {
            botStatusText.text = root.model.getBotGameStatus()
            playerStatusText.text = root.model.getPlayerGameStatus()
            for (var i = 0; i < 100; i++) {
                root.updateBotCell(i)
                root.updatePlayerCell(i)
            }
        }

        function onTurnChanged(myTurn) {
            if (myTurn) {
                statusMessageText.text = "Ваш ход!"
                statusMessageText.color = "#2ecc71"
            } else {
                statusMessageText.text = "Ход противника..."
                statusMessageText.color = "#e74c3c"
            }
        }

        function onGameMessage(message) {
            statusMessageText.text = message
            statusMessageText.color = "#2c3e50"
        }

        function onConnectionStatusChanged() {
            if (model.isConnected) {
                statusMessageText.text = "Соединение установлено"
                statusMessageText.color = "#27ae60"
            }
        }
    }

    Component.onCompleted: {
        if (root.model) {
            botStatusText.text = root.model.getBotGameStatus()
            playerStatusText.text = root.model.getPlayerGameStatus()
        }
    }
}
