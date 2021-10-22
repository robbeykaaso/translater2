import QtQuick 2.12
import QtQuick.Controls 2.12
import QSGBoard 2.0
import "../../../qml/gui/Custom"

Item{
    clip: true
    property alias name: brd.name

    property string detail
    property string rt
    property var stg_config
    Column{
        anchors.fill: parent
        Row{
            width: parent.width
            height: 30
            Button{
                width: 60
                height: parent.height
                text: Pipelines().tr("input")
                onClicked: Pipelines().run("pasteShapes_" + brd.name, {shapes: [{type: "poly",
                                                                                 points: [[0, 0, 100, 0, 100, 30, 0, 30, 0, 0]],
                                                                                 caption: "inputImage"}]})
            }
            Button{
                width: 60
                height: parent.height
                text: Pipelines().tr("showOne")
                onClicked: Pipelines().run("pasteShapes_" + brd.name, {shapes: [{type: "poly",
                                                                                 points: [[0, 0, 100, 0, 100, 30, 0, 30, 0, 0]],
                                                                                 caption: "showOneImage"}]})
            }
            Button{
                width: 60
                height: parent.height
                text: Pipelines().tr("opacity")
                onClicked: Pipelines().run("pasteShapes_" + brd.name, {shapes: [{type: "poly",
                                                                                 points: [[0, 0, 100, 0, 100, 30, 0, 30, 0, 0]],
                                                                                 caption: "setImageOpacity"}]})
            }
           /* Button{
                width: 60
                height: parent.height
                text: Pipelines().tr("format")
                onClicked: Pipelines().run("pasteShapes_" + brd.name, {shapes: [{type: "poly",
                                                                                 points: [[0, 0, 100, 0, 100, 30, 0, 30, 0, 0]],
                                                                                 caption: "convertImageFormat"}]})
            }*/
            Button{
                width: 60
                height: parent.height
                text: Pipelines().tr("undo")
                onClicked: Pipelines().run("qml_doCommand", false, "manual")
            }
            Button{
                width: 60
                height: parent.height
                text: Pipelines().tr("redo")
                onClicked: Pipelines().run("qml_doCommand", true, "manual")
            }
            Button{
                width: 60
                height: parent.height
                text: Pipelines().tr("run")
                onClicked:
                    Pipelines().run("runOperatorGraph", 0, "", {config: stg_config, root: rt, path: detail})
            }
        }
        QSGBoard{
            id: brd
            width: parent.width
            height: parent.height - 30
            plugins: [{type: "selectc2"}]
            clip: true

            BoardMenu{

            }

            Component.onDestruction: {
                beforeDestroy()
            }
        }
    }
    Rectangle{
        visible: false
        anchors.right: parent.right
        anchors.top: parent.top
        width: 150
        height: 200
        border.color: "black"
        Loader{
            id: prm_pnl
            property var cfg
            property var mdl
            property var opt
            anchors.fill: parent
            onLoaded: {
                prm_pnl.item.initialize(mdl, opt, cfg)
            }
        }
        Component.onCompleted: {
            Pipelines().add(function(aInput){
                var dt = aInput.data()
                visible = (Object.keys(dt).length > 0) && dt["type"]
                if (visible){
                    prm_pnl.mdl = aInput.scope().data("id")
                    prm_pnl.opt = aInput.scope().data("opt")
                    prm_pnl.cfg = dt
                    prm_pnl.source = "OptGph/" + dt["type"] + ".qml?" + Math.random()
                }
            }, {name: "updateQSGSelects_" + name})
        }
    }

    function beforeDestroy(){
        brd.beforeDestroy()
    }
    Component.onCompleted: {
        Pipelines().add(function(aInput){
            var scp = aInput.scope()
            detail = scp.data("path")
            stg_config = scp.data("config")
            rt = scp.data("root")
        }, {name: "updateOptGphAttr_" + name})
    }
}
