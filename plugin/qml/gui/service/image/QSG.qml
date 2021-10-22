import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../qml/gui/Custom"
import QSGBoard 2.0

Item{
    property alias plugins: brd.plugins
    property alias name: brd.name
    property var tools: ListModel{
        ListElement{
            cap: "default"
            prm: "transform"
            tag: ""
        }
        ListElement{
            cap: "select"
            prm: "select"
            tag: ""
        }
        ListElement{
            cap: "free"
            prm: "drawfree"
            tag: ""
        }
        ListElement{
            cap: "rect"
            prm: "drawrect"
            tag: ""
        }
        ListElement{
            cap: "line"
            prm: "drawline"
            tag: ""
        }
        ListElement{
            cap: "ellipse"
            prm: "drawellipse"
            tag: ""
        }
        ListElement{
            cap: "circle"
            prm: "drawcircle"
            tag: ""
        }
        ListElement{
            cap: "poly"
            prm: "drawpoly"
            tag: ""
        }
        ListElement{
            cap: "node"
            prm: "editnode"
            tag: ""
        }
        ListElement{
            cap: "undo"
            cmd: "qml_doCommand"
            prm_bool: false
            tag: "manual"
        }
        ListElement{
            cap: "redo"
            cmd: "qml_doCommand"
            prm_bool: true
            tag: "manual"
        }
        ListElement{
            cap: "synchronize"
            cmd: "_Synchronize"
            prm_nm: ""
            tag: ""
        }
        ListElement{
            cap: "opacity"
            cmd: "_ImageOpacity"
            prm_nm: ""
            tag: ""
        }
    }

    Column{
        anchors.fill: parent
        Row{
            width: parent.width
            height: 20
            Repeater{
                delegate: Button{
                    width: (parent === undefined) || (parent === null) ? 10 : parent.width / tools.count
                    height: (parent === undefined) || (parent === null) ? 10 : parent.height
                    text: Pipelines().tr(cap)
                    onClicked: {
                        if (!cmd)
                            Pipelines().run("qml_updateQSGCtrl_" + name, [{type: prm}], tag)
                        else
                            if (prm_nm != undefined)
                                Pipelines().run(cmd, name, tag)
                            else if (prm_bool != undefined)
                                Pipelines().run(cmd, prm_bool, tag)
                    }
                }
                model: tools
            }
        }
        QSGBoard{
            id: brd
            width: parent.width
            height: parent.height - 20
            plugins: [{type: "transform"}]
            clip: true
            Component.onDestruction: {
                beforeDestroy()
            }
            BoardMenu{

            }
        }
    }
    function beforeDestroy(){
        brd.beforeDestroy()
    }
}
