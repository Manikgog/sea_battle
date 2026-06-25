import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root
    property var adapter: null
    signal backToMenu()

    MessageDialog {
        id: gameMessageDialog
        title: "Игра окончена"
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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "← Назад"
                onClicked: {
                    if (adapter) adapter.disconnectFromServer()
                    root.backToMenu()
                }
                background: Rectangle {
                    color: "#95a5a6"
                    radius: 8
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: adapter ? adapter.getGameStatusText() : ""
                color: "#2c3e50"
                font.pixelSize: 14
                font.bold: true
            }
        }

        RowLayout {
            spacing: 40
            Layout.alignment: Qt.AlignHCenter

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
                                width: 20
                                height: 20
                                color: getCellColor(index)
                                border.color: "slategray"
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: getCellMark(index)
                                    font.pixelSize: 24
                                    font.bold: true
                                    color: getMarkColor(index)
                                }

                                function getCellIsShoted(idx) { return adapter ? adapter.getPlayerCellIsShoted(idx) : false; }
                                function getCellIsOccupied(idx) { return adapter ? adapter.getPlayerCellIsOccupied(idx) : false; }

                                function getCellMark(idx) {
                                    if (!adapter) return "";
                                    if (adapter.getPlayerCellIsShoted(idx)) {
                                        return adapter.getPlayerCellIsOccupied(idx) ? "✖" : "•";
                                    }
                                    return "";
                                }

                                function getMarkColor(idx) {
                                    if (!adapter) return "black";
                                    return adapter.getPlayerCellIsOccupied(idx) ? "red" : "";
                                }

                                function getCellColor(idx) {
                                    if (!adapter) return "#ecf0f1";
                                    if (adapter.getPlayerCellIsShoted(idx)) {
                                        if (adapter.getPlayerCellIsOccupied(idx)) return "#e74c3c";
                                    }
                                    if (adapter.getPlayerCellIsOccupied(idx)) return "#5dade2";
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
                        text: adapter ? adapter.getPlayerGameStatus() : ""
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        color: "#2c3e50"
                    }
                }
            }

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
                                width: 20
                                height: 20
                                color: getCellColor(index)
                                border.color: "slategray"
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: getCellMark(index)
                                    font.pixelSize: 24
                                    font.bold: true
                                    color: getMarkColor(index)
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    enabled: adapter && adapter.isConnected && adapter.isMyTurn
                                    onClicked: {
                                        if (adapter && adapter.isConnected && adapter.isMyTurn && !getCellIsShoted(index)) {
                                            adapter.shot(index);
                                            root.updateBotCell(index);
                                            botStatusText.text = adapter.getBotGameStatus();
                                            playerStatusText.text = adapter.getPlayerGameStatus();
                                        }
                                    }
                                }

                                function getCellIsShoted(idx) { return adapter ? adapter.getBotCellIsShoted(idx) : false; }
                                function getCellIsOccupied(idx) { return adapter ? adapter.getBotCellIsOccupied(idx) : false; }

                                function getCellMark(idx) {
                                    if (!adapter) return "";
                                    if (adapter.getBotCellIsShoted(idx)) {
                                        return adapter.getBotCellIsOccupied(idx) ? "✖" : "•";
                                    }
                                    return "";
                                }

                                function getMarkColor(idx) {
                                    if (!adapter) return "black";
                                    return adapter.getBotCellIsOccupied(idx) ? "red" : "";
                                }

                                function getCellColor(idx) {
                                    if (!adapter) return "#ecf0f1";
                                    if (adapter.getBotCellIsShoted(idx)) {
                                        if (adapter.getBotCellIsOccupied(idx)) return "#e74c3c";
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
                        text: adapter ? adapter.getBotGameStatus() : ""
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        color: "#2c3e50"
                    }
                }
            }
        }

        // Сетевая панель управления
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: "#ecf0f1"
            radius: 8
            border.color: "#bdc3c7"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                ColumnLayout {
                    spacing: 5
                    Layout.fillWidth: true

                    Text {
                        text: "Сервер"
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
                            onClicked: {
                                if (adapter && adapter.startServer(parseInt(serverPortField.text))) {
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
                        text: "Клиент"
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
                            onClicked: {
                                if (adapter) {
                                    adapter.setPlayerName(playerNameField.text)
                                    if (adapter.connectToServer(clientAddressField.text,
                                                              parseInt(clientPortField.text))) {
                                        statusMessageText.text = "Подключение к серверу..."
                                    }
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
        }

        Text {
            id: statusMessageText
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            text: "Создайте игру или подключитесь к существующей"
            font.pixelSize: 12
            color: "#6c757d"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        Button {
            text: "🔄 Новая игра"
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 14
            enabled: adapter && adapter.isConnected
            onClicked: {
                if (adapter && adapter.isConnected) {
                    adapter.newGame()
                    root.updateAllCells()
                    botStatusText.text = adapter.getBotGameStatus()
                    playerStatusText.text = adapter.getPlayerGameStatus()
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

    Connections {
        target: adapter
        function onGameWon() {
            botStatusText.text = "🏆 ПОБЕДА! 🏆"
            botStatusText.color = "#e74c3c"
            gameMessageDialog.title = "Победа!"
            gameMessageDialog.text = "🎉 ПОЗДРАВЛЯЕМ! 🎉\nВы уничтожили все корабли противника!"
            gameMessageDialog.open()
        }

        function onGameOver() {
            playerStatusText.text = "ВЫ ПРОИГРАЛИ! 💀"
            playerStatusText.color = "black"
            gameMessageDialog.title = "Игра окончена"
            gameMessageDialog.text = "😢 Вы проиграли!\nВсе ваши корабли уничтожены."
            gameMessageDialog.open()
        }

        function onGameStatusChanged() {
            if (!adapter) return
            botStatusText.text = adapter.getBotGameStatus()
            playerStatusText.text = adapter.getPlayerGameStatus()
            for (var i = 0; i < 100; i++) {
                root.updateBotCell(i)
                root.updatePlayerCell(i)
            }
        }

        function onGameMessage(message) {
            statusMessageText.text = message
            statusMessageText.color = "#2c3e50"
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
    }

    Component.onCompleted: {
        if (adapter) {
            botStatusText.text = adapter.getBotGameStatus()
            playerStatusText.text = adapter.getPlayerGameStatus()
        }
    }
}
