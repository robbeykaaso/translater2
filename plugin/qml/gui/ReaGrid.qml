import QtQuick 2.12
import QtWebEngine 1.8
import QtWebChannel 1.0

WebEngineView {
    id: webview
    property string name: ""
    property Component com: Rectangle{
        anchors.fill: parent
        border.color: "black"
    }
    property int cur
    property int cur_edit

    z: - 1
    /*Rectangle{
        width: parent.width - 300
        height: parent.height - 50
        color: "red"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 1
    }*/

    function getGridItem(aIndex){
        if (aIndex >= 0 && aIndex < children.length && children[aIndex].visible)
            return children[aIndex].children[0]
        else
            return null
    }

    Repeater{
        anchors.fill: parent
        z: 1
        Item{
            id: grd
            x: webview.width * posx / 12
            y: 30 * posy - dely
            z: 1
            width: webview.width * posw / 12
            height: 30 * posh
            visible: !deled
            Component.onCompleted: {
                com.createObject(grd, {name: name + "reagrid" + posi, index: parseInt(posi)})
            }
        }
        model: ListModel{
            id: mdls
            /*ListElement{
                posi: "a"
                posx: 0
                posy: 0
                posw: 1
                posh: 2
                dely: 0
                clr: "red"
            }*/
            Component.onCompleted: {
                Pipelines().add(function(aInput){
                    var lo = aInput.data()
                    if (aInput.scope().data("remove")){
                        mdls.get(aInput.scope().data("index")).deled = true
                    }
                    var mx = 0
                    for (var i in lo){
                        var idx = parseInt(lo[i].i)
                        if (idx >= mdls.count){
                            mdls.append({
                                            posi: lo[i].i,
                                            posx: lo[i].x,
                                            posy: lo[i].y,
                                            posw: lo[i].w,
                                            posh: lo[i].h,
                                            dely: lo[i].dely,
                                            deled: false
                                        })
                        }else{
                            mdls.get(idx).posx = lo[i].x
                            mdls.get(idx).posy = lo[i].y
                            mdls.get(idx).posw = lo[i].w
                            mdls.get(idx).posh = lo[i].h
                            mdls.get(idx).dely = lo[i].dely
                            mdls.get(idx).deled = false
                        }
                        mx = Math.max(mx, idx)
                    }
                    for (var j = mdls.count - 1; j > mx; --j)
                        mdls.remove(j)
                    Pipelines().run("reagridCountChanged", mdls.count)
                    aInput.out()
                }, {name: name + "layoutChanged"})
            }
        }
    }
    webChannel: WebChannel{
        id: webview_chn
        Component.onCompleted: {
            var stm = Pipelines().asyncCall("pipelineJSObject", 0)
            webview_chn.registerObject("Pipelinec++", stm.data())
            stm = Pipelines().asyncCall("pipelineQMLObject", 0)
            webview_chn.registerObject("Pipelineqml", stm.data())
            //webview_chn.registerObject("Pipeline", stm.scope().data("pipeline"))
            webview.url = "file:/nwlan_ui/html/grid.html"
        }
    }
    onContextMenuRequested: { //disable context menu
        request.accepted = true
    }
}
