import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../../qml/gui/Basic"

Rectangle{
    anchors.fill: parent
    color: "gray"

    property var cfg
    property var mdl
    property var opt
    function initialize(aID, aOpt, aConfig){
        mdl = aID
        opt = aOpt
        cfg = aConfig

        delete cfg["before"]
        input.spin.value = cfg["input"] !== undefined ? cfg["input"]  : - 1
        view.combo.currentIndex = view.val.indexOf(cfg["view"])
    }

    Column{
        anchors.fill: parent
        spacing: 5
        topPadding: 5
        Text {
            text: Pipelines().tr("inputImage")
            font.pixelSize: 16
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Spin{
            id: input
            anchors.horizontalCenter: parent.horizontalCenter
            height: 30
            width: parent.width - 20
            caption.text: Pipelines().tr("input") + ":"
            ratio: 0.3
            spin.value: - 1
            spin.minimumValue: - 1
            spin.maximumValue: 50
        }
        Combo{
            id: view
            property var val
            anchors.horizontalCenter: parent.horizontalCenter
            height: 30
            width: parent.width - 20
            caption.text: Pipelines().tr("view") + ":"
            ratio: 0.3
            combo.model: []
            ViewMonitor{

            }
        }
        Button{
            x: parent.width - width - 20
            width: 60
            height: 30
            text: Pipelines().tr("OK")
            onClicked: {
                cfg["input"] = input.spin.value
                cfg["view"] = view.val[view.combo.currentIndex]
                Pipelines().run("setOperatorConfig", cfg, "", {id: mdl, opt: opt})
            }
        }
    }
}
