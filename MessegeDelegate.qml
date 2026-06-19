import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: delegate
    height: Math.max(50, messageColumn.height + 20)
    width: ListView.view.width

    property string messageText: ""
    property string sender: ""
    property string time: ""
    property bool isOwnMessage: false

    Rectangle {
        id: messageBubble
        anchors {
            left: isOwnMessage ? undefined : parent.left
            right: isOwnMessage ? parent.right : undefined
            margins: 10
        }
        width: Math.min(parent.width * 0.7, 300)
        height: messageColumn.height + 20
        color: isOwnMessage ? "#2196F3" : "white"
        radius: 12

        // Хвостик сообщения
        Rectangle {
            width: 10
            height: 10
            color: parent.color
            rotation: 45
            anchors {
                right: isOwnMessage ? parent.left : undefined
                left: isOwnMessage ? undefined : parent.right
                bottom: parent.bottom
                margins: -4
                bottomMargin: isOwnMessage ? 8 : 8
            }
        }

        Column {
            id: messageColumn
            anchors {
                fill: parent
                margins: 10
            }
            spacing: 4
            width: parent.width - 20

            Text {
                id: senderText
                text: sender
                color: isOwnMessage ? "#e3f2fd" : (sender === "Система" ? "#FF9800" : "#1565c0")
                font.pixelSize: 12
                font.bold: sender !== "Система"
                width: parent.width
                wrapMode: Text.Wrap
            }

            Text {
                id: messageTextContent
                text: messageText
                color: isOwnMessage ? "black" : (sender === "Система" ? "#FF9800" : "#212121")
                font.pixelSize: sender === "Система" ? 12 : 14
                font.italic: sender === "Система"
                width: parent.width
                wrapMode: Text.Wrap
            }

            Text {
                id: timeText
                text: time
                color: isOwnMessage ? "#e3f2fd" : "#757575"
                font.pixelSize: 10
                anchors.right: parent.right
            }
        }
    }
}
