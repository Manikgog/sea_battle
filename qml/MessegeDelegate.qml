import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: delegate
    height: Math.max(50, messageColumn.height + 20)
        width: parent ? parent.width : 0

    property var messageData: ({})
    property bool isOwnMessage: false

    Rectangle {
        id: messageBubble
        anchors {
            left: isOwnMessage ? undefined : parent.left
            right: isOwnMessage ? parent.right : undefined
            margins: 10
            leftMargin: isOwnMessage ? 10 : 15
            rightMargin: isOwnMessage ? 15 : 10
        }
        width: Math.min((parent ? parent.width : 300) * 0.7, 300)
        implicitHeight: messageColumn.height + 40
        color: isOwnMessage ? "#2196F3" : "white"
        radius: 12

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
                bottomMargin: 8
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
                text: messageData && messageData.sender ? messageData.sender : ""
                color: isOwnMessage ? "#e3f2fd" : (text === "Система" ? "#FF9800" : "#1565c0")
                font.pixelSize: 12
                font.bold: text !== "Система"
                width: parent.width
                wrapMode: Text.Wrap
            }

            Text {
                id: messageTextContent
                text: messageData && messageData.text ? messageData.text : ""
                color: isOwnMessage ? "white" : (senderText.text === "Система" ? "#FF9800" : "#212121")
                font.pixelSize: senderText.text === "Система" ? 12 : 14
                font.italic: senderText.text === "Система"
                width: parent.width
                wrapMode: Text.Wrap
            }

            Text {
                id: timeText
                text: messageData && messageData.time ? messageData.time : ""
                color: isOwnMessage ? "#e3f2fd" : "#757575"
                font.pixelSize: 10
                anchors.right: parent.right
            }
        }
    }
}
