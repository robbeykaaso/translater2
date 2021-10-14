import QtQuick 2.12
import "../../../../qml/gui/Basic"

TWindow{
    width: 260
    height: 220
    caption: Pipelines().tr("image show")
    content:
        Rectangle{
            anchors.fill: parent
            color: "gray"
            Column{
                topPadding: 10
                anchors.fill: parent
                Spin{
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 20
                    caption.text: Pipelines().tr("opacity") + ":"
                    ratio: 0.4
                    spin.minimumValue: 0
                    spin.maximumValue: 255
                    spin.onValueChanged: {
                        Pipelines().run("setImageShow", {setImageOpacity: {value: spin.value}})
                    }
                }
                Combo{
                    id: fmt
                    property string lastFormat: ""
                    property bool inited: false
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 20
                    caption.text: Pipelines().tr("resize method") + ":"
                    ratio: 0.4
                    combo.model: ["linear", "nearest", "cubic", "area", "lanczos4"]
                    combo.onCurrentIndexChanged: {
                        if (inited){
                            Pipelines().run("setImageResizeFormat", fmt.combo.model[fmt.combo.currentIndex], "", {show: true})
                        }else
                            inited = true
                    }
                }
                Combo{
                    id: org
                    property string lastFormat: ""
                    property bool inited: false
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 20
                    caption.text: Pipelines().tr("origin format") + ":"
                    ratio: 0.4
                    combo.model: ["None", "Gray", "RGB", "BayerBG", "BayerGB", "BayerRG", "BayerGR"]
                    combo.onCurrentIndexChanged: {
                        if (inited){
                            Pipelines().run("setImageShow", {convertImageFormat: {origin: org.combo.model[org.combo.currentIndex],
                                                                                  target: tgt.combo.model[tgt.combo.currentIndex]}})
                        }else
                            inited = true
                    }
                }
                Combo{
                    id: tgt
                    property string lastFormat: ""
                    property bool inited: false
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 20
                    caption.text: Pipelines().tr("target format") + ":"
                    ratio: 0.4
                    combo.model: ["None", "Gray", "RGB", "BGR"]
                    combo.onCurrentIndexChanged: {
                        if (inited){
                            Pipelines().run("setImageShow", {convertImageFormat: {origin: org.combo.model[org.combo.currentIndex],
                                                                                  target: tgt.combo.model[tgt.combo.currentIndex]}})
                        }else
                            inited = true
                    }
                }
            }
        }
    footbuttons: [
        {cap: Pipelines().tr("Cancel"), func: function(){
            Pipelines().run("setImageResizeFormat", fmt.lastFormat, "", {show: false})
            Pipelines().run("setImageShow", {setImageOpacity: {value: - 1},
                                             convertImageFormat: {origin: org.lastFormat,
                                                                  target: tgt.lastFormat}
                                            })
            org.inited = false
            org.combo.currentIndex = Math.max(0, org.combo.model.indexOf(org.lastFormat))
            tgt.inited = false
            tgt.combo.currentIndex = Math.max(0, tgt.combo.model.indexOf(tgt.lastFormat))
            fmt.inited = false
            fmt.combo.currentIndex = Math.max(0, fmt.combo.model.indexOf(fmt.lastFormat))
            close()
        }}
    ]
    onVisibleChanged: {
        if (visible){
            org.lastFormat = org.combo.currentText
            tgt.lastFormat = tgt.combo.currentText
        }
    }

    Component.onCompleted: {
        Pipelines().add(function(aInput){
            show()
        }, {name: "_ImageOpacity"})
    }
}
