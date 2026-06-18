import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import MessengerApp 1.0

ApplicationWindow {
    id: root
    width: 600
    height: 700
    visible: true
    title: "Мессенджер"

    property string currentUser: backend.currentUser
    property bool isConnected: backend.isConnected

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
            Layout.preferredHeight: 150  // Увеличили высоту
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
                            // Обновляем имя при вводе
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
                                    // Клиент
                                    backend.networkManager.connectToHost(addressField.text, 8080)
                                } else {
                                    // Сервер
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



        // Список сообщений
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
            Layout.preferredHeight: 100
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
                                inputContainer.Layout.preferredHeight = 100
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
}
