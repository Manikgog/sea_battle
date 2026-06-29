import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: root
    property var adapter
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

        // Кнопка назад
        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "← Назад"
                onClicked: root.backToMenu()
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

            Rectangle {
                id: turnStatus
                Layout.preferredWidth: 300
                Layout.preferredHeight: 40
                color: "#3498db"
                radius: 10
                visible: adapter
                Text {
                    id: turnStatusText
                    anchors.centerIn: parent
                    text: adapter ? adapter.getTurnStatus() : ""
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                }
            }
        }

        RowLayout {
            spacing: 40
            Layout.alignment: Qt.AlignHCenter

            // Поле игрока
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

            // Поле бота
            Column {
                spacing: 5
                Text {
                    text: "🤖 Поле противника"
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

                    Rectangle {
                        anchors.fill: parent
                        color: "black"
                        opacity: adapter && adapter.isPlayerFieldBlocked() ? 0.3 : 0
                        visible: adapter && adapter.isPlayerFieldBlocked()
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
                                    enabled: adapter && !adapter.isPlayerFieldBlocked()
                                    onClicked: {
                                        if (adapter && !getCellIsShoted(index) && !adapter.isPlayerFieldBlocked()) {
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

        Button {
            text: "🔄 Новая игра"
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 14
            onClicked: {
                if (adapter) {
                    adapter.newGame();
                    root.updateAllCells();
                    botStatusText.text = adapter.getBotGameStatus();
                    playerStatusText.text = adapter.getPlayerGameStatus();
                }
            }
            background: Rectangle {
                color: parent.pressed ? "#2980b9" : "#3498db"
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

        function onTurnStatusChanged() {
            turnStatusText.text = adapter ? adapter.getTurnStatus() : ""
            if(adapter.isPlayerTurn()) {
                turnStatus.color = "green";
            } else {
                turnStatus.color = "red";
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
