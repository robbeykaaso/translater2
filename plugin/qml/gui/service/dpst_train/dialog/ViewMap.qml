import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../../../qml/gui/Basic"

TWindow{
    property var tr: Pipelines().tr
    width: 320
    height: mdl.count * 30 + 80
    caption: tr("set view map")

    content:
        ListView{
            anchors.fill: parent
            delegate: Spin {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 20
                caption.text: index + ":"
                ratio: 0.4
                spin.value: idx
                spin.onValueChanged: {
                    mdl.get(index).idx = spin.value
                }
            }
            model: ListModel{
                id: mdl
                ListElement{
                    idx: 0
                }
            }
            ScrollBar.vertical: ScrollBar {
            }
        }

    function getViewMap(){
        var ret = []
        for (var i = 0; i < mdl.count; ++i)
            ret.push(mdl.get(i).idx)
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

    Component.onCompleted: {
        Pipelines().add(function(aInput){
            show()
        }, {name: "setViewMap"})
        Pipelines().add(function(aInput){
            aInput.setData(getViewMap()).out()
        }, {name: "c++_getViewMap", type: "Partial", external: "c++"})

        Pipelines().add(function(aInput){
            aInput.out()
        }, {name: "logTrain"})
    }
}
