import QtQuick 2.12
import "../../../qml/gui/Basic"
import "../image"

QSG{
    property var prt: true
    property var selects
    tools: ListModel{
        ListElement{
            cap: "default"
            prm: "transform"
            tag: ""
        }
        ListElement{
            cap: "select"
            prm: "selectc"
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
        ListElement{
            cap: "visible"
            cmd: "setLabelVisible"
            prm_nm: ""
            tag: ""
        }
    }
    EditCombo{
        visible: false
        width: 80
        height: 30
        modellist: []

        Component.onDestruction: {
            Pipelines().find("qml_QSGAttrUpdated_" + parent.name).removeNext("objectDeleted_" + parent.name, true, false)
            Pipelines().find("updateQSGSelects_" + parent.name).removeNext("QSGSelectsUpdated_" + parent.name, true, false)
        }
        Component.onCompleted: {
            Pipelines().find("qml_QSGAttrUpdated_" + parent.name).nextF(function(aInput){
                var dt = aInput.data()
                for (var i in dt){
                    if (dt[i]["key"][0] === "objects" && dt[i]["type"] === "del" && parent.selects && parent.selects[dt[i]["tar"]] && dt[i]["cmd"]){
                        visible = false
                        break
                    }
                }
            }, "", {name: "objectDeleted_" + parent.name})

            Pipelines().find("updateQSGSelects_" + parent.name).nextF(function(aInput){
                var dt = aInput.data()
                if (dt["bound"]){
                    var bnd = dt["bound"]
                    x = bnd[0] + (bnd[2] - bnd[0] - 80) * 0.5
                    y = bnd[1] - 15
                    var shps = dt["shapes"]
                    var lbl = ""
                    var idx = 0
                    parent.selects = shps
                    for (var i in shps){
                        if (idx++)
                            lbl += "/"
                        lbl += shps[i]["caption"] || ""
                    }
                    currentSelect = lbl
                    visible = true
                    if (dt["invisible"])
                        visible = false
                    else if (dt["show_menu"]){
                        popupOpen = true
                    }
                }else{
                    if (parent.selects){
                        if (currentSelect != "" && (modellist.findIndex(e=>{return e === currentSelect}) < 0)){
                            modellist.push(currentSelect)
                            updateGUI()
                        }
                        var lbls = currentSelect.split("/")
                        var shps0 = Object.keys(parent.selects)
                        for (var j = 0; j < lbls.length; ++j)
                            if (j < shps0.length)
                                aInput.outs([{obj: shps0[j], key: ["caption"], val: lbls[j]}], "qml_updateQSGAttr_" + parent.name)
                    }
                    visible = false
                }
            }, "", {name: "QSGSelectsUpdated_" + parent.name})
        }
    }
    Component.onCompleted: {
        Pipelines().find("setDpstAnnoMode").nextF(function(aInput){
            if (prt.lasttype === "dpst_anno"){
                aInput.outs(aInput.data(), name.substring(0, name.indexOf("_ide_dpst_anno")) + "_refresh")
            }
        }, "", {name: "setDpstAnnoMode_" + name})
    }
    Component.onDestruction: {
        Pipelines().find("setDpstAnnoMode").removeNext("setDpstAnnoMode_" + name, true, false)
    }
}
