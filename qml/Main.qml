import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 800
    height: 600
    visible: true
    title: qsTr("Морской бой")

    property int gameMode: 0 // 0 - выбор режима, 1 - локальная игра, 2 - сетевая игра

    // Создаем адаптеры
    property var localAdapter: ModelAdapterLocal {}
    property var networkAdapter: ModelAdapterNetwork {}

    StackLayout {
        anchors.fill: parent
        currentIndex: gameMode

        // Экран выбора режима
        Item {
            id: menuScreen

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    text: "Морской бой"
                    font.pixelSize: 48
                    font.bold: true
                    color: "#2c3e50"
                    Layout.alignment: Qt.AlignHCenter
                }

                Button {
                    text: "Игра с ботом"
                    font.pixelSize: 24
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 60
                    onClicked: {
                        localAdapter.newGame()
                        gameMode = 1
                    }
                    background: Rectangle {
                        color: parent.pressed ? "#2980b9" : "#3498db"
                        radius: 12
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: parent.font.pixelSize
                    }
                }

                Button {
                    text: "Сетевая игра"
                    font.pixelSize: 24
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 60
                    onClicked: {
                        gameMode = 2
                    }
                    background: Rectangle {
                        color: parent.pressed ? "#27ae60" : "#2ecc71"
                        radius: 12
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: parent.font.pixelSize
                    }
                }

                Button {
                    text: "Выход"
                    font.pixelSize: 18
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 50
                    onClicked: Qt.quit()
                    background: Rectangle {
                        color: parent.pressed ? "#c0392b" : "#e74c3c"
                        radius: 12
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

        // Локальная игра
        LocalGame {
            id: localGame
            adapter: localAdapter
            onBackToMenu: gameMode = 0
        }

        // Сетевая игра
        NetworkGame {
            id: networkGame
            adapter: networkAdapter
            onBackToMenu: {
                networkAdapter.disconnectFromServer()
                gameMode = 0
            }
        }
    }
}
