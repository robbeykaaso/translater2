import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import "../qml/gui/Basic"
import "../qml/gui/Pipe"
import "../service"

ApplicationWindow {
    id: main_window
    width: 800
    height: 600
    visible: true
    menuBar: MenuBar{
        Menu{
            title: Pipelines().tr("View")
            /*MenuItem{
                text: Pipelines().tr("test")
                onTriggered: Pipelines().run("testUpdateLayout", {})
            }*/
            MenuItem{
                text: Pipelines().tr("loadView")
                onTriggered: Pipelines().run("loadView", {})
            }
            MenuItem{
                text: Pipelines().tr("saveView")
                onTriggered: Pipelines().run("saveView", 0)
            }
            MenuItem{
                text: Pipelines().tr("layout")
                onTriggered: Pipelines().run("enableLayout", 0)
            }
        }
        Menu{
            title: Pipelines().tr("Help")
        }
    }

    contentData:
        Column{
            anchors.fill: parent
            ReaGrid{
                id: container
                width: parent.width
                height: parent.height - 30
                com: GridItem{
                        anchors.fill: parent
                    }
                Component.onCompleted: {
                   // Pipelines().run("updateLayout", dt)
                }
            }
            Status{
                style: StatusBarStyle {
                    background: Rectangle {
                        color: "#F2F2F2"
                    }
                }
                name: "status"
                height: 30
                width: parent.width
                Component.onCompleted: {
                    updateStatus([])
                }
            }
        }
    Progress{

    }
    File{

    }
    Service{

    }
    Interface{

    }

    GridsModel{

    }

    Timer {
        id: timer
        interval: 1000; running: false; repeat: false
        onTriggered: {
            Pipelines().run("closeSplash", true)
        }
    }

    Component.onCompleted: {
        timer.start()
    }
}
