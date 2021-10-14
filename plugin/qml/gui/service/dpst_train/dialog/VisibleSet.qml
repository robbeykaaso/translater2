import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../../qml/gui/Basic"

TWindow{
    property var tr: Pipelines().tr
    property string after_set
    width: 240
    height: mdl.count * 30 + 80
    caption: tr("set invisible label")

    content:
        ListView{
            anchors.fill: parent
            delegate: Edit {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 20
                ratio: 0.1
                caption.text: ""
                input.text: cap ? cap : ""
                input.onTextChanged: {
                    mdl.get(index).cap = input.text
                }
            }
            model: ListModel{
                id: mdl
                ListElement{
                    cap: ""
                }
            }
            ScrollBar.vertical: ScrollBar {
            }
        }

    function getInVisibleMap(){
        var ret = {}
        for (var i = 0; i < mdl.count; ++i)
            if (mdl.get(i).cap !== "")
                ret[mdl.get(i).cap] = true
        return ret
    }

    footbuttons: [
        {cap: tr("add"), func: function(){
            mdl.append({idx: mdl.count})
        }},
        {cap: tr("delete"), func: function(){
            if (mdl.count)
                mdl.remove(mdl.count - 1)
        }},
        {cap: tr("close"), func: function(){
            close()
        }}
    ]

    onVisibleChanged: {
        if (!visible)
            Pipelines().run(after_set, "")
    }

    Component.onCompleted: {
        Pipelines().add(function(aInput){
            show()
            var nm = aInput.data()
            after_set = nm.substring(0, nm.indexOf("_ide_dpst_anno")) + "_refresh"
        }, {name: "setLabelVisible"})
        Pipelines().add(function(aInput){
            aInput.setData(getInVisibleMap()).out()
        }, {name: "c++_getLabelVisible", type: "Partial", external: "c++"})
    }
}
