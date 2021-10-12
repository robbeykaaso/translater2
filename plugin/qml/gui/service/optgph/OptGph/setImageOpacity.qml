import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../qml/gui/Basic"

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

        seq.combo.model.clear()
        var bf = cfg["before"]
        var seqs = cfg["seq"] || []
        for (var i in bf)
            seq.combo.model.append({name: bf[i], seled: seqs.indexOf(bf[i]) >= 0})
        for (var i = seqs.length - 1; i >= 0; --i)
            if (bf.indexOf(seqs[i]) < 0)
                seqs.splice(i, 1)
        delete cfg["before"]
        seq.combo.displayText = Util.arrayToText(seqs)
        var idxes = cfg["imageIndex"] || []
        for (var i = 0; i < imgIdx.combo.model.count; ++i)
            imgIdx.combo.model.get(i).seled = idxes.indexOf(i) >= 0
        imgIdx.combo.displayText = Util.arrayToText(idxes)
        thread.spin.value = cfg["thread"] || 0
        value.spin.value = cfg["value"] || 0
    }
    Column{
        anchors.fill: parent
        spacing: 5
        topPadding: 5
        Text {
            text: Pipelines().tr("setImageOpacity")
            font.pixelSize: 16
            anchors.horizontalCenter: parent.horizontalCenter
        }
        MultiSelectCombo{
            id: seq
            anchors.horizontalCenter: parent.horizontalCenter
            height: 30
            width: parent.width - 20
            ratio: 0.3
            caption.text: Pipelines().tr("seq") + ":"
        }
        MultiSelectCombo{
            id: imgIdx
            anchors.horizontalCenter: parent.horizontalCenter
            height: 30
            width: parent.width - 20
            ratio: 0.3
            caption.text: Pipelines().tr("imgIdx") + ":"
            combo.model: ListModel {
                ListElement { name: 0; seled: false}
                ListElement { name: 1; seled: false}
                ListElement { name: 2; seled: false}
                ListElement { name: 3; seled: false}
            }
        }
        Spin{
            id: thread
            anchors.horizontalCenter: parent.horizontalCenter
            height: 30
            width: parent.width - 20
            caption.text: Pipelines().tr("thread") + ":"
            ratio: 0.3
            spin.value: 0
            spin.minimumValue: 0
            spin.maximumValue: 1023
        }
        Spin{
            id: value
            anchors.horizontalCenter: parent.horizontalCenter
            height: 30
            width: parent.width - 20
            caption.text: Pipelines().tr("value") + ":"
            ratio: 0.3
            spin.value: 0
            spin.minimumValue: 0
            spin.maximumValue: 255
        }
        Button{
            x: parent.width - width - 20
            width: 60
            height: 30
            text: Pipelines().tr("OK")
            onClicked: {
                cfg["thread"] = thread.spin.value
                cfg["value"] = value.spin.value
                cfg["seq"] = seq.combo.displayText.split("、")
                cfg["imageIndex"] = imgIdx.combo.displayText.split("、").map(function(aInput){return aInput === "" ? 0 : parseInt(aInput)})
                Pipelines().run("setOperatorConfig", cfg, "", {id: mdl, opt: opt})
            }
        }
    }
}
