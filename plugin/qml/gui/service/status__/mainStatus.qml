import QtQuick 2.12
import QtQuick.Controls.Styles 1.4
import "../../../qml/gui/Pipe"

Status{
    style: StatusBarStyle {
        background: Rectangle {
            color: "#F2F2F2"
        }
    }
    height: 30
    width: parent.width
    Component.onCompleted: {
        Pipelines().find("_updateStatus").next(name + "_updateStatus")
        updateStatus([])
    }
}
