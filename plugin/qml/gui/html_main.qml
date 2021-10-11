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
                onTriggered: Pipelines().run("enableLayout", "")
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
                property bool layout_mode: true
                property var ide_type: ({})
                width: parent.width
                height: parent.height - 30
                com: GridItem{
                        layout_mode: container.layout_mode
                        init_edit_mode: container.ide_type[index.toString()] || "auto"
                        anchors.fill: parent
                    }
                Component.onCompleted: {
                    Pipelines().find("enableLayout").nextF(function(aInput){
                        ide_type = aInput.scope().data("ide_type") || {}
                        layout_mode = aInput.data()
                    })
                    Pipelines().add(function(aInput){
                        aInput.out()
                    }, {name: "js_openWorkFile"}).nextF(function(aInput){
                        var itm = getGridItem(cur_edit)
                        if (itm && itm.mode !== "resource"){
                            itm.detail = aInput.data()
                            itm.rt = aInput.scope().data("root")
                            itm.stg_config = aInput.scope().data("config")
                            aInput.scope().cache("index", cur_edit)

                            if (itm.mode === "auto")
                                aInput.out(aInput.data().split(".").pop().toLowerCase())
                            else
                                aInput.out(itm.mode)
                        }
                    }, "", {name: "openWorkFile", type: "Partial", external: "c++"})
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
    PWindow{

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
