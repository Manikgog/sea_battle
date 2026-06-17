import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    id: root
    width: 600
    height: 450
    visible: true
    title: qsTr("Морской бой")

    property var model: ModelAdapter


    MessageDialog {
        id: gameMessageDialog
        title: "Игра окончена"
        buttons: MessageDialog.Ok
        onAccepted: {
            // Можно добавить действие при закрытии диалога
        }
    }

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
            updateBotCell(i);
            updatePlayerCell(i);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

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
                    height: 50
                    color: "#ecf0f1"
                    radius: 8
                    border.color: "#bdc3c7"
                    border.width: 1

                    Text {
                        id: playerStatusText
                        anchors.centerIn: parent
                        text: root.model ? root.model.getPlayerGameStatus() : ""
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        color: "#2c3e50"
                    }
                }
            }

            // Поле бота
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

                    // затемнение поля игрока во время хода бота
                    Rectangle {
                        anchors.fill: parent
                        color: "black"
                        opacity: root.model.isPlayerFieldBlocked() ? 0.3 : 0
                        visible: root.model.isPlayerFieldBlocked()
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
                                    enabled: !root.model.isPlayerFieldBlocked()
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
                    height: 50
                    color: "#ecf0f1"
                    radius: 8
                    border.color: "#bdc3c7"
                    border.width: 1

                    Text {
                        id: botStatusText
                        anchors.centerIn: parent
                        text: root.model ? root.model.getBotGameStatus() : ""
                        font.pixelSize: 16
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
            font.pixelSize: 16
            onClicked: {
                root.model.newGame();
                root.updateAllCells();
                botStatusText.text = root.model.getBotGameStatus();
                botStatusText.color = "#27ae60";
                playerStatusText.text = root.model.getPlayerGameStatus();
                playerStatusText.color = "#27ae60";
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
                botStatusText.color = "#2c3e50"
                // Обновляем поле бота при изменении статуса (после хода бота)
                for (var i = 0; i < 100; i++) {
                    root.updateBotCell(i);
                    root.updatePlayerCell(i);
                }
            }
        }
    }

    Component.onCompleted: {
        if (root.model) {
            botStatusText.text = root.model.getBotGameStatus();
            playerStatusText.text = root.model.getPlayerGameStatus();
        }
    }
}
