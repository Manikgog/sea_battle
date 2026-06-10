import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 600
    height: 700
    visible: true
    title: qsTr("Морской бой")

    property var model: ModelAdapter

    // Функция для обновления конкретной клетки
    function updateCell(index) {
        var item = gridRepeater.itemAt(index);
        if (item) {
            item.color = item.getCellColor(index);
            // Принудительно обновляем текст
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
            updateCell(i);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
/*
        Text {
            text: "Морской бой"
            font.pixelSize: 28
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            color: "darkblue"
        }
*/
        Rectangle {
            Layout.preferredWidth: 500
            Layout.preferredHeight: 500
            Layout.alignment: Qt.AlignHCenter
            color: "lightgray"
            border.color: "silver"
            border.width: 2
            radius: 5

            Grid {
                id: gameGrid
                anchors.centerIn: parent
                rows: 10
                columns: 10
                spacing: 2

                Repeater {
                    id: gridRepeater
                    model: 100

                    Rectangle {
                        id: cellRect
                        width: 46
                        height: 46
                        color: getCellColor(index)
                        border.color: "slategray"
                        border.width: 1

                        Text {
                            id: cellText
                            anchors.centerIn: parent
                            text: getCellMark(index)
                            font.pixelSize: 24
                            font.bold: true
                            color: getMarkColor(index)
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (root.model && !getCellIsShoted(index)) {
                                    root.model.shot(index);
                                    // Обновляем только эту клетку
                                    root.updateCell(index);
                                    // Обновляем статус игры
                                    statusText.text = root.model.getGameStatus();
                                }
                            }
                        }

                        function getCellIsShoted(cellIndex) {
                            if (!root.model) return false;
                            return root.model.getCellIsShoted(cellIndex);
                        }

                        function getCellIsOccupied(cellIndex) {
                            if (!root.model) return false;
                            return root.model.getCellIsOccupied(cellIndex);
                        }

                        function getCellMark(cellIndex) {
                            if (!root.model) return "";
                            if (root.model.getCellIsShoted(cellIndex)) {
                                if (root.model.getCellIsOccupied(cellIndex)) {
                                    return "✖";
                                } else {
                                    return "•";
                                }
                            }
                            return "";
                        }

                        function getMarkColor(cellIndex) {
                            if (!root.model) return "black";
                            if (root.model.getCellIsOccupied(cellIndex)) {
                                return "red";
                            } else {
                                return "blue";
                            }
                        }

                        function getCellColor(cellIndex) {
                            if (!root.model) return "#ecf0f1";
                            if (root.model.getCellIsShoted(cellIndex)) {
                                if (root.model.getCellIsOccupied(cellIndex)) {
                                    return "mistyrose";
                                } else {
                                    return "aliceblue";
                                }
                            }
                            return "#ffffff";
                        }
                    }
                }
            }
        }

        Button {
            text: "Новая игра"
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 16
            onClicked: {
                root.model.newGame();
                // Обновляем все клетки
                root.updateAllCells();
                // Обновляем статус игры
                statusText.text = root.model.getGameStatus();
                statusText.color = "seagreen";
            }
            background: Rectangle {
                color: parent.pressed ? "royalblue" : "cornflowerblue"
                radius: 5
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: parent.font.pixelSize
            }
        }

        Text {
            id: statusText
            text: root.model ? root.model.getGameStatus() : ""
            font.pixelSize: 18
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            color: "seagreen"
        }

        Connections {
            target: root.model
            function onGameWon() {
                statusText.text = "ПОБЕДА! Все корабли уничтожены!"
                statusText.color = "tomato"
            }
            function onGameStatusChanged() {
                // Обновляем статус
                statusText.text = root.model.getGameStatus()
                // Не перерисовываем все клетки, только если нужно
            }
        }
    }
}
